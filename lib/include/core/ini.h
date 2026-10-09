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
 * @file ini.h
 * @brief A small INI file on the SD card: the player can edit it in a text
 *        editor.
 */

#pragma once

#include "common.h"

namespace core {
/**
 * @brief Reads and writes a small INI file (UTF-8).
 *
 * The format: `[section]` lines, `key = value` lines, and comment lines that
 * start with `;` or `#`. The keys do not use the case of the letters.
 *
 * @code
 * static core::Ini ini;  // 8 KB: do not put it on the stack.
 * if (ini.Load(u"sdmc:/sango/menu.ini")) {
 *   bool shadow = ini.GetBool("menu", "text_shadow", true);
 * }
 * ini.Clear();
 * ini.AddSection("menu");
 * ini.SetInt("text_shadow", 1);
 * ini.Save(u"sdmc:/sango/menu.ini");
 * @endcode
 */
class Ini {
public:
  /// The maximum size of the file, in bytes.
  static constexpr u32 kMaxSize = 6144;
  /// The maximum number of values.
  static constexpr u32 kMaxValues = 160;

  Ini() : size_(0), count_(0) { text_[0] = '\0'; }

  /// Reads a file. Returns false when the file does not exist or is empty.
  bool Load(const c16* path);

  /// Returns the value of a key, or `fallback` when the key does not exist.
  const c8* Get(const c8* section, const c8* key,
                const c8* fallback = nullptr) const;
  /// Returns the value of a key as an integer (decimal, or hexadecimal with
  /// 0x).
  s32 GetInt(const c8* section, const c8* key, s32 fallback) const;
  /// Returns the value of a key as On / Off: 1, on, true or yes are true.
  bool GetBool(const c8* section, const c8* key, bool fallback) const;
  /// Returns the value of a key as a hexadecimal number without 0x.
  u32 GetHex(const c8* section, const c8* key, u32 fallback) const;

  /// Removes all the values. Use it before you write a new file.
  void Clear();
  /// Adds a comment line (without the `;`).
  void AddComment(const c8* text);
  /// Adds a `[section]` line. The next values go into this section.
  void AddSection(const c8* name);
  /// Adds a `key = value` line.
  void Set(const c8* key, const c8* value);
  /// Adds a `key = value` line with an integer.
  void SetInt(const c8* key, s32 value);
  /// Adds a `key = value` line with a hexadecimal number (8 digits).
  void SetHex(const c8* key, u32 value);
  /// Writes the file. Returns false when the text is too long.
  bool Save(const c16* path) const;

  /// Returns the text that Save() writes.
  const c8* GetText() const { return text_; }
  /// Returns the size of the text, in bytes.
  u32 GetSize() const { return size_; }

private:
  struct Value {
    const c8* section;
    const c8* key;
    const c8* value;
  };

  /// Adds text at the end of the buffer.
  void Append(const c8* text);

  c8 text_[kMaxSize];
  u32 size_;
  Value values_[kMaxValues];
  u32 count_;
};
} // namespace core
