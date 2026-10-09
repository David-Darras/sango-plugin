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
/// A numpad on the bottom screen. The player types decimal or hexadecimal
/// numbers.
class Numpad {
public:
  Numpad();

  /// Places the numpad: (x, y) is its top-left corner, in pixels.
  void Initialize(s32 x, s32 y);

  /// Draws the numpad on the bottom screen.
  void Draw() const;

  /// Reads the touch screen and the digits. Call it one time for each frame.
  void Update();

  /// Returns true at the frame when the player releases the OK button.
  bool IsButtonOkReleased() const;

  /// Returns the typed number. A number that starts with "0x" is
  /// hexadecimal.
  u32 GetInput() const;

private:
  enum ButtonId : u8 {
    kButton0 = 0, ///< The digit 0.
    kButton1, ///< The digit 1.
    kButton2, ///< The digit 2.
    kButton3, ///< The digit 3.
    kButton4, ///< The digit 4.
    kButton5, ///< The digit 5.
    kButton6, ///< The digit 6.
    kButton7, ///< The digit 7.
    kButton8, ///< The digit 8.
    kButton9, ///< The digit 9.
    kButtonInput, ///< The bar that shows the typed number.
    kButtonCancel, ///< CLR: clears the number.
    kButtonDelete, ///< DEL: removes the last digit.
    kButtonOk, ///< OK: confirms the number.
    kButtonMax
  };

  /// Adds a digit (0 to 9) at the end of the number.
  void AddDigit(u32 digit);

  /// Removes the last digit.
  void RemoveLastDigit();

  /// Converts a UTF-16 text into a number.
  static u32 UnicodeToInteger(const c16* str);

  Button buttons_[kButtonMax];
  c16 input_[16]; ///< The typed digits (UTF-16).
  s8 cursor_;
};
} // namespace ui
