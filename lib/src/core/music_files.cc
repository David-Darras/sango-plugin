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
 * @file music_files.cc
 * @brief Plays the music files of the SD card instead of the music files of
 *        the game.
 *
 * The declarations are in core/patch/music_files.h.
 */

#include "core/patch/music_files.h"
#include <cstdio>
#include <3ds.h>
#include <cstring>
#include "core/hook.h"
#include "system/address.h"
#include "system/native/file.h"
#include "system/native/sound.h"

namespace core {
namespace {

constexpr const char* kFolder = "sdmc:/sango/bgm/";
constexpr u32 kFolderLength = 16;
constexpr const char* kExtension = ".dspadpcm.bcstm";
constexpr u32 kPathCapacity = 128;
constexpr u32 kListCapacity = 32 * 1024;
// The music that the game starts when PlayFile() runs. Its own file is not
// read: the hook gives another file. Two musics, because the game does not
// restart the music that it already plays.
constexpr BackgroundMusicId kCarriers[2] = {BackgroundMusicId::kDefault,
                                            BackgroundMusicId::kBicycle};
// PlayFile() asks for a file. If the game does not start a music before this
// time, the request ends.
constexpr u64 kPendingTicks = 3ull * SYSCLOCK_ARM11;

// Returns the file name at the end of a path.
const char* GetFileName(const char* path) {
  const char* name = path;
  for (const char* c = path; *c != '\0'; c++) {
    if (*c == '/') name = c + 1;
  }
  return name;
}

// Returns true when a text starts with a prefix.
bool StartsWith(const char* text, const char* prefix) {
  return std::strncmp(text, prefix, std::strlen(prefix)) == 0;
}

// Returns true when the SD card has this file.
bool Exists(const char* path) {
  c16 wide[kPathCapacity];
  u32 i = 0;
  for (; path[i] != '\0' && i + 1 < kPathCapacity; i++) {
    wide[i] = static_cast<c16>(static_cast<u8>(path[i]));
  }
  wide[i] = 0;
  sys::File file;
  file.Open(wide, sys::File::kRead);
  if (!file.IsOpen()) return false;
  file.Close();
  return true;
}

} // namespace

u32 MusicFiles::LoadList() {
  // The text stays in memory only while the function runs.
  char* text = new char[kListCapacity];
  count_ = 0;
  names_[0][0] = '\0';
  titles_[0][0] = '\0';
  const u32 size = sys::File::ReadAll(u"sdmc:/sango/bgm/list.txt", text,
                                      kListCapacity - 1);
  text[size] = '\0';
  // A line is "name|title". `part` 0 reads the name, 1 reads the title.
  u32 length = 0;
  u32 part = 0;
  for (u32 i = 0; i <= size && count_ < kMaxFiles; i++) {
    const char c = text[i];
    if (c == '\n' || c == '\r' || c == '\0') {
      if (names_[count_][0] != '\0') {
        if (part == 0) std::strcpy(titles_[count_], names_[count_]);
        count_++;
      }
      length = 0;
      part = 0;
      if (count_ < kMaxFiles) {
        names_[count_][0] = '\0';
        titles_[count_][0] = '\0';
      }
    } else if (c == '|' && part == 0) {
      part = 1;
      length = 0;
    } else if (part == 0 && length + 1 < kNameCapacity) {
      names_[count_][length++] = c;
      names_[count_][length] = '\0';
    } else if (part == 1 && length + 1 < kTitleCapacity) {
      titles_[count_][length++] = c;
      titles_[count_][length] = '\0';
    }
  }
  delete[] text;
  return count_;
}

void MusicFiles::PlayFile(u32 index) {
  if (index >= count_) return;
  std::strcpy(pending_, names_[index]);
  has_pending_ = true;
  pending_tick_ = svcGetSystemTick();
  use_second_carrier_ = !use_second_carrier_;
  sys::Sound::PlayBackgroundMusic(kCarriers[use_second_carrier_ ? 1 : 0]);
}

const char* MusicFiles::TakePendingName() {
  if (!has_pending_) return nullptr;
  has_pending_ = false;
  if (svcGetSystemTick() - pending_tick_ > kPendingTicks) return nullptr;
  return pending_;
}

// The game opens a file of the sound archive by its path, for example
// "rom:/sound/bgm_sg_ajito.dspadpcm.bcstm". PlayFile() asks for a file of the
// list for the next music: the game opens it in place of the music file.
// Without it, a file of the SD card with the same name replaces the file of
// the game.
HOOK(void*, OpenStreamFile,
     (void* stream, const char* path, u32 begin, u32 length, bool is_tryable,
      void* callback, void* argument),
     sys::address::kSoundOpenStreamFile) {
  auto& files = MusicFiles::GetInstance();
  if (path != nullptr && files.is_enabled) {
    static char redirected[kPathCapacity];
    const char* name = GetFileName(path);
    bool found = false;
    if (StartsWith(name, "bgm_")) {
      if (const char* pending = files.TakePendingName()) {
        std::snprintf(redirected, sizeof(redirected), "%s%s%s", kFolder,
                      pending, kExtension);
        found = Exists(redirected);
      }
    }
    if (!found && std::strlen(name) + kFolderLength < kPathCapacity) {
      std::snprintf(redirected, sizeof(redirected), "%s%s", kFolder, name);
      found = Exists(redirected);
    }
    if (found) {
      return original(stream, redirected, begin, length, is_tryable, callback,
                      argument);
    }
  }
  return original(stream, path, begin, length, is_tryable, callback, argument);
}

} // namespace core
