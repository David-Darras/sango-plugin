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
 * @file keyboard.h
 * @brief The keyboard of the menu on the bottom screen.
 */

#pragma once

#include "ui/widget/button.h"

namespace ui {
/// A Unicode keyboard on the bottom screen. The player selects a page of
/// characters, then touches the characters. DEL and CLR edit the text.
class Keyboard {
public:
  Keyboard();

  /// Draws the keyboard on the bottom screen.
  void Draw() const;

  /// Reads the touch screen. Call it one time for each frame.
  void Update();

  /// Returns true at the frame when the player releases the OK button.
  bool IsButtonOkReleased() const;

  /// Returns the typed text (UTF-16).
  const c16* GetInput() const;

private:
  /// Adds a character at the end of the text, when there is space.
  void AddChar(c16 character);

  /// Removes the last character.
  void RemoveLastChar();

  static constexpr u32 kColNum = 15;
  static constexpr u32 kRowNum = 4;
  static constexpr u32 kPageSize = kColNum * kRowNum;
  /// The size of the text, in characters (with the end character).
  static constexpr u32 kBufferSize = 17;

  enum ButtonId : u8 {
    kButtonInput = 0, ///< The bar that shows the typed text.
    kButtonPrev10, ///< Goes back 10 pages.
    kButtonPrev,
    kButtonNext,
    kButtonNext10, ///< Goes forward 10 pages.
    kButtonCancel, ///< CLR: clears the text.
    kButtonDelete, ///< DEL: removes the last character.
    kButtonOk, ///< OK: confirms the text.
    kButtonGridStart, ///< The first character button.
    kButtonMax = kButtonGridStart + kPageSize
  };

  Button buttons_[kButtonMax];
  c16 input_[kBufferSize]; ///< The typed text (UTF-16).
  u16 page_index_ : 11; ///< The current page of characters (2048 at most).
  u16 cursor_ : 5; ///< The position in the text (0 to 16).
};
} // namespace ui
