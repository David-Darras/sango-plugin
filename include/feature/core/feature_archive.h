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

#pragma once
#include "archive.h"
#include "common.h"
#include "feature/core/hook_manager.h"
#include "kaizo/kaizo.h"
#include "ui/log_application.h"

namespace feature {
class ArchivePatch {
  MAKE_SINGLETON(ArchivePatch)
public:
  STATIC_INLINE void Initiliaze() {
    HookManager::Initialize(HookID::kReadFileAsync,
                            ADDRESS_ARCHIVE_READ_FILE_ASYNC,
                            (uptr)ReadFileAsync);
    HookManager::Initialize(HookID::kReadFileAsync2,
                            ADDRESS_ARCHIVE_READ_FILE_ASYNC_2,
                            (uptr)ReadFileAsync2);
    // HookManager::Initialize(HookID::kReadMapFile,
    //                         0x003A0C44,
    //                         (uptr)ReadMapFile);
  }

  struct Input {
    u8 priority;
    ArchiveID archive_id;
    u32 file_id;
    bool is_compressed;
    uptr heap[4];
    uptr buffer;
    u32* size;
  };

  // static u32 ReadMapFile(u32 self, u32 file_id, u32 x, u32 y) {
  //   ui::LogApplication::Print(u"map=%d", file_id);
  //   return HookManager::Call<u32>(HookID::kReadMapFile, self, file_id,
  //                                  x, y);
  // }

  STATIC_INLINE bool IsArchive(const u32* archive_data,
                               const ArchiveID archive_id) {
    u32* archive_table = (u32*)ADDRESS_ARCHIVE_FILENAME_TABLE;
    return archive_data[12] == archive_table[static_cast<u32>(archive_id)];
  }

  STATIC_INLINE bool IsArchive(const Input* input, const ArchiveID archive_id) {
    return input->archive_id == archive_id;
  }

  static bool ReadFileAsync2(u32* archive, void* heap, u32 file_id,
                             void* buffer,
                             u32 p4, u32 p5, u32 p6) {
#ifdef KAIZO
    if (IsArchive(archive, ArchiveID::kOverworldModel)) {
      file_id = kaizo::PatchOverworldModels(file_id, true);
    }
#endif
    return HookManager::Call<bool>(HookID::kReadFileAsync2, archive, heap,
                                   file_id,
                                   buffer, p4, p5, p6);
  }

  static bool ReadFileAsync(void* file_manager, Input* input) {
#ifdef KAIZO
    if (IsArchive(input, ArchiveID::kOverworldModel)) {
      input->file_id = kaizo::PatchOverworldModels(input->file_id, false);
    }
    if (IsArchive(input, ArchiveID::kPlayerIcon)) {
      input->file_id = 72; // STEVEN
    }
#endif
    return HookManager::Call<bool>(HookID::kReadFileAsync, file_manager, input);
  }
};
}