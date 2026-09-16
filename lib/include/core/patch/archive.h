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

#include "common.h"
#include "core/constant/archive_id.h"
#include "core/native/archive_input.h"

namespace core {

class Archive {
  MAKE_SINGLETON(Archive)

public:

  /// Lets a product redirect a file about to be streamed from an archive
  /// (see IsArchive to tell archives apart); returns the file id to use.
  typedef u32 (*StreamFileCallback)(const u32* archive, u32 file_id);
  /// Same for the queued reads; the input can be rewritten in place.
  typedef void (*ReadFileCallback)(ArchiveInput* input);

  StreamFileCallback on_stream_file = nullptr;
  ReadFileCallback on_read_file = nullptr;

  static void Initialize();

  static bool IsArchive(const u32* archive_data, const ArchiveId archive_id);
  static bool IsArchive(const ArchiveInput* input, const ArchiveId archive_id);

private:
  static void LoadDataHook(uptr self, u32 id, uptr heap, uptr buffer,
                           uptr buffer_size, u32* size);
  static bool ReadFileAsync2(u32* archive, void* heap, u32 file_id,
                             void* buffer, u32 p4, u32 p5, u32 p6);
  static bool ReadFileAsync(void* file_manager, ArchiveInput* input);
};

} // namespace core
