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
 * @file line_reader.cc
 * @brief Reads a text file of the SD card line by line.
 *
 * The declarations are in core/line_reader.h.
 */

#include "core/line_reader.h"

namespace core {

bool LineReader::Open(const c16* path) {
  file_.Open(path, sys::File::kRead);
  chunk_size_ = 0;
  chunk_position_ = 0;
  line_number_ = 0;
  is_end_ = !file_.IsOpen();
  return file_.IsOpen();
}

bool LineReader::NextByte(c8* c) {
  if (chunk_position_ >= chunk_size_) {
    if (is_end_) return false;
    chunk_size_ = file_.Read(chunk_, sizeof(chunk_));
    chunk_position_ = 0;
    // A short read is the end of the file.
    if (chunk_size_ < static_cast<s32>(sizeof(chunk_))) is_end_ = true;
    if (chunk_size_ <= 0) return false;
  }
  *c = chunk_[chunk_position_++];
  return true;
}

bool LineReader::Next(c8* line, u32 capacity) {
  if (capacity == 0) return false;
  for (;;) {
    u32 length = 0;
    bool has_byte = false;
    c8 c = 0;
    while (NextByte(&c)) {
      has_byte = true;
      if (c == '\n') break;
      if (c != '\r' && length < capacity - 1) line[length++] = c;
    }
    if (!has_byte) return false;
    line[length] = '\0';
    line_number_++;
    if (length != 0 && line[0] != '#') return true;
  }
}

bool ParseInt(const c8*& p, s32& value, bool allow_negative) {
  while (*p == ' ') p++;
  bool is_negative = false;
  if (allow_negative && *p == '-') {
    is_negative = true;
    p++;
  }
  if (*p < '0' || *p > '9') return false;
  s32 result = 0;
  while (*p >= '0' && *p <= '9') result = result * 10 + (*p++ - '0');
  value = is_negative ? -result : result;
  return true;
}

} // namespace core
