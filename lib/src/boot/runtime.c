/*
 * The C runtime of the plugin: the heap, the threads and the system calls of
 * newlib. It replaces the files of libctrpf in ctrulibExtension (syscalls.c,
 * thread.c, initSystem.c, allocateHeaps.cpp, appExit.c, ctru_exit.c and
 * aptExtension.c). Altered source version.
 *
 * Why libctru does not do it: the code of the plugin also runs on the threads
 * of the game (the hooks). The game made these threads, so they do not have
 * the thread data that libctru needs for malloc(), snprintf() and errno.
 * The functions below give them one, the first time they need it.
 */

#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <sys/iosupport.h>
#include <sys/lock.h>
#include <sys/reent.h>
#include <sys/time.h>

#include "plgldr.h"
#include "runtime.h"

// Declared here, defined by libctru.
Result __sync_init(void);
Result __sync_fini(void) __attribute__((weak));
void envDestroyHandles(void);
void mappableInit(u32 start, u32 end);

// The heap of newlib (sbrk) is between these two addresses.
extern char* fake_heap_start;
extern char* fake_heap_end;

// The heap of the 3DS system calls.
extern u32 __ctru_heap;
extern u32 __ctru_heap_size;

// These names stop libctru from using its own files for them.
void (*__system_retAddr)(void);
u32 __apt_appid = 0x300;
u32 __system_runflags = 0;
void __libctru_init(void) {}

//------------------------------------------------------------------------
// The heap
//------------------------------------------------------------------------

/// Gives the heap to newlib. The whole memory that the plugin loader gave to
/// the plugin, after the code and the data, is the heap.
void __system_allocateHeaps(void) {
  __ctru_heap = PLUGIN_HEADER->heapVA;
  __ctru_heap_size = PLUGIN_HEADER->heapSize;

  // The addresses that libctru uses to map memory blocks.
  mappableInit(0x11000000, 0x14000000);

  fake_heap_start = (char*)__ctru_heap;
  fake_heap_end = fake_heap_start + __ctru_heap_size;
}

/// The boot opens no service that it has to close.
void __appExit(void) {}

void __libctru_exit(int rc) {
  (void)rc;
  envDestroyHandles();
  if (__sync_fini) __sync_fini();
}

//------------------------------------------------------------------------
// The thread data of the threads of the game
//------------------------------------------------------------------------

static ThreadVars* g_mainThreadVars;

/// The data of a thread of the game.
typedef struct {
  u32 key; ///< The TLS address of the thread. 0: this entry is free.
  ThreadVars vars;
  struct _reent reent;
  u8 tls[0x80];
} GameThreadData;

#define GAME_THREAD_COUNT 16

static GameThreadData g_gameThreads[GAME_THREAD_COUNT];
static LightLock g_gameThreadsLock;
static bool g_gameThreadsReady;

/// Returns the thread data of a thread that the game made. At the first call
/// for a thread, it makes the data. When the table is full, the thread uses
/// the data of the main thread.
static ThreadVars* GetGameThreadVars(void) {
  const u32 key = (u32)getThreadLocalStorage();
  ThreadVars* result = g_mainThreadVars;

  if (!g_gameThreadsReady) {
    LightLock_Init(&g_gameThreadsLock);
    g_gameThreadsReady = true;
  }
  LightLock_Lock(&g_gameThreadsLock);

  GameThreadData* free_entry = NULL;
  for (u32 i = 0; i < GAME_THREAD_COUNT; i++) {
    if (g_gameThreads[i].key == key) {
      result = &g_gameThreads[i].vars;
      free_entry = NULL;
      goto done;
    }
    if (g_gameThreads[i].key == 0 && free_entry == NULL)
      free_entry = &g_gameThreads[i];
  }

  const size_t tls_size = __tls_end - __tls_start;
  const size_t tls_load_size = __tdata_lma_end - __tdata_lma;
  if (free_entry != NULL && tls_size <= sizeof(free_entry->tls)) {
    ThreadVars* tv = &free_entry->vars;
    u8* tls = (u8*)alignTo((size_t)free_entry->tls, __tdata_align);

    tv->magic = THREADVARS_MAGIC;
    tv->tls_tp = tls - 8; // The ARM ELF ABI puts a header of 8 bytes here.
    tv->reent = &free_entry->reent;
    if (tls_load_size) memcpy(tls, __tdata_lma, tls_load_size);

    // The standard files are the files of the main thread.
    _REENT_INIT_PTR(&free_entry->reent);
    struct _reent* main_reent = g_mainThreadVars->reent;
    free_entry->reent._stdin = main_reent->_stdin;
    free_entry->reent._stdout = main_reent->_stdout;
    free_entry->reent._stderr = main_reent->_stderr;

    free_entry->key = key; // Last: the data is complete.
    result = tv;
  }

done:
  LightLock_Unlock(&g_gameThreadsLock);
  return result;
}

struct _reent* __SYSCALL(getreent)() {
  ThreadVars* tv = getThreadVars();
  if (tv->magic != THREADVARS_MAGIC) return GetGameThreadVars()->reent;
  return tv->reent;
}

void* __attribute__((__used__)) __getThreadLocalStorage(void) {
  ThreadVars* tv = getThreadVars();
  if (tv->magic != THREADVARS_MAGIC) return GetGameThreadVars()->tls_tp;
  return tv->tls_tp;
}

void* __attribute__((__used__)) __attribute__((__naked__)) __aeabi_read_tp() {
  __asm__ __volatile__(
      "stmfd sp!, {r1-r3, lr}        \n"
      "bl __getThreadLocalStorage    \n"
      "ldmfd sp!, {r1-r3, pc}        \n");
}

void initThreadVars(struct Thread_tag* thread) {
  ThreadVars* tv = getThreadVars();
  tv->magic = THREADVARS_MAGIC;
  tv->reent = thread != NULL ? &thread->reent : _impure_ptr;
  tv->thread_ptr = thread;
  // The ARM ELF ABI puts a header of 8 bytes before the TLS.
  tv->tls_tp = (thread != NULL ? (u8*)thread->stacktop : __tls_start) - 8;
  tv->srv_blocking_policy = false;

  // The kernel does not set fpscr: no flag, no interrupt, scalar mode,
  // default NaN, flush to zero, round to nearest.
  __builtin_arm_set_fpscr(BIT(25) | BIT(24) | (0u << 22));
}

void __system_initSyscalls(void) {
  // The thread data of the thread that starts the plugin.
  initThreadVars(NULL);
  g_mainThreadVars = getThreadVars();

  u32 tls_size = __tdata_lma_end - __tdata_lma;
  size_t tdata_start = alignTo((size_t)__tls_start, __tdata_align);
  if (tls_size) memcpy((void*)tdata_start, __tdata_lma, tls_size);
}

//------------------------------------------------------------------------
// The system calls of newlib
//------------------------------------------------------------------------

int __SYSCALL(clock_gettime)(clockid_t clock_id, struct timespec* tp) {
  if (clock_id == CLOCK_REALTIME) {
    if (tp != NULL) {
      // The epoch of the 3DS is 1900, the epoch of the library is 1970.
      s64 ms_since_epoch = osGetTime() - 2208988800000ULL;
      tp->tv_sec = ms_since_epoch / 1000;
      tp->tv_nsec = (ms_since_epoch % 1000) * 1000000;
    }
  } else if (clock_id == CLOCK_MONOTONIC) {
    if (tp != NULL) {
      u64 ticks_since_boot = svcGetSystemTick();
      tp->tv_sec = ticks_since_boot / SYSCLOCK_ARM11;
      tp->tv_nsec =
          ((ticks_since_boot % SYSCLOCK_ARM11) * 1000000000ULL) / SYSCLOCK_ARM11;
    }
  } else {
    errno = EINVAL;
    return -1;
  }
  return 0;
}

int __SYSCALL(clock_getres)(clockid_t clock_id, struct timespec* res) {
  if (clock_id == CLOCK_REALTIME) {
    if (res != NULL) {
      res->tv_sec = 0;
      res->tv_nsec = 1000000;
    }
  } else if (clock_id == CLOCK_MONOTONIC) {
    if (res != NULL) {
      res->tv_sec = 0;
      res->tv_nsec = 1;
    }
  } else {
    errno = EINVAL;
    return -1;
  }
  return 0;
}

int __SYSCALL(gettod_r)(struct _reent* ptr, struct timeval* tp,
                        struct timezone* tz) {
  (void)ptr;
  if (tp != NULL) {
    s64 now = osGetTime() - 2208988800000ULL;
    tp->tv_sec = now / 1000;
    tp->tv_usec = (now - 1000 * tp->tv_sec) * 1000;
  }
  if (tz != NULL) {
    tz->tz_minuteswest = 0;
    tz->tz_dsttime = 0;
  }
  return 0;
}

int __SYSCALL(nanosleep)(const struct timespec* req, struct timespec* rem) {
  (void)rem;
  svcSleepThread(req->tv_sec * 1000000000ull + req->tv_nsec);
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

void __ctru_exit(int rc);

void __SYSCALL(exit)(int rc) { __ctru_exit(rc); }

int __SYSCALL(cond_signal)(_COND_T* cond) {
  CondVar_Signal((CondVar*)cond);
  return 0;
}

int __SYSCALL(cond_broadcast)(_COND_T* cond) {
  CondVar_Broadcast((CondVar*)cond);
  return 0;
}

int __SYSCALL(cond_wait)(_COND_T* cond, _LOCK_T* lock, uint64_t timeout_ns) {
  return CondVar_WaitTimeout((CondVar*)cond, lock, timeout_ns) ? ETIMEDOUT : 0;
}

int __SYSCALL(cond_wait_recursive)(_COND_T* cond, _LOCK_RECURSIVE_T* lock,
                                   uint64_t timeout_ns) {
  if (lock->counter != 1) return EBADF;

  uint32_t thread_tag_backup = lock->thread_tag;
  lock->thread_tag = 0;
  lock->counter = 0;

  int err = CondVar_WaitTimeout((CondVar*)cond, &lock->lock, timeout_ns);

  lock->thread_tag = thread_tag_backup;
  lock->counter = 1;

  return err ? ETIMEDOUT : 0;
}

//------------------------------------------------------------------------
// The threads of the plugin
//------------------------------------------------------------------------

static void ThreadBegin(void* arg) {
  Thread t = (Thread)arg;
  initThreadVars(t);
  t->ep(t->arg);
  threadExit(0);
}

Thread threadCreate(ThreadFunc entrypoint, void* arg, size_t stack_size,
                    int prio, int affinity, bool detached) {
  // The stack must be 8-aligned at least.
  size_t align = __tdata_align > 8 ? __tdata_align : 8;

  size_t stackoffset = alignTo(sizeof(struct Thread_tag), align);
  size_t allocsize = alignTo(stackoffset + stack_size, align);
  size_t tlssize = __tls_end - __tls_start;
  size_t tlsloadsize = __tdata_lma_end - __tdata_lma;
  size_t tbsssize = tlssize - tlsloadsize;

  // memalign needs a size that is a multiple of the alignment.
  size_t size = alignTo(allocsize + tlssize, align);

  // Overflows
  if (allocsize < stackoffset) return NULL;
  if ((allocsize - stackoffset) < stack_size) return NULL;
  if (size < allocsize) return NULL;

  Thread t = (Thread)memalign(align, size);
  if (!t) return NULL;

  t->ep = entrypoint;
  t->arg = arg;
  t->detached = detached;
  t->finished = false;
  t->stacktop = (u8*)t + allocsize;

  // tls_tp must be aligned: after the 8 bytes of the header too.
  size_t tdata_start = 8 + alignTo((size_t)t->stacktop - 8, align);

  if (tlsloadsize) memcpy((void*)tdata_start, __tdata_lma, tlsloadsize);
  if (tbsssize) memset((void*)tdata_start + tlsloadsize, 0, tbsssize);

  // The new thread uses the standard files of the current thread.
  _REENT_INIT_PTR(&t->reent);
  struct _reent* cur = getThreadVars()->reent;
  t->reent._stdin = cur->_stdin;
  t->reent._stdout = cur->_stdout;
  t->reent._stderr = cur->_stderr;

  if (R_FAILED(svcCreateThread(&t->handle, ThreadBegin, (u32)t,
                               (u32*)t->stacktop, prio, affinity))) {
    free(t);
    return NULL;
  }
  return t;
}

Handle threadGetHandle(Thread thread) {
  if (!thread || thread->finished) return ~0UL;
  return thread->handle;
}

int threadGetExitCode(Thread thread) {
  if (!thread || !thread->finished) return 0;
  return thread->rc;
}

void threadFree(Thread thread) {
  if (!thread || !thread->finished) return;
  svcCloseHandle(thread->handle);
  free(thread);
}

Result threadJoin(Thread thread, u64 timeout_ns) {
  if (!thread || thread->finished) return 0;
  return svcWaitSynchronization(thread->handle, timeout_ns);
}

void threadDetach(Thread thread) {
  if (!thread || thread->detached) return;

  if (thread->finished) {
    threadFree(thread);
    return;
  }
  thread->detached = true;
}

Thread threadGetCurrent(void) {
  ThreadVars* tv = getThreadVars();
  // A thread of the game has no thread object.
  if (tv->magic != THREADVARS_MAGIC) return NULL;
  return tv->thread_ptr;
}

void threadExit(int rc) {
  Thread t = threadGetCurrent();
  if (!t) {
    svcBreak(USERBREAK_PANIC);
    for (;;)
      ;
  }

  t->finished = true;
  if (t->detached)
    threadFree(t);
  else
    t->rc = rc;

  svcExitThread();
}

//------------------------------------------------------------------------
// The functions of newlib's pthread layer
//------------------------------------------------------------------------

int __SYSCALL(thread_create)(struct __pthread_t** thread,
                             void* (*func)(void*), void* arg, void* stack_addr,
                             size_t stack_size) {
  if (stack_addr) return EINVAL;
  if (!stack_size) stack_size = 32 * 1024;

  Thread t = threadCreate((ThreadFunc)func, arg, stack_size, 0x3F, 0, false);
  if (t) {
    *thread = (struct __pthread_t*)t;
    return 0;
  }
  return ENOMEM;
}

void* __SYSCALL(thread_join)(struct __pthread_t* thread) {
  threadJoin((Thread)thread, U64_MAX);
  void* rc = (void*)threadGetExitCode((Thread)thread);
  threadFree((Thread)thread);
  return rc;
}

int __SYSCALL(thread_detach)(struct __pthread_t* thread) {
  threadDetach((Thread)thread);
  return 0;
}

void __SYSCALL(thread_exit)(void* value) { threadExit((int)value); }

struct __pthread_t* __SYSCALL(thread_self)(void) {
  return (struct __pthread_t*)threadGetCurrent();
}
