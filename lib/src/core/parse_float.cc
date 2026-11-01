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
 * @file parse_float.cc
 * @brief Reads a decimal number of a text file, like strtof().
 *
 * The declarations are in core/parse_float.h.
 */

#include "core/parse_float.h"

namespace core {

f32 ParseFloat(const c8* text, c8** end) {
  const c8* p = text;
  while (*p == ' ' || *p == '\t') p++;
  bool is_negative = false;
  if (*p == '-' || *p == '+') is_negative = *p++ == '-';
  // The digits go in an integer, then one division at the end: the result is
  // as exact as possible for the numbers of the files.
  f64 value = 0;
  f64 scale = 1;
  bool has_digit = false;
  for (; *p >= '0' && *p <= '9'; p++) {
    value = value * 10 + (*p - '0');
    has_digit = true;
  }
  if (*p == '.') {
    p++;
    for (; *p >= '0' && *p <= '9'; p++) {
      value = value * 10 + (*p - '0');
      scale *= 10;
      has_digit = true;
    }
  }
  if (!has_digit) {
    if (end != nullptr) *end = const_cast<c8*>(text);
    return 0;
  }
  if (*p == 'e' || *p == 'E') {
    const c8* exponent_start = p + 1;
    bool is_exponent_negative = false;
    if (*exponent_start == '-' || *exponent_start == '+') {
      is_exponent_negative = *exponent_start++ == '-';
    }
    if (*exponent_start >= '0' && *exponent_start <= '9') {
      u32 exponent = 0;
      for (p = exponent_start; *p >= '0' && *p <= '9'; p++) {
        if (exponent < 100) exponent = exponent * 10 + (*p - '0');
      }
      for (u32 i = 0; i < exponent; i++) {
        if (is_exponent_negative) {
          scale *= 10;
        } else {
          value *= 10;
        }
      }
    }
  }
  if (end != nullptr) *end = const_cast<c8*>(p);
  const f64 result = value / scale;
  return static_cast<f32>(is_negative ? -result : result);
}

} // namespace core
