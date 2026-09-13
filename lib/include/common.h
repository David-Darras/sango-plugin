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

#ifdef __CLION_IDE__
#define USE_SANGO_PLUGIN
#endif

// Modular Core includes
#include "core/types.h"
#include "core/singleton.h"
#include "core/bitmask.h"
#include "core/math.h"
#include "core/color.h"
#include "core/memory.h"
#include "core/option.h"

#include <functional>
#include <CTRPluginFramework/System/Process.hpp>

// Address definitions
#include "address.h"

// Game constants
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender.h"
#include "pokemon/constant/species.h"
#include "system/constant/language.h"

namespace ui {
class MainApplication;
}

typedef void (*menu_callback_t)(ui::MainApplication& app, void* args);
typedef std::function<void(void*)> callback_t;
typedef std::function<void()> cheat_code_callback_t;

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

struct Message {
  uptr vtable;
  uptr heap;
  u32* archive;
  u8 kind; // 0 -> one, 1 -> all
  u32 file_id;
  Language language;
  u8 padding[3];
  uptr _0, _1, _2;

  INLINE void GetString(u32 str_id, String* output) {
    return ((void(*)(Message*, u32, String*))
      sys::address::kMessageGetString)(this, str_id, output);
  }
};

struct Bundle {
  u16 signature; // "PC"
  u16 resource_count;
  u32 resource_offset[];

  u32 GetResourceSize(u32 idx) const {
    u32 safe_idx = (idx >= resource_count) ? 0 : idx;
    return resource_offset[safe_idx + 1] - resource_offset[safe_idx];
  }

  uptr GetResource(u32 idx) {
    u32 safe_idx = (idx >= resource_count) ? 0 : idx;
    return ((uptr)this + resource_offset[safe_idx]);
  }
};

struct PokeInfo {
  SpeciesId species;
  Form form;
  Gender gender;
  bool is_shiny;
  bool is_egg;
  u32 _0;
};