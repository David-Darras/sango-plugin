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
#include "ui/widget/icon.h"

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

/**
 * @brief A function that gives the suggested values of an entry (see
 *        PageItem::WithSuggestions()).
 * @param ids Receives the values, the best first.
 * @param capacity The size of `ids`.
 * @return The number of values.
 */
typedef u32 (*suggest_t)(u16* ids, u32 capacity);
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

  /// The player can see the value, but cannot change it.
  PageItem& WithReadOnly();
  /**
   * @brief The menu shows the icons of the game for the value: a grid of
   *        icons on the bottom screen (in place of the numpad), and the
   *        icon of the value on the top screen.
   * @param kind The set of icons.
   * @param ids The icon of each value, or null: then the value is the icon
   *        (for example the item). For example, the species of each slot of
   *        a team. The array must stay in memory and contain one icon for
   *        each value.
   */
  PageItem& WithIcons(IconKind kind, const u16* ids = nullptr);
  /**
   * @brief Sets the function that gives the suggested values: for example
   *        the abilities of the species. The grid and the filter show these
   *        values first, with a mark. The player can still choose any value.
   *
   * The menu calls the function only when the player edits the entry: it
   * can read the game data.
   */
  PageItem& WithSuggestions(suggest_t suggest);

  u8 GetType() const;

  /// Returns the name of the entry.
  const c8* GetName() const { return name_; }
  /// Changes the name of the entry. The text must stay in memory.
  void SetName(const c8* name) { name_ = name; }
  /// Returns the address of the value of the entry.
  void* GetAddress() const { return address_; }
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
  /// Returns true when the player cannot change the value.
  bool IsReadOnly() const { return is_read_only_ != 0; }
  /// Returns the icons of WithIcons(), or IconKind::kNone.
  IconKind GetIconKind() const { return icon_kind_; }
  /// Returns true when WithIcons() received an icon for each value.
  bool HasIconIds() const { return icon_ids_ != nullptr; }
  /// Returns the function of WithSuggestions(), or null.
  suggest_t GetSuggest() const { return suggest_; }
  /// Returns the icon of a value (see WithIcons()).
  u32 GetIconId(u32 value) const {
    return icon_ids_ != nullptr ? icon_ids_[value] : value;
  }
  /// Returns true when the page must be built again after a change.
  bool NeedsRefresh() const { return refresh_ != 0; }
  /// Returns true for a value that can be less than 0.
  bool IsSigned() const;
  /// Returns true for a decimal value (f32, f64).
  bool IsDecimal() const;
  /// Returns the maximum number of characters of a text entry.
  u32 GetTextCapacity() const { return bit_offset_; }
  /// Returns the minimum value, or false when there is no minimum.
  bool GetMin(s32& min) const { min = min_; return is_min_used_ != 0; }
  /// Returns the maximum value, or false when there is no maximum.
  bool GetMax(s32& max) const { max = max_; return is_max_used_ != 0; }
  /// Returns the current value as an index (for an entry with texts).
  s32 GetIndex() const;
  /// Returns true when both entries show the same data.
  bool IsSameAs(const PageItem& other) const;

  /// Writes the name and the value of the entry into `buffer` (UTF-16).
  void GetDisplayValue(c16* buffer) const;

  /// Writes the name of the entry (UTF-16), with the A icon for an action.
  void GetNameText(c16* buffer) const;

  /// Writes only the value of the entry (UTF-16), or an empty text.
  void GetValueText(c16* buffer) const;

  /// Returns the size of the value in bytes (0 for a page or an action).
  u32 GetValueSize() const;

  /// Returns the value that the page function or the action receives.
  void* GetArgs() const { return args_; }

  /**
   * @brief Returns the value as a number: the bits of a kTypeBits entry, the
   *        bytes of the other values (8 at most). For a text, returns a hash
   *        of the text.
   */
  u64 ReadValue() const;
  /// Returns true when WriteValue() can write the value (not for a text).
  bool CanWriteValue() const;
  /// Writes a value of ReadValue() back. The limits do not apply.
  void WriteValue(u64 value);
  /// Increases the value (Right button).
  void Increment(u32 count = 1);

  /// Decreases the value (Left button).
  void Decrement(u32 count = 1);

  /// Sets the value that the player typed. `value` points to the new value.
  void Edit(const void* value);

  /**
   * @brief Sets a number that the player typed on the numpad.
   * @param text The number (UTF-16), for example "-12" or "0.25". A number
   *        that starts with "0x" is hexadecimal.
   */
  void EditNumber(const c16* text);

  /// Writes the current value as a number that the numpad can show.
  void GetNumberText(c16* buffer) const;

  /// Runs the entry (A button).
  void Execute(MainApplication& application);

private:
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
  u8 is_read_only_;
  IconKind icon_kind_; ///< See WithIcons().
  const u16* icon_ids_; ///< See WithIcons().
  suggest_t suggest_; ///< See WithSuggestions().
};
} // namespace ui