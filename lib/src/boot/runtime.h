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
 * @file runtime.h
 * @brief The thread data that the minimal boot shares with libctru.
 *
 * The code of the plugin runs on the threads of the game. These threads do
 * not have the thread data of libctru. The boot gives them one on the first
 * call (see runtime.c). This file has the structures of that data. They are
 * the structures of libctru, because the library does not export them.
 */

#pragma once

#include <sys/reent.h>
#include <3ds.h>

#ifdef __cplusplus
extern "C" {
#endif

/// The value that marks the thread data of a thread that libctru made.
#define THREADVARS_MAGIC 0x21545624 // !TV$

/// The data of a thread made by threadCreate().
struct Thread_tag {
  Handle handle;
  ThreadFunc ep;
  void* arg;
  int rc;
  bool detached, finished;
  struct _reent reent;
  void* stacktop;
};

/// The data that libctru keeps at the start of the TLS of each thread. It
/// must stay under 0x80 bytes.
typedef struct {
  u32 magic;
  Thread thread_ptr;
  struct _reent* reent;
  void* tls_tp; ///< The offset is used by __aeabi_read_tp.
  u32 fs_magic;
  Handle fs_session;
  bool srv_blocking_policy;
} ThreadVars;

static inline ThreadVars* getThreadVars(void) {
  return (ThreadVars*)getThreadLocalStorage();
}

static inline size_t alignTo(const size_t base, const size_t align) {
  return (base + (align - 1)) & ~(align - 1);
}

/// Gives thread data to the current thread. thread is null for the main
/// thread of the plugin.
void initThreadVars(struct Thread_tag* thread);

extern const size_t __tdata_align;
extern const u8 __tdata_lma[];
extern const u8 __tdata_lma_end[];
extern u8 __tls_start[];
extern u8 __tls_end[];

#ifdef __cplusplus
}
#endif
