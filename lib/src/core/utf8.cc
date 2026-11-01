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
 * @file utf8.cc
 * @brief Changes a UTF-8 text into a UTF-16 text.
 *
 * The declarations are in core/utf8.h.
 */

#include "core/utf8.h"

namespace core {

u32 Utf8ToUtf16(const c8* in, u32 length, c16* out, u32 capacity) {
  u32 count = 0;
  u32 i = 0;
  while (i < length && count + 1 < capacity) {
    const u8 first = static_cast<u8>(in[i]);
    u32 c;
    u32 extra;
    if (first < 0x80) {
      c = first;
      extra = 0;
    } else if ((first & 0xE0) == 0xC0) {
      c = first & 0x1F;
      extra = 1;
    } else if ((first & 0xF0) == 0xE0) {
      c = first & 0x0F;
      extra = 2;
    } else {
      i++;
      continue;
    }
    if (i + extra >= length && extra != 0) break;
    for (u32 k = 1; k <= extra; k++) {
      c = (c << 6) | (static_cast<u8>(in[i + k]) & 0x3F);
    }
    out[count++] = static_cast<c16>(c);
    i += 1 + extra;
  }
  out[count] = 0;
  return count;
}

} // namespace core
