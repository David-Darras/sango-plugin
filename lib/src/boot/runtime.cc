/*
 * Copyright (C) 2026  David Darras
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file runtime.cc
 * @brief The C runtime of the plugin: the heap, the threads and newlib.
 *
 * The functions come from the files syscalls.c, thread.c, initSystem.c and
 * allocateHeaps.cpp of libctrpf (zlib license). They replace the files of
 * libctru with the same names. The declarations are in boot/runtime.h.
 */

#include "boot/runtime.h"

#include <errno.h>
#include <malloc.h>
#include <sys/iosupport.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <sys/time.h>

#include <cstring>

#include "boot/plgldr.h"
#include "core/types.h"
#include "types.h"

// The names of this block belong to libctru and to the linker script.
extern "C" {

Result __sync_init(void);
Result __sync_fini(void) __attribute__((weak));
void envDestroyHandles(void);
void mappableInit(u32 start, u32 end);
void __libc_init_array(void);
void __libc_fini_array(void);

// The heap of newlib (sbrk) is between these two addresses.
extern char* fake_heap_start;
extern char* fake_heap_end;
// The heap of the system calls of libctru.
extern u32 __ctru_heap;
extern u32 __ctru_heap_size;

// The initial values of the TLS, and its size, from the linker script.
extern const size_t __tdata_align;
extern const u8 __tdata_lma[];
extern const u8 __tdata_lma_end[];
extern u8 __tls_start[];
extern u8 __tls_end[];

// These names stop libctru from using its own start-up files.
void (*__system_retAddr)(void);
u32 __apt_appid = 0x300;
u32 __system_runflags = 0;
void __libctru_init(void) {}

} // extern "C"

// The data of a thread made by threadCreate(). The name is the name that
// libctru gives to the type behind Thread.
struct Thread_tag {
  Handle handle;
  ThreadFunc entry;
  void* argument;
  int result;
  bool is_detached;
  bool is_finished;
  struct _reent reent;
  void* stack_top;
};

namespace boot {

namespace {

/// The value that marks the thread data of a thread that libctru made.
constexpr u32 kThreadVarsMagic = 0x21545624;
/// The seconds from 1900 (the epoch of the 3DS) to 1970 (the epoch of
/// newlib), in milliseconds.
constexpr u64 kEpochDifference = 2208988800000ull;
/// The number of threads of the game that have thread data.
constexpr u32 kMaxGameThreads = 16;
/// The size of the TLS of a thread of the game.
constexpr u32 kGameTlsSize = 0x80;
/// The size of the header that the ARM ELF ABI puts before the TLS.
constexpr u32 kTlsHeaderSize = 8;
/// The smallest alignment of the stack and of the TLS of a thread.
constexpr u32 kMinAlignment = 8;

// The data that libctru keeps at the start of the TLS of each thread. The
// code of libctru reads it: the fields stay in this order.
struct ThreadVars {
  u32 magic;
  Thread thread;
  struct _reent* reent;
  void* tls_pointer; ///< The offset is used by __aeabi_read_tp.
  u32 fs_magic;
  Handle fs_session;
  bool srv_blocking_policy;
};

static_assert(sizeof(ThreadVars) <= 0x80, "ThreadVars must stay under 0x80");

// The data of a thread of the game.
struct GameThreadData {
  u32 key; ///< The TLS address of the thread. 0: this entry is free.
  ThreadVars vars;
  struct _reent reent;
  alignas(kMinAlignment) u8 tls[kGameTlsSize];
};

ThreadVars* main_thread_vars = nullptr;
GameThreadData game_threads[kMaxGameThreads];
LightLock game_threads_lock;

ThreadVars* GetThreadVars() {
  return reinterpret_cast<ThreadVars*>(getThreadLocalStorage());
}

size_t AlignTo(size_t value, size_t alignment) {
  return (value + (alignment - 1)) & ~(alignment - 1);
}

size_t GetTlsSize() { return __tls_end - __tls_start; }

size_t GetTlsLoadSize() { return __tdata_lma_end - __tdata_lma; }

// Gives the thread data to the current thread. `thread` is null for the
// thread that starts the plugin.
void InitThreadVars(Thread_tag* thread) {
  ThreadVars* vars = GetThreadVars();
  vars->magic = kThreadVarsMagic;
  vars->reent = thread != nullptr ? &thread->reent : _impure_ptr;
  vars->thread = thread;
  vars->tls_pointer =
      (thread != nullptr ? static_cast<u8*>(thread->stack_top) : __tls_start) -
      kTlsHeaderSize;
  vars->srv_blocking_policy = false;

  // The kernel does not set fpscr: no flag, no interrupt, scalar mode,
  // default NaN, flush to zero, round to nearest.
  __builtin_arm_set_fpscr(BIT(25) | BIT(24));
}

// Makes the thread data of a thread of the game. The thread uses the standard
// files of the main thread.
ThreadVars* AddGameThread(GameThreadData* entry, u32 key) {
  u8* tls = entry->tls;
  ThreadVars* vars = &entry->vars;
  vars->magic = kThreadVarsMagic;
  vars->tls_pointer = tls - kTlsHeaderSize;
  vars->reent = &entry->reent;
  if (GetTlsLoadSize() != 0) std::memcpy(tls, __tdata_lma, GetTlsLoadSize());

  _REENT_INIT_PTR(&entry->reent);
  const struct _reent* main_reent = main_thread_vars->reent;
  entry->reent._stdin = main_reent->_stdin;
  entry->reent._stdout = main_reent->_stdout;
  entry->reent._stderr = main_reent->_stderr;

  entry->key = key; // Last: the data is complete.
  return vars;
}

// Returns the thread data of a thread that the game made. At the first call
// of a thread, the function makes the data. When the table is full, the thread
// uses the data of the main thread.
ThreadVars* GetGameThreadVars() {
  const u32 key = reinterpret_cast<u32>(getThreadLocalStorage());
  ThreadVars* result = main_thread_vars;
  GameThreadData* found = nullptr;
  GameThreadData* free_entry = nullptr;

  LightLock_Lock(&game_threads_lock);
  for (GameThreadData& entry : game_threads) {
    if (entry.key == key) {
      found = &entry;
      break;
    }
    if (entry.key == 0 && free_entry == nullptr) free_entry = &entry;
  }
  if (found != nullptr) {
    result = &found->vars;
  } else if (free_entry != nullptr && GetTlsSize() <= kGameTlsSize &&
             __tdata_align <= kMinAlignment) {
    result = AddGameThread(free_entry, key);
  }
  LightLock_Unlock(&game_threads_lock);
  return result;
}

// The entry point of the threads that threadCreate() makes.
void ThreadBegin(void* argument) {
  Thread thread = static_cast<Thread>(argument);
  InitThreadVars(thread);
  thread->entry(thread->argument);
  threadExit(0);
}

} // namespace

void InitializeRuntime() {
  __sync_init();

  // The thread data of the thread that starts the plugin.
  InitThreadVars(nullptr);
  main_thread_vars = GetThreadVars();
  const size_t tls_load_size = GetTlsLoadSize();
  if (tls_load_size != 0) {
    std::memcpy(
        reinterpret_cast<void*>(AlignTo(reinterpret_cast<size_t>(__tls_start),
                                        __tdata_align)),
        __tdata_lma, tls_load_size);
  }
  LightLock_Init(&game_threads_lock);

  // The heap is all the memory that the plugin loader gave to the plugin,
  // after the code and the data.
  __ctru_heap = GetPluginHeader().heap_address;
  __ctru_heap_size = GetPluginHeader().heap_size;
  fake_heap_start = reinterpret_cast<char*>(__ctru_heap);
  fake_heap_end = fake_heap_start + __ctru_heap_size;
  // The addresses that libctru uses to map memory blocks.
  mappableInit(0x11000000, 0x14000000);

  __libc_init_array();
}

void ShutDownRuntime() {
  envDestroyHandles();
  if (__sync_fini != nullptr) __sync_fini();
  __libc_fini_array();
}

} // namespace boot

// The system calls of newlib. The names belong to newlib.
extern "C" {

struct _reent* __SYSCALL(getreent)() {
  boot::ThreadVars* vars = boot::GetThreadVars();
  if (vars->magic != boot::kThreadVarsMagic) {
    return boot::GetGameThreadVars()->reent;
  }
  return vars->reent;
}

// Returns the TLS of the current thread. __aeabi_read_tp calls it, in assembly:
// the compiler cannot see the call, so the function must stay.
USED void* __getThreadLocalStorage(void) {
  boot::ThreadVars* vars = boot::GetThreadVars();
  if (vars->magic != boot::kThreadVarsMagic) {
    return boot::GetGameThreadVars()->tls_pointer;
  }
  return vars->tls_pointer;
}

// Gives the thread pointer to the code that uses the TLS.
USED NAKED void* __aeabi_read_tp() {
  __asm__ __volatile__(
      "stmfd sp!, {r1-r3, lr}        \n"
      "bl __getThreadLocalStorage    \n"
      "ldmfd sp!, {r1-r3, pc}        \n");
}

int __SYSCALL(clock_gettime)(clockid_t clock_id, struct timespec* time) {
  if (clock_id == CLOCK_REALTIME) {
    if (time != nullptr) {
      const s64 milliseconds = osGetTime() - boot::kEpochDifference;
      time->tv_sec = milliseconds / 1000;
      time->tv_nsec = (milliseconds % 1000) * 1000000;
    }
  } else if (clock_id == CLOCK_MONOTONIC) {
    if (time != nullptr) {
      const u64 ticks = svcGetSystemTick();
      time->tv_sec = ticks / SYSCLOCK_ARM11;
      time->tv_nsec =
          ((ticks % SYSCLOCK_ARM11) * 1000000000ull) / SYSCLOCK_ARM11;
    }
  } else {
    errno = EINVAL;
    return -1;
  }
  return 0;
}

int __SYSCALL(clock_getres)(clockid_t clock_id, struct timespec* resolution) {
  if (clock_id == CLOCK_REALTIME) {
    if (resolution != nullptr) {
      resolution->tv_sec = 0;
      resolution->tv_nsec = 1000000;
    }
  } else if (clock_id == CLOCK_MONOTONIC) {
    if (resolution != nullptr) {
      resolution->tv_sec = 0;
      resolution->tv_nsec = 1;
    }
  } else {
    errno = EINVAL;
    return -1;
  }
  return 0;
}

int __SYSCALL(gettod_r)(struct _reent* /*reent*/, struct timeval* time,
                        struct timezone* zone) {
  if (time != nullptr) {
    const s64 now = osGetTime() - boot::kEpochDifference;
    time->tv_sec = now / 1000;
    time->tv_usec = (now - 1000 * time->tv_sec) * 1000;
  }
  // The 3DS has no time zone.
  if (zone != nullptr) {
    zone->tz_minuteswest = 0;
    zone->tz_dsttime = 0;
  }
  return 0;
}

int __SYSCALL(nanosleep)(const struct timespec* request,
                         struct timespec* /*remaining*/) {
  svcSleepThread(request->tv_sec * 1000000000ull + request->tv_nsec);
  return 0;
}

void __SYSCALL(lock_init)(_LOCK_T* lock) { LightLock_Init(lock); }
void __SYSCALL(lock_acquire)(_LOCK_T* lock) { LightLock_Lock(lock); }
int __SYSCALL(lock_try_acquire)(_LOCK_T* lock) {
  return LightLock_TryLock(lock);
}
void __SYSCALL(lock_release)(_LOCK_T* lock) { LightLock_Unlock(lock); }

void __SYSCALL(lock_init_recursive)(_LOCK_RECURSIVE_T* lock) {
  RecursiveLock_Init(lock);
}
void __SYSCALL(lock_acquire_recursive)(_LOCK_RECURSIVE_T* lock) {
  RecursiveLock_Lock(lock);
}
int __SYSCALL(lock_try_acquire_recursive)(_LOCK_RECURSIVE_T* lock) {
  return RecursiveLock_TryLock(lock);
}
void __SYSCALL(lock_release_recursive)(_LOCK_RECURSIVE_T* lock) {
  RecursiveLock_Unlock(lock);
}

void __SYSCALL(exit)(int result) {
  (void)result;
  boot::ShutDownRuntime();
}

int __SYSCALL(cond_signal)(_COND_T* condition) {
  CondVar_Signal(reinterpret_cast<CondVar*>(condition));
  return 0;
}

int __SYSCALL(cond_broadcast)(_COND_T* condition) {
  CondVar_Broadcast(reinterpret_cast<CondVar*>(condition));
  return 0;
}

int __SYSCALL(cond_wait)(_COND_T* condition, _LOCK_T* lock,
                         uint64_t timeout_ns) {
  return CondVar_WaitTimeout(reinterpret_cast<CondVar*>(condition), lock,
                             timeout_ns)
             ? ETIMEDOUT
             : 0;
}

int __SYSCALL(cond_wait_recursive)(_COND_T* condition,
                                   _LOCK_RECURSIVE_T* lock,
                                   uint64_t timeout_ns) {
  if (lock->counter != 1) return EBADF;

  const uint32_t thread_tag = lock->thread_tag;
  lock->thread_tag = 0;
  lock->counter = 0;
  const int error = CondVar_WaitTimeout(reinterpret_cast<CondVar*>(condition),
                                        &lock->lock, timeout_ns);
  lock->thread_tag = thread_tag;
  lock->counter = 1;
  return error ? ETIMEDOUT : 0;
}

// The threads of the plugin. libctru has the same functions, but they do not
// know the threads of the game.

Thread threadCreate(ThreadFunc entry, void* argument, size_t stack_size,
                    int priority, int affinity, bool is_detached) {
  // The stack must be 8-aligned at least.
  const size_t alignment =
      __tdata_align > boot::kMinAlignment ? __tdata_align : boot::kMinAlignment;
  const size_t stack_offset = boot::AlignTo(sizeof(Thread_tag), alignment);
  const size_t allocation_size =
      boot::AlignTo(stack_offset + stack_size, alignment);
  const size_t tls_size = boot::GetTlsSize();
  const size_t tls_load_size = boot::GetTlsLoadSize();
  // memalign needs a size that is a multiple of the alignment.
  const size_t size = boot::AlignTo(allocation_size + tls_size, alignment);

  // A size that overflows.
  if (allocation_size < stack_offset) return nullptr;
  if (allocation_size - stack_offset < stack_size) return nullptr;
  if (size < allocation_size) return nullptr;

  Thread thread = static_cast<Thread>(memalign(alignment, size));
  if (thread == nullptr) return nullptr;

  thread->entry = entry;
  thread->argument = argument;
  thread->is_detached = is_detached;
  thread->is_finished = false;
  thread->stack_top = reinterpret_cast<u8*>(thread) + allocation_size;

  // The TLS starts after the stack. The pointer to it must be aligned, with
  // the header of 8 bytes.
  const size_t tls_start = boot::kTlsHeaderSize +
                           boot::AlignTo(reinterpret_cast<size_t>(
                                             thread->stack_top) -
                                             boot::kTlsHeaderSize,
                                         alignment);
  if (tls_load_size != 0) {
    std::memcpy(reinterpret_cast<void*>(tls_start), __tdata_lma, tls_load_size);
  }
  if (tls_size > tls_load_size) {
    std::memset(reinterpret_cast<u8*>(tls_start) + tls_load_size, 0,
                tls_size - tls_load_size);
  }

  // The new thread uses the standard files of the current thread.
  _REENT_INIT_PTR(&thread->reent);
  const struct _reent* current = boot::GetThreadVars()->reent;
  thread->reent._stdin = current->_stdin;
  thread->reent._stdout = current->_stdout;
  thread->reent._stderr = current->_stderr;

  if (R_FAILED(svcCreateThread(
          &thread->handle, boot::ThreadBegin, reinterpret_cast<u32>(thread),
          static_cast<u32*>(thread->stack_top), priority, affinity))) {
    free(thread);
    return nullptr;
  }
  return thread;
}

Handle threadGetHandle(Thread thread) {
  if (thread == nullptr || thread->is_finished) return ~0UL;
  return thread->handle;
}

int threadGetExitCode(Thread thread) {
  if (thread == nullptr || !thread->is_finished) return 0;
  return thread->result;
}

void threadFree(Thread thread) {
  if (thread == nullptr || !thread->is_finished) return;
  svcCloseHandle(thread->handle);
  free(thread);
}

Result threadJoin(Thread thread, u64 timeout_ns) {
  if (thread == nullptr || thread->is_finished) return 0;
  return svcWaitSynchronization(thread->handle, timeout_ns);
}

void threadDetach(Thread thread) {
  if (thread == nullptr || thread->is_detached) return;
  if (thread->is_finished) {
    threadFree(thread);
    return;
  }
  thread->is_detached = true;
}

Thread threadGetCurrent(void) {
  boot::ThreadVars* vars = boot::GetThreadVars();
  // A thread of the game has no thread object.
  if (vars->magic != boot::kThreadVarsMagic) return nullptr;
  return vars->thread;
}

void threadExit(int result) {
  Thread thread = threadGetCurrent();
  if (thread == nullptr) {
    svcBreak(USERBREAK_PANIC);
    for (;;) {
    }
  }
  thread->is_finished = true;
  if (thread->is_detached) {
    threadFree(thread);
  } else {
    thread->result = result;
  }
  svcExitThread();
}

// The functions of the threads of newlib.

int __SYSCALL(thread_create)(struct __pthread_t** thread,
                             void* (*function)(void*), void* argument,
                             void* stack_address, size_t stack_size) {
  constexpr size_t kDefaultStackSize = 32 * 1024;
  constexpr int kLowestPriority = 0x3F;
  if (stack_address != nullptr) return EINVAL;
  if (stack_size == 0) stack_size = kDefaultStackSize;

  // newlib gives a function that returns a pointer. The thread ignores it.
  Thread created = threadCreate(
      reinterpret_cast<ThreadFunc>(reinterpret_cast<void*>(function)), argument,
      stack_size, kLowestPriority, 0, false);
  if (created == nullptr) return ENOMEM;
  *thread = reinterpret_cast<struct __pthread_t*>(created);
  return 0;
}

void* __SYSCALL(thread_join)(struct __pthread_t* thread) {
  Thread joined = reinterpret_cast<Thread>(thread);
  threadJoin(joined, U64_MAX);
  void* result = reinterpret_cast<void*>(threadGetExitCode(joined));
  threadFree(joined);
  return result;
}

int __SYSCALL(thread_detach)(struct __pthread_t* thread) {
  threadDetach(reinterpret_cast<Thread>(thread));
  return 0;
}

void __SYSCALL(thread_exit)(void* value) {
  threadExit(reinterpret_cast<int>(value));
}

struct __pthread_t* __SYSCALL(thread_self)(void) {
  return reinterpret_cast<struct __pthread_t*>(threadGetCurrent());
}

} // extern "C"
