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
 * @file string.h
 * @brief A UTF-16 text object of the game.
 */

#pragma once

#include "core/types.h"
#include "system/address.h"

namespace sys {

/**
 * @brief A UTF-16 text object of the game.
 *
 * Many game functions write their result in a String.
 */
struct String {
  /// A shared temporary String. core::Utils::FormatString() uses it.
  static String s_tmp;
  /// The buffer of the shared temporary String.
  static c16 s_buffer[128];

  static String* GetTmpStr() { return &s_tmp; }
  static c16* GetTmpBuf() { return s_buffer; }

  /// Makes a String that uses the shared buffer.
  String() {
    vtable = (void*)sys::address::kStringVtable;
    buffer = s_buffer;
    capacity = 128;
    size = 0;
    is_initialized = true;
  }

  /// Returns the characters.
  INLINE c16* GetBuffer() const {
    return buffer;
  }

  /// Copies a text. The text is cut at `capacity - 1` characters.
  void Set(const c16* input) {
    if (capacity == 0) {
      is_initialized = false;
      return;
    }
    u32 index = 0;
    while (true) {
      if (index == capacity - 1 || input[index] == 0) {
        buffer[index] = 0;
        size = index;
        is_initialized = true;
        return;
      }
      buffer[index] = input[index];
      index++;
    }
  }

  void* vtable;
  c16* buffer;
  u16 capacity; ///< The size of the buffer, in characters.
  u16 size; ///< The length of the text, in characters.
  bool is_initialized;
};

} // namespace sys

using sys::String;
