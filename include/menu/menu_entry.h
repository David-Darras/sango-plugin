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

#ifndef SANGO_PLUGIN_MENU_MENU_ENTRY_H
#define SANGO_PLUGIN_MENU_MENU_ENTRY_H

#include "common.h"

namespace menu {

/**
 * @brief Enumeration of supported data types for a menu entry.
 */
enum MenuEntryType {
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
  kTypeMax,
};

/**
 * @brief Represents a single item in the menu, linked to a memory address.
 * Handles display formatting, increment/decrement logic, and execution.
 */
class MenuEntry {
 public:
  /**
   * @brief Default constructor initializing members to default/null states.
   */
  MenuEntry();

  /**
   * @brief Initializes the entry with core properties.
   * @param name The display name of the entry.
   * @param addr Pointer to the data in memory.
   * @param type The data type (from MenuEntryType).
   * @param bit_offset Offset in bits (used for kTypeBits or string length).
   * @param bit_size Size in bits (used for kTypeBits).
   */
  void Initialize(const c8 *name, void *addr, u8 type, u32 bit_offset = 0,
                  u32 bit_size = 0);

  /**
   * @brief Attaches a list of strings to map numerical values to text labels.
   * @param array An array of C-strings.
   * @param array_size Number of elements in the array.
   * @return Reference to this entry for method chaining.
   */
  MenuEntry &WithArray(const c8 *array[], u32 array_size);

  /**
   * @brief Attaches a custom callback function to be triggered on execution.
   * @param callback Function pointer of type callback_t.
   * @return Reference to this entry for method chaining.
   */
  MenuEntry &WithCallback(callback_t callback);

  /**
   * @brief Marks the entry to request a menu refresh after modification.
   * * Sets the internal `refresh_` flag to indicate that the current menu
   * should be rebuilt following an interaction with this specific item.
   * @return Reference to the current entry for method chaining.
   */
  MenuEntry &WithRefresh();

  /**
   * @brief Sets the minimum allowed value for this entry.
   * @param min The lower bound value.
   * @return Reference to this entry for method chaining.
   */
  MenuEntry &WithMin(s32 min);

  /**
   * @brief Sets the maximum allowed value for this entry.
   * @param max The upper bound value.
   * @return Reference to this entry for method chaining.
   */
  MenuEntry &WithMax(s32 max);

  /**
   * @brief Gets the current entry type.
   * @return The MenuEntryType value.
   */
  u8 GetType() const;

  /**
   * @brief Formats the entry name and its value into a displayable string.
   * @param buffer The output buffer (UTF-16).
   */
  void GetDisplayValue(c16 *buffer) const;

  /**
   * @brief Increments the underlying value.
   * @param count The amount to add.
   */
  void Increment(u32 count = 1);

  /**
   * @brief Decrements the underlying value.
   * @param count The amount to subtract.
   */
  void Decrement(u32 count = 1);

  /**
   * @brief Directly sets the value or enters a sub-menu depending on type.
   * @param value Pointer to the new value or context data.
   */
  void Edit(const void *value);

  /**
   * @brief Triggers the attached callback if one exists.
   * @param args Pointer to arguments to pass to the callback.
   */
  void Execute(void *args);

 private:
  /**
   * @brief Standard formatting for basic types (int, float, etc.).
   */
  void GetDefaultDisplayValue(c16 *buffer) const;

  /**
   * @brief Formatting for entries using a string array lookup.
   */
  void GetArrayDisplayValue(c16 *buffer) const;

  const c8 *name_;       ///< Display name of the entry.
  void *address_;        ///< Memory address of the targeted variable.
  const c8 **array_;     ///< Optional array for string mapping.
  callback_t callback_;  ///< Optional execution callback.

  // Bitfields for memory efficiency
  u32 type_ : 6;         ///< Storage for MenuEntryType.
  u32 bit_offset_ : 6;   ///< Bit position or string capacity.
  u32 bit_size_ : 6;     ///< Number of bits to read/write.
  u32 array_size_ : 13;  ///< Size of the mapping array.
  u32 refresh_ : 1;      ///< Flag to trigger a menu refresh on change.

  s32 min_ : 15;
  s32 max_ : 15;
  s32 is_min_used_ : 1;
  s32 is_max_used_ : 1;
};

}  // namespace menu

#endif  // SANGO_PLUGIN_MENU_MENU_ENTRY_H