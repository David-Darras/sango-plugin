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

#include "ui/widget/button.h"

namespace ui {
/**
* @brief A visual Unicode keyboard for text input using a touch interface.
* Supports pagination, character selection, and basic editing (DEL/CLR).
*/
class Keyboard {
public:
  Keyboard();

  void Draw() const;

  void Update();

  bool IsButtonOkReleased() const;

  /**
* @brief Gets the current string entered by the user.
* @return A pointer to the UTF-16 character buffer.
*/
  const c16* GetInput() const;

private:
  /**
* @brief Appends a character to the input buffer if space is available.
* @param character The UTF-16 character to add.
*/
  void AddChar(c16 character);

  void RemoveLastChar();

  static constexpr u32 kColNum = 15;
  static constexpr u32 kRowNum = 4;
  static constexpr u32 kPageSize = kColNum * kRowNum;
  static constexpr u32 kBufferSize =
      17; ///< Max string length including null terminator.

  enum ButtonId : u8 {
    kButtonInput = 0, ///< The text display bar.
    kButtonPrev10, ///< Jump back 10 pages.
    kButtonPrev,
    kButtonNext,
    kButtonNext10, ///< Jump forward 10 pages.
    kButtonCancel, ///< Clear (CLR) action.
    kButtonDelete, ///< Backspace (DEL) action.
    kButtonOk, ///< Confirm (OK) action.
    kButtonGridStart, ///< Start index for the character keys.
    kButtonMax = kButtonGridStart + kPageSize
  };

  Button buttons_[kButtonMax];
  c16 input_[kBufferSize]; ///< UTF-16 input buffer.
  u16 page_index_ : 11; ///< Current Unicode page (supports up to 2048 pages).
  u16 cursor_ : 5; ///< Current character position (0 to 16).
};
} // namespace ui