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
 * @file ini.cc
 * @brief A small INI file on the SD card.
 *
 * The declarations are in core/ini.h.
 */

#include "core/ini.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "system/native/file.h"

namespace core {
namespace {
bool IsSpace(c8 c) { return c == ' ' || c == '\t' || c == '\r'; }

// Removes the spaces at the start and at the end of a text.
c8* Trim(c8* text) {
  while (IsSpace(*text)) text++;
  c8* end = text + strlen(text);
  while (end > text && IsSpace(end[-1])) end--;
  *end = '\0';
  return text;
}

c8 ToLower(c8 c) { return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c; }

// Compares two texts without the case of the letters.
bool IsSame(const c8* a, const c8* b) {
  if (a == nullptr || b == nullptr) return a == b;
  while (*a != '\0' && ToLower(*a) == ToLower(*b)) {
    a++;
    b++;
  }
  return ToLower(*a) == ToLower(*b);
}
} // namespace

bool Ini::Load(const c16* path) {
  Clear();
  const u32 size = sys::File::ReadAll(path, text_, kMaxSize - 1);
  if (size == 0) return false;
  text_[size] = '\0';
  size_ = size;

  const c8* section = "";
  c8* line = text_;
  // Skip the UTF-8 mark of some text editors.
  if ((u8)line[0] == 0xEF && (u8)line[1] == 0xBB && (u8)line[2] == 0xBF) {
    line += 3;
  }
  while (line != nullptr && *line != '\0') {
    c8* next = strchr(line, '\n');
    if (next != nullptr) *next++ = '\0';
    c8* text = Trim(line);
    line = next;

    if (*text == '\0' || *text == ';' || *text == '#') continue;
    if (*text == '[') {
      c8* end = strchr(text, ']');
      if (end != nullptr) *end = '\0';
      section = Trim(text + 1);
      continue;
    }
    c8* equal = strchr(text, '=');
    if (equal == nullptr || count_ >= kMaxValues) continue;
    *equal = '\0';
    values_[count_].section = section;
    values_[count_].key = Trim(text);
    values_[count_].value = Trim(equal + 1);
    count_++;
  }
  return count_ > 0;
}

const c8* Ini::Get(const c8* section, const c8* key,
                   const c8* fallback) const {
  for (u32 i = 0; i < count_; i++) {
    if (IsSame(values_[i].section, section) && IsSame(values_[i].key, key)) {
      return values_[i].value;
    }
  }
  return fallback;
}

s32 Ini::GetInt(const c8* section, const c8* key, s32 fallback) const {
  const c8* value = Get(section, key);
  if (value == nullptr || *value == '\0') return fallback;
  return (s32)strtol(value, nullptr, 0);
}

bool Ini::GetBool(const c8* section, const c8* key, bool fallback) const {
  const c8* value = Get(section, key);
  if (value == nullptr || *value == '\0') return fallback;
  return IsSame(value, "1") || IsSame(value, "on") || IsSame(value, "true") ||
         IsSame(value, "yes");
}

u32 Ini::GetHex(const c8* section, const c8* key, u32 fallback) const {
  const c8* value = Get(section, key);
  if (value == nullptr || *value == '\0') return fallback;
  return (u32)strtoul(value, nullptr, 16);
}

void Ini::Clear() {
  size_ = 0;
  count_ = 0;
  text_[0] = '\0';
}

void Ini::Append(const c8* text) {
  const u32 length = strlen(text);
  if (size_ + length >= kMaxSize) return;
  memcpy(text_ + size_, text, length + 1);
  size_ += length;
}

void Ini::AddComment(const c8* text) {
  Append("; ");
  Append(text);
  Append("\r\n");
}

void Ini::AddSection(const c8* name) {
  if (size_ != 0) Append("\r\n");
  Append("[");
  Append(name);
  Append("]\r\n");
}

void Ini::Set(const c8* key, const c8* value) {
  Append(key);
  Append(" = ");
  Append(value);
  Append("\r\n");
}

void Ini::SetInt(const c8* key, s32 value) {
  c8 text[16];
  snprintf(text, sizeof(text), "%ld", (long)value);
  Set(key, text);
}

void Ini::SetHex(const c8* key, u32 value) {
  c8 text[16];
  snprintf(text, sizeof(text), "%08lX", (unsigned long)value);
  Set(key, text);
}

bool Ini::Save(const c16* path) const {
  if (size_ + 1 >= kMaxSize) return false;
  sys::File file(path, true);
  if (!file.IsOpen()) return false;
  file.Write(text_, size_);
  return true;
}
} // namespace core
