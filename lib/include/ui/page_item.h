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
 * @file page_item.h
 * @brief One entry of a menu page.
 */

#pragma once

#include "common.h"

namespace ui {
/// The type of an entry: it selects how the entry shows and changes its value.
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
/// One entry of a menu page. ui::MainApplication::Add() makes it.
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

  /// Sets the text that the bottom screen shows for the entry.
  PageItem& WithDescription(const c8* description);

  u8 GetType() const;

  /// Returns the name of the entry.
  const c8* GetName() const { return name_; }
  /// Changes the name of the entry. The text must stay in memory.
  void SetName(const c8* name) { name_ = name; }
  /// Returns the description of the entry, or null.
  const c8* GetDescription() const { return description_; }
  /// Returns true when A runs a function.
  bool HasCallback() const { return callback_ != nullptr; }
  /// Returns the texts of WithArray(), or null.
  const c8** GetArray() const { return array_; }
  /// Returns the number of texts of WithArray().
  u32 GetArraySize() const { return array_size_; }
  /// Returns false for a separator and a section title.
  bool IsSelectable() const { return type_ != kTypeSeparator; }
  /// Returns true when the entry shows a value that the player changes.
  bool HasValue() const;
  /// Returns the current value as an index (for an entry with texts).
  s32 GetIndex() const;
  /// Returns true when both entries show the same data.
  bool IsSameAs(const PageItem& other) const;

  /// Writes the name and the value of the entry into `buffer` (UTF-16).
  void GetDisplayValue(c16* buffer) const;

  /// Increases the value (Right button).
  void Increment(u32 count = 1);

  /// Decreases the value (Left button).
  void Decrement(u32 count = 1);

  /// Sets the value that the player typed. `value` points to the new value.
  void Edit(const void* value);

  /// Runs the entry (A button).
  void Execute(MainApplication& application);

private:
  void GetDefaultDisplayValue(c16* buffer) const;

  void GetArrayDisplayValue(c16* buffer) const;

  const c8* name_;
  void* address_;
  const c8** array_;
  callback_t callback_;
  void* args_;
  const c8* description_;

  u32 type_ : 6;
  u32 bit_offset_ : 6; ///< The first bit, or the size of a text.
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