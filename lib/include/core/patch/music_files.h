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
 * @file music_files.h
 * @brief Plays the music files of the SD card instead of the music files of
 *        the game.
 */

#pragma once

#include "common.h"

namespace core {

/**
 * @brief Replaces a music file of the game by a file of the SD card.
 *
 * The game plays its music from streams (".bcstm" files) of the folder
 * "sound" of the ROM. The folder `sdmc:/sango/bgm/` has the files of the SD
 * card. They must be DSP-ADPCM ".bcstm" files, as the files of the game.
 * There are two ways to use them:
 *
 * - Same name: when a file has the name of a music file of the game, for
 *   example `bgm_sg_ajito.dspadpcm.bcstm`, the game plays it in place of the
 *   file of the game.
 * - PlayFile(): plays any file of the list `sdmc:/sango/bgm/list.txt`. The
 *   list has one line for each file: the name of the file without
 *   ".dspadpcm.bcstm", then "|", then the title that the menu shows. Without
 *   "|", the title is the name.
 *
 * The hook is automatic: plugin::InitializeEngine() installs it.
 */
class MusicFiles {
  MAKE_SINGLETON(MusicFiles)

public:
  static constexpr u32 kMaxFiles = 400;
  static constexpr u32 kNameCapacity = 40;
  static constexpr u32 kTitleCapacity = 48;

  /// false: the game plays only its own files.
  bool is_enabled = true;

  /// Reads the list of the SD card. Returns the number of names.
  u32 LoadList();

  /// Returns the number of names of the list.
  u32 GetCount() const { return count_; }

  /// Returns the name of a file of the list, or an empty text.
  const char* GetName(u32 index) const {
    return index < count_ ? names_[index] : "";
  }

  /// Returns the title of a file of the list, or an empty text.
  const char* GetTitle(u32 index) const {
    return index < count_ ? titles_[index] : "";
  }

  /// Plays a file of the list. The game starts a music of its own, then
  /// reads the file of the list in place of the file of this music. The
  /// game does not restart the music that it already plays, so each call
  /// uses a different music of the game than the last call.
  void PlayFile(u32 index);

  /// Returns the name that the next music of the game must read, then
  /// forgets it. Returns nullptr when there is none, or when PlayFile() ran
  /// more than 3 seconds ago. The hook calls it.
  const char* TakePendingName();

private:
  u32 count_ = 0;
  char names_[kMaxFiles][kNameCapacity] = {};
  char titles_[kMaxFiles][kTitleCapacity] = {};
  char pending_[kNameCapacity] = {};
  u64 pending_tick_ = 0; ///< The time of PlayFile().
  bool has_pending_ = false;
  bool use_second_carrier_ = false;
};

} // namespace core
