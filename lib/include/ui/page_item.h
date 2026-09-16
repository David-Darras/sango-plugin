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

#include "common.h"

namespace ui {
enum PageItemType : u8 {
  kTypeU8,
  kTypeU16,
  kTypeU32,
  kTypeU64,
  kTypeS8,
  kTypeS16,
  kTypeS32,
  kTypeS64,
  kTypeF32,
  kTypeF64,
  kTypeBoolean,
  kTypeBits,
  kTypePointer,
  kTypeMenu,
  kTypeUnicode,
  kTypeCheatCode,
  kTypeIdle,
  kTypeSpecies,
  kTypeAbility,
  kTypeMove,
  kTypeItem,
  kTypeSeparator,
  kTypeMax,
};

class MainApplication;
class PageItem {
public:
  PageItem();

  void Initialize(const c8* name, void* addr, u8 type, u32 bit_offset = 0,
                  u32 bit_size = 0);

  PageItem& WithArray(const c8* array[], u32 array_size);

  PageItem& WithCallback(callback_t callback);

  PageItem& WithRefresh();

  PageItem& WithMin(s32 min);

  PageItem& WithMax(s32 max);

  PageItem& WithArgs(void* args);

  PageItem& WithFactor(f32 factor);

  u8 GetType() const;

  /**
* @brief Formats the entry name and its value into a displayable string.
* @param buffer The output buffer (UTF-16).
*/
  void GetDisplayValue(c16* buffer) const;

  void Increment(u32 count = 1);

  void Decrement(u32 count = 1);

  /**
* @brief Directly sets the value or enters a sub-menu depending on type.
* @param value Pointer to the new value or context data.
*/
  void Edit(const void* value);

  void Execute(MainApplication& application);

private:
  void GetDefaultDisplayValue(c16* buffer) const;

  void GetArrayDisplayValue(c16* buffer) const;

  const c8* name_;
  void* address_;
  const c8** array_;
  callback_t callback_;
  void* args_;

  u32 type_ : 6;
  u32 bit_offset_ : 6; ///< Bit position or string capacity.
  u32 bit_size_ : 6;
  u32 array_size_ : 13;
  u32 refresh_ : 1;

  s32 min_ : 15;
  s32 max_ : 15;
  s32 is_min_used_ : 1;
  s32 is_max_used_ : 1;

  f32 factor_;
};
} // namespace ui