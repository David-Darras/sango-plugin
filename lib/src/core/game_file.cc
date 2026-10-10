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
 * @file game_file.cc
 * @brief Reads the files of the archives of the game into the memory of the
 *        plugin.
 *
 * The declarations are in core/game_file.h.
 */

#include "core/game_file.h"

#include <3ds.h>
#include <cstring>

#include "core/native/game_manager.h"
#include "system/address.h"
#include "ui/log_application.h"

namespace core {
namespace {
// The ids of the heaps of the game that exist for all the game.
constexpr s32 kHeapSystem = 0;
constexpr s32 kHeapResident = 4;
constexpr s32 kHeapResidentDevice = 5;

// The function of the table of a heap that gives the size of its largest
// free block.
constexpr u32 kHeapGetAllocatableSize = 0x2C / 4;
// The memory of a heap that the plugin keeps free for the game.
constexpr u32 kHeapReserve = 0x80000;
// The memory that a block uses more than its size (a header, the alignment).
constexpr u32 kHeapMargin = 0x100;
// The heaps that LogHeaps() shows.
constexpr s32 kLoggedHeaps = 24;
// The memory that an archive keeps in a heap (its table of files), and the
// memory that the plugin keeps free for the game in this heap.
constexpr u32 kArchiveMemory = 0x8000;
constexpr u32 kArchiveReserve = 0x8000;
// The size of a page of memory of the system.
constexpr u32 kPageSize = 0x1000;

// An archive object of the game, and the number of archives that the
// plugin keeps open.
constexpr u32 kArchiveSize = 120;
constexpr u32 kMaxArchives = 512;
constexpr u32 kArchiveOpen = 1 << 0;

void* GetHeap(s32 id) {
  return ((void* (*)(s32))sys::address::kGetHeapById)(id);
}

u32 GetAllocatableSize(void* heap) {
  const uptr* table = *(const uptr**)heap;
  const s32 size = ((s32 (*)(void*))table[kHeapGetAllocatableSize])(heap);
  return size > 0 ? (u32)size : 0;
}

// Returns true when the heap can give `size` bytes and keep `reserve`
// bytes free for the game.
bool HasRoom(void* heap, u32 size, u32 reserve = kHeapReserve) {
  return heap != nullptr &&
         GetAllocatableSize(heap) >= size + kHeapMargin + reserve;
}

// Writes the free memory of the first heaps of the game in the log.
void LogHeaps() {
  for (s32 id = 0; id < kLoggedHeaps; id++) {
    void* heap = GetHeap(id);
    if (heap == nullptr) continue;
    ui::LogApplication::Print(u"Heap %ld: %lu KB free", id,
                              GetAllocatableSize(heap) / 1024);
  }
}

void* Allocate(void* heap, u32 size, u32 alignment) {
  return ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(heap, size,
                                                                alignment);
}

// Returns an archive object of the game, open for all the game.
void* OpenArchive(ArchiveId archive_id) {
  static void* archives[kMaxArchives] = {};
  const u32 index = static_cast<u32>(archive_id);
  if (index >= kMaxArchives) return nullptr;
  if (archives[index] != nullptr) return archives[index];

  // The archive keeps its table of files in a heap that exists for all the
  // game: the first heap with free memory.
  void* const heaps[] = {GetHeap(kHeapResident),
                         GameManager::GetInstance().GetSystemHeap(),
                         GetHeap(kHeapSystem)};
  void* heap = nullptr;
  for (void* candidate : heaps) {
    if (HasRoom(candidate, kArchiveMemory, kArchiveReserve)) {
      heap = candidate;
      break;
    }
  }
  if (heap == nullptr) {
    static bool is_logged = false;
    if (!is_logged) {
      is_logged = true;
      LogHeaps();
    }
    return nullptr;
  }
  auto* archive = new u32[kArchiveSize / sizeof(u32)];
  if (archive == nullptr) return nullptr;
  memset(archive, 0, kArchiveSize);
  ((void (*)(void*, void*, u32, u32))sys::address::kArchiveInitialize)(
      archive, heap, index, kArchiveOpen);
  archives[index] = archive;
  return archive;
}

// Decompresses LZ10 or LZ11 data. Returns null on an error.
u8* Decompress(const u8* data, u32 size, u32* out_size) {
  if (size < 4 || (data[0] != 0x10 && data[0] != 0x11)) return nullptr;
  const bool is_lz11 = data[0] == 0x11;
  u32 length = data[1] | (data[2] << 8) | (data[3] << 16);
  u32 in = 4;
  if (length == 0 && size >= 8) {
    length = data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
    in = 8;
  }
  u8* out = new u8[length];
  if (out == nullptr) return nullptr;
  u32 position = 0;
  while (position < length && in < size) {
    const u8 flags = data[in++];
    for (u32 bit = 0; bit < 8 && position < length; bit++) {
      if ((flags & (0x80 >> bit)) == 0) {
        if (in >= size) break;
        out[position++] = data[in++];
        continue;
      }
      // A copy of earlier bytes: a length and a distance.
      if (in + 1 >= size) break;
      u32 count;
      u32 distance;
      const u32 high = data[in] >> 4;
      if (!is_lz11) {
        count = high + 3;
        distance = (((data[in] & 0xF) << 8) | data[in + 1]) + 1;
        in += 2;
      } else if (high == 0) {
        if (in + 2 >= size) break;
        count = (((data[in] & 0xF) << 4) | (data[in + 1] >> 4)) + 0x11;
        distance = (((data[in + 1] & 0xF) << 8) | data[in + 2]) + 1;
        in += 3;
      } else if (high == 1) {
        if (in + 3 >= size) break;
        count = (((data[in] & 0xF) << 12) | (data[in + 1] << 4) |
                 (data[in + 2] >> 4)) +
                0x111;
        distance = (((data[in + 2] & 0xF) << 8) | data[in + 3]) + 1;
        in += 4;
      } else {
        count = high + 1;
        distance = (((data[in] & 0xF) << 8) | data[in + 1]) + 1;
        in += 2;
      }
      if (distance > position) {
        delete[] out;
        return nullptr;
      }
      for (u32 i = 0; i < count && position < length; i++, position++) {
        out[position] = out[position - distance];
      }
    }
  }
  if (position != length) {
    delete[] out;
    return nullptr;
  }
  *out_size = length;
  return out;
}
} // namespace

u8* GameFile::Read(ArchiveId archive_id, u32 file_id, bool compressed,
                   u32* out_size) {
  void* archive = OpenArchive(archive_id);
  if (archive == nullptr) {
    ui::LogApplication::Print(u"Archive %lu: no memory to open it",
                              static_cast<u32>(archive_id));
    return nullptr;
  }
  const u32 size = ((u32 (*)(void*, u32))sys::address::kArchiveGetFileSize)(
      archive, file_id);
  if (size == 0) return nullptr;
  u8* file = new u8[size];
  if (file == nullptr) return nullptr;
  ((void (*)(void*, u32, u32, u32, void*))sys::address::kArchiveLoadFile)(
      archive, file_id, 0, size, file);

  u32 file_size = size;
  if (compressed) {
    u8* data = Decompress(file, size, &file_size);
    delete[] file;
    if (data == nullptr) return nullptr;
    file = data;
  }
  if (out_size != nullptr) *out_size = file_size;
  return file;
}

u32 GameFile::GetFreeHeapMemory(s32 heap_id) {
  void* heap = GetHeap(heap_id);
  return heap != nullptr ? GetAllocatableSize(heap) : 0;
}

void* GameFile::AllocateDevice(u32 size, u32 alignment) {
  // New linear memory of the system first: it is not in a heap of the game,
  // so the game keeps all its memory. Its pages are aligned on 4 KB.
  const u32 pages = (size + kPageSize - 1) & ~(kPageSize - 1);
  u32 address = 0;
  const Result result =
      alignment <= kPageSize
          ? svcControlMemory(&address, 0, 0, pages, MEMOP_ALLOC_LINEAR,
                             (MemPerm)(MEMPERM_READ | MEMPERM_WRITE))
          : -1;
  static bool is_logged = false;
  if (!is_logged) {
    is_logged = true;
    ui::LogApplication::Print(u"Linear memory: result %08lX, address %08lX",
                              (u32)result, address);
  }
  if (R_SUCCEEDED(result) && address != 0) return (void*)address;

  // Else a resident heap of the game for the GPU, when it keeps its reserve.
  void* const heaps[] = {GetHeap(kHeapResidentDevice),
                         GameManager::GetInstance().GetDeviceHeap()};
  for (void* heap : heaps) {
    if (HasRoom(heap, size + alignment)) return Allocate(heap, size, alignment);
  }
  static bool is_reported = false;
  if (!is_reported) {
    is_reported = true;
    ui::LogApplication::Print(
        u"GPU memory: %lu KB, %lu KB free (%lu KB needed)",
        heaps[0] != nullptr ? GetAllocatableSize(heaps[0]) / 1024 : 0,
        heaps[1] != nullptr ? GetAllocatableSize(heaps[1]) / 1024 : 0,
        (size + kHeapReserve) / 1024);
  }
  return nullptr;
}
} // namespace core
