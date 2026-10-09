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
/**
 * @brief A keyboard on the bottom screen, for the texts of the game.
 *
 * The keyboard shows only the characters that the font of the game can draw
 * (the same method as ui::KeyboardPatch). < and > show the previous and the
 * next page of characters. << and >> jump to the previous and the next group
 * of characters (Latin, Hiragana, Katakana, the icons of the game...).
 */
class Keyboard {
public:
  Keyboard();

  /// Places the keyboard: (x, y) is the top-left corner of the text bar.
  void Initialize(s32 x, s32 y);

  /// Draws the keyboard on the bottom screen.
  void Draw() const;

  /// Reads the touch screen. Call it one time for each frame.
  void Update();

  /// Returns true at the frame when the player releases the OK button.
  bool IsButtonOkReleased() const;

  /**
   * @brief Starts a new text.
   * @param text The current text of the entry (UTF-16).
   * @param capacity The maximum number of characters, with the end
   *        character.
   */
  void SetInput(const c16* text, u32 capacity);

  /// Returns the typed text (UTF-16).
  const c16* GetInput() const;

private:
  static constexpr u32 kColNum = 10;
  static constexpr u32 kRowNum = 3;
  static constexpr u32 kPageSize = kColNum * kRowNum;
  /// The size of the text, in characters (with the end character).
  static constexpr u32 kBufferSize = 32;

  enum ButtonId : u8 {
    kButtonInput = 0, ///< The bar that shows the typed text.
    kButtonPrevGroup, ///< <<: the previous group of characters.
    kButtonPrev, ///< <: the previous page.
    kButtonNext, ///< >: the next page.
    kButtonNextGroup, ///< >>: the next group of characters.
    kButtonClear, ///< CLR: clears the text.
    kButtonDelete, ///< DEL: removes the last character.
    kButtonOk, ///< OK: confirms the text.
    kButtonGridStart, ///< The first character button.
    kButtonMax = kButtonGridStart + kPageSize
  };

  /// Adds a character at the end of the text, when there is space.
  void AddChar(c16 character);

  /// Removes the last character.
  void RemoveLastChar();

  /// Shows the page of characters that starts at `first` (or after it).
  void ShowPage(u32 first);

  /// Shows the page before the current page.
  void ShowPreviousPage();

  /// Jumps to the next (1) or previous (-1) group of characters.
  void JumpGroup(s32 direction);

  /// Returns the index of the group of a character.
  static u32 FindGroup(u32 character);

  /// Draws one key with its border.
  void DrawKey(u32 id, const c16* label, u32 offset_x) const;

  Button buttons_[kButtonMax];
  c16 input_[kBufferSize]; ///< The typed text (UTF-16).
  c16 chars_[kPageSize]; ///< The characters of the page (0: no character).
  u16 first_char_; ///< The first character of the page.
  u8 cursor_; ///< The number of characters of the text.
  u8 capacity_; ///< The maximum number of characters (with the end).
};
} // namespace ui
