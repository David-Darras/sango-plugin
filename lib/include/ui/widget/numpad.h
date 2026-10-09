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
 * @file numpad.h
 * @brief The numpad of the menu on the bottom screen.
 */

#pragma once

#include "ui/widget/button.h"

namespace ui {
/**
 * @brief A numpad on the bottom screen, for integers and decimal numbers.
 *
 * The keys are in the layout of a phone (1 2 3 at the top), with a minus key
 * and a decimal point. The numpad starts with the current value. The first
 * key replaces this value; DEL edits it.
 */
class Numpad {
public:
  /// The width of the keys, in pixels. The menu places its buttons next to
  /// the keys.
  static constexpr s32 kKeysWidth = 160;
  /// The height of one row of keys, with the space under it.
  static constexpr s32 kRowHeight = 27;

  Numpad();

  /// Places the numpad: (x, y) is the top-left corner of the text bar.
  void Initialize(s32 x, s32 y);

  /// Draws the numpad on the bottom screen.
  void Draw() const;

  /// Reads the touch screen. Call it one time for each frame.
  void Update();

  /// Returns true at the frame when the player releases the OK button.
  bool IsButtonOkReleased() const;

  /**
   * @brief Starts a new number.
   * @param text The current value (UTF-16). The first key replaces it.
   * @param allow_minus true when the value can be less than 0.
   * @param allow_dot true for a decimal value.
   */
  void SetInput(const c16* text, bool allow_minus, bool allow_dot);

  /// Returns the typed number (UTF-16). A number that starts with "0x" is
  /// hexadecimal.
  const c16* GetInput() const { return input_; }

private:
  enum ButtonId : u8 {
    kButton0 = 0, ///< The digit 0. The digits 1 to 9 follow.
    kButtonMinus = 10, ///< "-": changes the sign of the number.
    kButtonDot, ///< ".": the decimal point.
    kButtonInput, ///< The bar that shows the number.
    kButtonDelete, ///< DEL: removes the last character.
    kButtonClear, ///< CLR: clears the number.
    kButtonOk, ///< OK: confirms the number.
    kButtonMax
  };

  static constexpr u32 kMaxLength = 15;

  /// Adds a character at the end of the number.
  void AddChar(c16 character);

  /// Removes the last character.
  void RemoveLastChar();

  /// Draws one key with its border.
  void DrawKey(ButtonId id, const c16* label, bool is_enabled) const;

  Button buttons_[kButtonMax];
  c16 input_[kMaxLength + 1]; ///< The typed number (UTF-16).
  u8 cursor_; ///< The number of characters.
  bool is_edited_; ///< false: the bar shows the value of the entry.
  bool allow_minus_;
  bool allow_dot_;
};
} // namespace ui
