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
#include "hook_manager.h"
#include "kaizo.h"
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
  }

  struct Input {
    u8 priority;
    u32 archive_id;
    u32 file_id;
    bool is_compressed;
    uptr heap[4];
    uptr buffer;
    u32* size;
  };

  static bool ReadFileAsync2(u32* archive, void* heap, u32 file_id,
                             void* buffer,
                             u32 p4, u32 p5, u32 p6) {
    u32* archive_table = (u32*)ADDRESS_ARCHIVE_FILENAME_TABLE;
#ifdef KAIZO
    if (archive[12] == archive_table[ARCHIVE_OVERWORLD_MODEL]) {
      file_id = kaizo::PatchOverworldModels(file_id, true);
    }
#endif
    return HookManager::Call<bool>(HookID::kReadFileAsync2, archive, heap,
                                   file_id,
                                   buffer, p4, p5, p6);
  }

  static bool ReadFileAsync(void* file_manager, Input* input) {
#ifdef KAIZO
    if (input->archive_id == ARCHIVE_OVERWORLD_MODEL) {
      input->file_id = kaizo::PatchOverworldModels(input->file_id, false);
    }
    if (input->archive_id == ARCHIVE_PLAYER_ICON) {
      input->file_id = 72; // STEVEN
    }
#endif
    return HookManager::Call<bool>(HookID::kReadFileAsync, file_manager, input);
  }
};
}