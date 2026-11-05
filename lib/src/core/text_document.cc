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
 * @file text_document.cc
 * @brief A text file of Sango Maker in memory, for the menus that change it.
 *
 * The declarations are in core/text_document.h.
 */

#include "core/text_document.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "core/data_folder.h"
#include "core/heap_buffer.h"

namespace core {

namespace {
// Makes the path of a file of the data folder, with an end.
bool MakePath(const c8* name, const c8* extra, c16* path, u32 capacity) {
  static const c16 kFolder[] = SANGO_DATA_FOLDER u"/";
  u32 length = 0;
  for (const c16* p = kFolder; *p != 0; p++) {
    if (length + 1 >= capacity) return false;
    path[length++] = *p;
  }
  for (const c8* p = name; *p != 0; p++) {
    if (length + 1 >= capacity) return false;
    path[length++] = static_cast<c16>(static_cast<u8>(*p));
  }
  for (const c8* p = extra; *p != 0; p++) {
    if (length + 1 >= capacity) return false;
    path[length++] = static_cast<c16>(static_cast<u8>(*p));
  }
  path[length] = 0;
  return true;
}

bool IsSpace(c8 letter) { return letter == ' ' || letter == '\t'; }

c8 Lower(c8 letter) {
  return (letter >= 'A' && letter <= 'Z') ? static_cast<c8>(letter + 32)
                                          : letter;
}

// The start and the end of the text of a line, without the spaces at the ends.
void Trim(const c8* line, u32* start, u32* end) {
  u32 a = 0;
  u32 b = static_cast<u32>(std::strlen(line));
  while (a < b && IsSpace(line[a])) a++;
  while (b > a && IsSpace(line[b - 1])) b--;
  *start = a;
  *end = b;
}

// The length of a text, kMaxLine - 1 at most.
u32 Length(const c8* text) {
  u32 length = 0;
  while (length < TextDocument::kMaxLine - 1 && text[length] != 0) length++;
  return length;
}

// Compares a key with the start of a line (the case does not count).
bool SameKey(const c8* line, const c8* key) {
  u32 start = 0;
  u32 end = 0;
  Trim(line, &start, &end);
  u32 i = 0;
  while (key[i] != 0) {
    if (start + i >= end || Lower(line[start + i]) != Lower(key[i])) {
      return false;
    }
    i++;
  }
  u32 after = start + i;
  while (after < end && IsSpace(line[after])) after++;
  return after < end && line[after] == '=';
}
} // namespace

bool TextDocument::Load(const c8* file_name) {
  Unload();
  std::strncpy(file_name_, file_name, sizeof(file_name_) - 1);
  c16 path[96];
  if (!MakePath(file_name, "", path, sizeof(path) / sizeof(path[0]))) {
    return false;
  }
  text_ = static_cast<c8*>(std::malloc(kMaxText + 2));
  if (text_ == nullptr) return false;
  const u32 read = sys::File::ReadAll(path, text_, kMaxText);
  if (read >= kMaxText) {
    Unload();
    return false;
  }
  length_ = read;
  text_[length_] = 0;
  // The lines end with a single line feed.
  u32 out = 0;
  for (u32 i = 0; i < length_; i++) {
    if (text_[i] == '\r') continue;
    text_[out++] = text_[i];
  }
  length_ = out;
  if (length_ != 0 && text_[length_ - 1] != '\n') text_[length_++] = '\n';
  text_[length_] = 0;
  is_changed_ = false;
  return true;
}

void TextDocument::Unload() {
  std::free(text_);
  text_ = nullptr;
  length_ = 0;
  is_changed_ = false;
}

bool TextDocument::Save() {
  if (text_ == nullptr) return false;
  c16 path[96];
  c16 backup[96];
  if (!MakePath(file_name_, "", path, sizeof(path) / sizeof(path[0])) ||
      !MakePath(file_name_, ".bak", backup, sizeof(backup) / sizeof(backup[0]))) {
    return false;
  }
  // The copy of the file as it is on the SD card.
  {
    HeapBuffer old_text(kMaxText);
    if (old_text.IsValid()) {
      const u32 size = sys::File::ReadAll(path, old_text.Get(), kMaxText);
      if (size != 0) {
        sys::File copy(backup, true);
        if (copy.IsOpen()) copy.Write(old_text.Get(), size);
      }
    }
  }
  sys::File file(path, true);
  if (!file.IsOpen()) return false;
  file.Write(text_, length_);
  is_changed_ = false;
  return true;
}

bool TextDocument::GetLineRange(u32 line, u32* start, u32* length) const {
  if (text_ == nullptr) return false;
  u32 current = 0;
  u32 begin = 0;
  for (u32 i = 0; i < length_; i++) {
    if (text_[i] != '\n') continue;
    if (current == line) {
      *start = begin;
      *length = i - begin;
      return true;
    }
    current++;
    begin = i + 1;
  }
  return false;
}

u32 TextDocument::GetLineCount() const {
  if (text_ == nullptr) return 0;
  u32 count = 0;
  for (u32 i = 0; i < length_; i++) {
    if (text_[i] == '\n') count++;
  }
  return count;
}

bool TextDocument::GetLine(u32 line, c8* out, u32 capacity) const {
  u32 start = 0;
  u32 length = 0;
  if (capacity == 0 || !GetLineRange(line, &start, &length)) return false;
  if (length >= capacity) length = capacity - 1;
  std::memcpy(out, text_ + start, length);
  out[length] = 0;
  return true;
}

bool TextDocument::Replace(u32 start, u32 old_length, const c8* text,
                           u32 length) {
  if (text_ == nullptr || length_ - old_length + length > kMaxText) {
    return false;
  }
  std::memmove(text_ + start + length, text_ + start + old_length,
               length_ - start - old_length + 1);
  std::memcpy(text_ + start, text, length);
  length_ = length_ - old_length + length;
  is_changed_ = true;
  return true;
}

bool TextDocument::SetLine(u32 line, const c8* text) {
  u32 start = 0;
  u32 length = 0;
  if (!GetLineRange(line, &start, &length)) return false;
  return Replace(start, length, text,
                 static_cast<u32>(Length(text)));
}

bool TextDocument::InsertLine(u32 line, const c8* text) {
  if (text_ == nullptr) return false;
  u32 start = length_;
  if (line < GetLineCount()) {
    u32 length = 0;
    if (!GetLineRange(line, &start, &length)) return false;
  }
  c8 buffer[kMaxLine + 1];
  const u32 length = static_cast<u32>(Length(text));
  std::memcpy(buffer, text, length);
  buffer[length] = '\n';
  return Replace(start, 0, buffer, length + 1);
}

bool TextDocument::RemoveLine(u32 line) {
  u32 start = 0;
  u32 length = 0;
  if (!GetLineRange(line, &start, &length)) return false;
  return Replace(start, length + 1, "", 0);
}

bool TextDocument::IsHeaderLine(u32 line) const {
  c8 buffer[kMaxLine];
  if (!GetLine(line, buffer, sizeof(buffer))) return false;
  u32 start = 0;
  u32 end = 0;
  Trim(buffer, &start, &end);
  return end > start + 1 && buffer[start] == '[' && buffer[end - 1] == ']';
}

u32 TextDocument::GetSectionCount() const {
  u32 count = 0;
  const u32 lines = GetLineCount();
  for (u32 i = 0; i < lines; i++) {
    if (IsHeaderLine(i)) count++;
  }
  return count;
}

u32 TextDocument::GetSectionLine(u32 section) const {
  u32 count = 0;
  const u32 lines = GetLineCount();
  for (u32 i = 0; i < lines; i++) {
    if (!IsHeaderLine(i)) continue;
    if (count == section) return i;
    count++;
  }
  return kNone;
}

u32 TextDocument::GetSectionEnd(u32 section) const {
  const u32 first = GetSectionLine(section);
  if (first == kNone) return kNone;
  const u32 lines = GetLineCount();
  for (u32 i = first + 1; i < lines; i++) {
    if (IsHeaderLine(i)) return i;
  }
  return lines;
}

bool TextDocument::GetHeader(u32 section, c8* out, u32 capacity) const {
  const u32 line = GetSectionLine(section);
  c8 buffer[kMaxLine];
  if (line == kNone || capacity == 0 || !GetLine(line, buffer, sizeof(buffer))) {
    return false;
  }
  u32 start = 0;
  u32 end = 0;
  Trim(buffer, &start, &end);
  u32 length = end - start - 2;
  if (length >= capacity) length = capacity - 1;
  std::memcpy(out, buffer + start + 1, length);
  out[length] = 0;
  return true;
}

bool TextDocument::SetHeader(u32 section, const c8* header) {
  const u32 line = GetSectionLine(section);
  if (line == kNone) return false;
  c8 buffer[kMaxLine];
  std::snprintf(buffer, sizeof(buffer), "[%s]", header);
  return SetLine(line, buffer);
}

u32 TextDocument::AddSection(const c8* header) {
  if (text_ == nullptr) return kNone;
  const u32 lines = GetLineCount();
  // An empty line before the section.
  if (lines != 0) {
    c8 last[kMaxLine];
    if (GetLine(lines - 1, last, sizeof(last)) && last[0] != 0 &&
        !InsertLine(lines, "")) {
      return kNone;
    }
  }
  c8 buffer[kMaxLine];
  std::snprintf(buffer, sizeof(buffer), "[%s]", header);
  if (!InsertLine(GetLineCount(), buffer)) return kNone;
  return GetSectionCount() - 1;
}

bool TextDocument::RemoveSection(u32 section) {
  const u32 first = GetSectionLine(section);
  const u32 end = GetSectionEnd(section);
  if (first == kNone) return false;
  for (u32 i = first; i < end; i++) RemoveLine(first);
  return true;
}

bool TextDocument::IsKeyLine(const c8* line) {
  u32 start = 0;
  u32 end = 0;
  Trim(line, &start, &end);
  if (start >= end || line[start] == '#' || line[start] == '[') return false;
  u32 i = start;
  while (i < end && line[i] != '=' && line[i] != ',') i++;
  return i < end && line[i] == '=' && i > start;
}

bool TextDocument::IsBlankLine(const c8* line) {
  u32 start = 0;
  u32 end = 0;
  Trim(line, &start, &end);
  return start >= end || line[start] == '#';
}

bool TextDocument::GetBody(u32 section, u32* begin, u32* end) const {
  if (section == kPreamble) {
    *begin = 0;
    const u32 first = GetSectionLine(0);
    *end = first == kNone ? GetLineCount() : first;
    return true;
  }
  const u32 first = GetSectionLine(section);
  if (first == kNone) return false;
  *begin = first + 1;
  *end = GetSectionEnd(section);
  return true;
}

u32 TextDocument::FindKey(u32 section, const c8* key, u32 index) const {
  u32 begin = 0;
  u32 end = 0;
  if (!GetBody(section, &begin, &end)) return kNone;
  u32 found = 0;
  for (u32 i = begin; i < end; i++) {
    c8 buffer[kMaxLine];
    if (!GetLine(i, buffer, sizeof(buffer))) break;
    if (!SameKey(buffer, key)) continue;
    if (found++ == index) return i;
  }
  return kNone;
}

bool TextDocument::GetValue(u32 section, const c8* key, c8* out,
                            u32 capacity, u32 index) const {
  const u32 line = FindKey(section, key, index);
  c8 buffer[kMaxLine];
  if (line == kNone || capacity == 0 ||
      !GetLine(line, buffer, sizeof(buffer))) {
    return false;
  }
  const c8* equal = std::strchr(buffer, '=');
  if (equal == nullptr) return false;
  u32 start = static_cast<u32>(equal - buffer) + 1;
  while (buffer[start] != 0 && IsSpace(buffer[start])) start++;
  u32 end = static_cast<u32>(std::strlen(buffer));
  // The comment at the end of the line stays out of the value.
  for (u32 i = start; i < end; i++) {
    if (buffer[i] == '#') {
      end = i;
      break;
    }
  }
  while (end > start && IsSpace(buffer[end - 1])) end--;
  u32 length = end - start;
  if (length >= capacity) length = capacity - 1;
  std::memcpy(out, buffer + start, length);
  out[length] = 0;
  return true;
}

bool TextDocument::SetKey(u32 section, const c8* key, const c8* value,
                          u32 index) {
  const u32 line = FindKey(section, key, index);
  if (value == nullptr || value[0] == 0) {
    return line == kNone ? true : RemoveLine(line);
  }
  c8 buffer[kMaxLine];
  std::snprintf(buffer, sizeof(buffer), "%s = %s", key, value);
  if (line != kNone) return SetLine(line, buffer);
  // At the end of the lines of the section, before its empty lines.
  return InsertAtEnd(section, buffer);
}

bool TextDocument::InsertAtEnd(u32 section, const c8* text) {
  u32 begin = 0;
  u32 end = 0;
  if (!GetBody(section, &begin, &end)) return false;
  u32 at = end;
  while (at > begin) {
    c8 last[kMaxLine];
    if (!GetLine(at - 1, last, sizeof(last)) || !IsBlankLine(last) ||
        last[0] == '#') {
      break;
    }
    at--;
  }
  return InsertLine(at, text);
}

bool TextDocument::RemoveKey(u32 section, const c8* key, u32 index) {
  const u32 line = FindKey(section, key, index);
  return line != kNone && RemoveLine(line);
}

u32 TextDocument::GetRowLine(u32 section, u32 row) const {
  u32 begin = 0;
  u32 end = 0;
  if (!GetBody(section, &begin, &end)) return kNone;
  u32 found = 0;
  for (u32 i = begin; i < end; i++) {
    c8 buffer[kMaxLine];
    if (!GetLine(i, buffer, sizeof(buffer))) break;
    if (IsBlankLine(buffer) || IsKeyLine(buffer)) continue;
    if (found++ == row) return i;
  }
  return kNone;
}

u32 TextDocument::GetRowCount(u32 section) const {
  u32 count = 0;
  while (GetRowLine(section, count) != kNone) count++;
  return count;
}

u32 TextDocument::GetContentLine(u32 section, u32 index) const {
  u32 begin = 0;
  u32 end = 0;
  if (!GetBody(section, &begin, &end)) return kNone;
  u32 found = 0;
  for (u32 i = begin; i < end; i++) {
    c8 buffer[kMaxLine];
    if (!GetLine(i, buffer, sizeof(buffer))) break;
    if (IsBlankLine(buffer)) continue;
    if (found++ == index) return i;
  }
  return kNone;
}

u32 TextDocument::GetContentLineCount(u32 section) const {
  u32 count = 0;
  while (GetContentLine(section, count) != kNone) count++;
  return count;
}

bool TextDocument::AddRow(u32 section, const c8* text) {
  return InsertAtEnd(section, text);
}

bool TextDocument::GetPart(const c8* line, u32 index, c8* out, u32 capacity) {
  if (capacity == 0) return false;
  u32 current = 0;
  u32 start = 0;
  const u32 length = static_cast<u32>(std::strlen(line));
  for (u32 i = 0; i <= length; i++) {
    if (i != length && line[i] != ',') continue;
    if (current == index) {
      u32 a = start;
      u32 b = i;
      while (a < b && IsSpace(line[a])) a++;
      while (b > a && IsSpace(line[b - 1])) b--;
      u32 count = b - a;
      if (count >= capacity) count = capacity - 1;
      std::memcpy(out, line + a, count);
      out[count] = 0;
      return true;
    }
    current++;
    start = i + 1;
  }
  return false;
}

} // namespace core
