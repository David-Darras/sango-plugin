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

#pragma once

#include "core/types.h"
#include "system/address.h"

namespace sys {

struct String {
  static String s_tmp;
  static c16 s_buffer[128];

  static String* GetTmpStr() { return &s_tmp; }
  static c16* GetTmpBuf() { return s_buffer; }

  String() {
    vtable = (void*)sys::address::kStringVtable;
    buffer = s_buffer;
    capacity = 128;
    size = 0;
    is_initialized = true;
  }

  INLINE c16* GetBuffer() const {
    return buffer;
  }

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
  u16 capacity;
  u16 size;
  bool is_initialized;
};

} // namespace sys

using sys::String;
