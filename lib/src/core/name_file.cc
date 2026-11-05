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
 * @file name_file.cc
 * @brief A list of names that the plugin reads from the SD card.
 *
 * The declarations are in core/name_file.h.
 */

#include "core/name_file.h"

#include <cstdlib>
#include <cstring>

#include "system/native/file.h"

namespace core {

namespace {
// The size of a list file at most, in bytes.
constexpr u32 kMaxFileSize = 128 * 1024;
// The number of names of a list at most.
constexpr u32 kMaxNames = 20000;
// The size of the beginning of a file: its size and the number of names.
constexpr u32 kHeaderSize = 8;

// Makes the path "sdmc:/sango/names/<kind>.bin".
bool MakePath(const c8* kind, c16* path, u32 capacity) {
  static const c16 kPrefix[] = u"sdmc:/sango/names/";
  static const c16 kSuffix[] = u".bin";
  u32 length = 0;
  for (const c16* p = kPrefix; *p != 0; p++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *p;
  }
  for (const c8* p = kind; *p != 0; p++) {
    if (length + 1 >= capacity) return false;
    path[length++] = static_cast<c16>(static_cast<u8>(*p));
  }
  for (const c16* p = kSuffix; *p != 0; p++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *p;
  }
  path[length] = 0;
  return true;
}

// The letters and the digits of a name, in lowercase.
c8 Simplify(c8 letter) {
  if (letter >= 'A' && letter <= 'Z') return static_cast<c8>(letter + 32);
  return letter;
}

bool IsSignificant(c8 letter) {
  return (letter >= 'a' && letter <= 'z') || (letter >= '0' && letter <= '9');
}

// Compares two names without the case, the spaces and the signs.
bool SameName(const c8* a, const c8* b) {
  while (true) {
    while (*a != 0 && !IsSignificant(Simplify(*a))) a++;
    while (*b != 0 && !IsSignificant(Simplify(*b))) b++;
    if (*a == 0 || *b == 0) return *a == *b;
    if (Simplify(*a) != Simplify(*b)) return false;
    a++;
    b++;
  }
}
} // namespace

bool NameFile::Load(const c8* kind) {
  Unload();
  c16 path[96];
  if (!MakePath(kind, path, sizeof(path) / sizeof(path[0]))) return false;
  sys::File file;
  file.Open(path, sys::File::kRead);
  if (!file.IsOpen()) return false;

  u32 header[2] = {};
  if (file.Read(header, kHeaderSize) != static_cast<s32>(kHeaderSize)) {
    return false;
  }
  const u32 size = header[0];
  const u32 count = header[1];
  if (size < kHeaderSize + 4 * count || size > kMaxFileSize ||
      count > kMaxNames) {
    return false;
  }
  u8* data = static_cast<u8*>(std::malloc(size));
  if (data == nullptr) return false;
  std::memcpy(data, header, kHeaderSize);
  const u32 rest = size - kHeaderSize;
  if (rest != 0 && file.Read(data + kHeaderSize, rest) !=
                       static_cast<s32>(rest)) {
    std::free(data);
    return false;
  }

  // The offsets become addresses.
  u32* offsets = reinterpret_cast<u32*>(data + kHeaderSize);
  for (u32 i = 0; i < count; i++) {
    if (offsets[i] >= size) {
      std::free(data);
      return false;
    }
    offsets[i] += reinterpret_cast<u32>(data);
  }
  data[size - 1] = 0;  // The last name ends.
  data_ = data;
  names_ = reinterpret_cast<const c8**>(offsets);
  count_ = count;
  return true;
}

void NameFile::Unload() {
  std::free(data_);
  data_ = nullptr;
  names_ = nullptr;
  count_ = 0;
}

const c8* NameFile::Get(u32 value) const {
  if (names_ == nullptr || value >= count_) return "";
  return names_[value];
}

bool NameFile::Find(const c8* text, u32* value) const {
  for (u32 i = 0; i < count_; i++) {
    if (names_[i][0] != 0 && SameName(names_[i], text)) {
      *value = i;
      return true;
    }
  }
  return false;
}

} // namespace core
