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
#include "core/fixed_list.h"
#include "core/constant/archive_id.h"
#include "core/native/archive_input.h"

namespace core {

/**
 * @brief Changes the files that the game reads from its archives.
 *
 * To read a different file always, use ReplaceFile():
 *
 * @code
 * // The player looks like a Team Aqua Grunt.
 * core::Archive::ReplaceFile(ArchiveId::kOverworldModel, ModelId::kBrendan,
 *                            ModelId::kTeamAquaGruntMale);
 * @endcode
 *
 * For a change that depends on the moment (for example on the title screen
 * only), use the callbacks. The game reads files in two ways: with a stream
 * and with a queued read. Set the two callbacks. ReplaceFile() applies before
 * the callbacks.
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

  /// The number of replacements of ReplaceFile().
  static constexpr u32 kMaxReplacements = 256;

  static void Initialize();

  /// Returns true when a file of the SD card replaces or adds the file
  /// `file_id` of the archive (for example the files of sdmc:/sango/pokemodel).
  /// The game reads a file that is not in the archive and not on the SD card
  /// without an answer: check it before you use a new file number.
  static bool HasSdFile(ArchiveId archive_id, u32 file_id);

  /**
   * @brief Makes the game read the file `to` when it reads the file `from` of
   *        an archive. The change also applies to the reads of a whole file
   *        (the sequences and the camera clips of the battles, for example).
   *        Call it in SetUpHack(), or at any time for a temporary change.
   * @return false when the list is full (see the problems screen).
   */
  static bool ReplaceFile(ArchiveId archive_id, u32 from, u32 to);
  /// ReplaceFile() with ids of an enum: ModelId, SpeciesId...
  template <typename Id>
  static bool ReplaceFile(ArchiveId archive_id, Id from, Id to) {
    return ReplaceFile(archive_id, static_cast<u32>(from), static_cast<u32>(to));
  }
  /// Removes all the replacements of ReplaceFile().
  static void ClearReplacements();
  /// Removes the replacement of the file `from` of an archive.
  static void RemoveReplacement(ArchiveId archive_id, u32 from);

  /// Makes the game read a BCLIM file of the SD card as the file `file_id` of
  /// the archive of the item icons. The file id must be above the files of the
  /// archive (631 files). Returns false when the file is not an image.
  static bool RegisterItemIcon(u32 file_id, const c16* path);

  /// The size of the file of the data of an item, in bytes. The file has no
  /// compression.
  static constexpr u32 kItemDataFileSize = 36;

  /// Makes the game read `data` as the file of the item `item_id` in the
  /// archive of the item data. The game then sees the data that the plugin
  /// changed (for example "use in the Bag"). `item_id` must be above the files
  /// of the archive.
  static bool RegisterItemData(u32 item_id, const u8* data);

  /// Returns true when the archive of a stream is `archive_id`.
  static bool IsArchive(const u32* archive_data, const ArchiveId archive_id);
  /// Returns true when the archive of a queued read is `archive_id`.
  static bool IsArchive(const ArchiveInput* input, const ArchiveId archive_id);

  /// Returns the size of a file, without the changes of the plugin.
  static u32 GetOriginalFileSize(u32* archive, u32 file_id);
  /// Reads a file into `buffer`, without the changes of the plugin.
  static u32 LoadOriginalData(u32* archive, u32 file_id, void* buffer);

private:
  /// A file that ReplaceFile() replaces.
  struct Replacement {
    ArchiveId archive_id;
    u32 from;
    u32 to;
  };

  /// Returns the file of ReplaceFile() for a stream, or `file_id`.
  static u32 ReplacedStreamFile(const u32* archive, u32 file_id);
  /// Returns the file of ReplaceFile() for a queued read, or `file_id`.
  static u32 ReplacedQueuedFile(const ArchiveInput* input, u32 file_id);

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

  FixedList<Replacement, kMaxReplacements> replacements_;
};

} // namespace core
