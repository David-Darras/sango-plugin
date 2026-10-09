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
 * @file archive.h
 * @brief Lets a product change the files that the game reads from its archives.
 *
 * @see docs/tutorials/10-replace-models.md
 */

#pragma once

#include "common.h"
#include "core/constant/archive_id.h"
#include "core/native/archive_input.h"

namespace core {

/**
 * @brief Calls the callbacks of the product before the game reads a file.
 *
 * The game reads files in two ways: with a stream and with a queued read.
 * Set the two callbacks.
 */
class Archive {
  MAKE_SINGLETON(Archive)

public:

  /// Called before the game streams a file of an archive. Returns the id of
  /// the file to load. Use IsArchive() to find the archive.
  typedef u32 (*StreamFileCallback)(const u32* archive, u32 file_id);
  /// Called before the game runs a queued read. Change `input->file_id` to
  /// load a different file.
  typedef void (*ReadFileCallback)(ArchiveInput* input);

  StreamFileCallback on_stream_file = nullptr;
  ReadFileCallback on_read_file = nullptr;

  static void Initialize();

  /// Returns true when the archive of a stream is `archive_id`.
  static bool IsArchive(const u32* archive_data, const ArchiveId archive_id);
  /// Returns true when the archive of a queued read is `archive_id`.
  static bool IsArchive(const ArchiveInput* input, const ArchiveId archive_id);

  /// Returns the size of a file, without the changes of the plugin.
  static u32 GetOriginalFileSize(u32* archive, u32 file_id);
  /// Reads a file into `buffer`, without the changes of the plugin.
  static u32 LoadOriginalData(u32* archive, u32 file_id, void* buffer);

private:
  static void* LoadCompressedHook(u32* archive, u32 file_id, void* heap_work,
                                  void* heap_data, s32 align, u32* out_size);
  static u32 GetFileSizeHook(u32* archive, u32 file_id);
  static u32 GetFileSizeHook2(u32* archive, u32 file_id);
  static void GetInfoHook(u32* archive, u32 file_id, u32* info);
  static void ReadHook(u32* archive, u32 offset, u32 size,
                                void* buffer, u32* read);
  static u32 LoadDataHook2(u32* archive, u32 file_id, void* buffer);
  static bool ReadFileAsync2(u32* archive, void* heap, u32 file_id,
                             void* buffer, u32 p4, u32 p5, u32 p6);
  static bool ReadFileAsync(void* file_manager, ArchiveInput* input);
};

} // namespace core
