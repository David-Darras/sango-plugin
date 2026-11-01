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
 * @file line_reader.h
 * @brief Reads a text file of the SD card line by line, and reads the numbers
 * of a line.
 *
 * @code
 * core::LineReader reader;
 * if (!reader.Open(SANGO_DATA_PATH(u"zones.txt"))) return;
 * c8 line[96];
 * while (reader.Next(line, sizeof(line))) {
 *   const c8* p = line;
 *   s32 map = 0;
 *   if (!core::ParseInt(p, map)) continue;
 * }
 * @endcode
 */

#pragma once

#include "common.h"
#include "system/native/file.h"

namespace core {

/**
 * @brief Reads a text file line by line.
 *
 * Next() removes the "\r" of the line ends, and skips the empty lines and the
 * lines that start with '#'. A line longer than the buffer is cut.
 */
class LineReader {
public:
  LineReader() : chunk_size_(0), chunk_position_(0), line_number_(0),
                 is_end_(true) {}
  LineReader(const LineReader&) = delete;
  LineReader& operator=(const LineReader&) = delete;

  /// Opens a file. Returns false when the file does not exist.
  bool Open(const c16* path);

  /**
   * @brief Reads the next line that is not empty and not a comment.
   * @param line Receives the line, with a 0 at the end.
   * @param capacity The size of `line`, in bytes.
   * @return false at the end of the file.
   */
  bool Next(c8* line, u32 capacity);

  /// Returns the number of the last line that Next() gave (1 for the first
  /// line of the file).
  u32 GetLineNumber() const { return line_number_; }

private:
  /// Gives the next byte of the file. Returns false at the end.
  bool NextByte(c8* c);

  sys::File file_;
  c8 chunk_[128];
  s32 chunk_size_;
  s32 chunk_position_;
  u32 line_number_;
  bool is_end_;
};

/**
 * @brief Reads a number after the spaces, and moves `p` after it.
 * @param p The text. It moves after the number.
 * @param value Receives the number.
 * @param allow_negative true: a '-' before the number is accepted.
 * @return false when the text has no number here.
 */
bool ParseInt(const c8*& p, s32& value, bool allow_negative = true);

} // namespace core
