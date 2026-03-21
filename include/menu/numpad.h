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

#ifndef SANGO_PLUGIN_MENU_NUMPAD_H
#define SANGO_PLUGIN_MENU_NUMPAD_H

#include "button.h"

namespace menu {

/**
 * @brief A visual numeric keypad for entering integer and hexadecimal values.
 */
class Numpad {
 public:
  /**
   * @brief Constructor that initializes button positions and dimensions.
   */
  Numpad();

  /**
   * @brief Renders the numpad and all its buttons to the screen.
   */
  void Draw() const;

  /**
   * @brief Updates the state of the buttons and handles digit input.
   * Must be called once per frame.
   */
  void Update();

  /**
   * @brief Checks if the 'OK' button was pressed and released.
   * @return True if the user confirmed the input.
   */
  bool IsButtonOkReleased() const;

  /**
   * @brief Converts the current buffer input into a numeric value.
   * Handles both decimal and hexadecimal (0x) formats.
   * @return The resulting unsigned 32-bit integer.
   */
  u32 GetInput() const;

 private:
  /**
   * @brief Internal button identifiers for array mapping.
   */
  enum ButtonId {
    kButton0 = 0,   ///< Number 0
    kButton1,       ///< Number 1
    kButton2,       ///< Number 2
    kButton3,       ///< Number 3
    kButton4,       ///< Number 4
    kButton5,       ///< Number 5
    kButton6,       ///< Number 6
    kButton7,       ///< Number 7
    kButton8,       ///< Number 8
    kButton9,       ///< Number 9
    kButtonInput,   ///< Input display bar
    kButtonCancel,  ///< CLR/Cancel action
    kButtonDelete,  ///< DEL/Backspace action
    kButtonOk,      ///< OK/Confirm action
    kButtonMax      ///< Sentinel for array size
  };

  /**
   * @brief Adds a digit to the current input buffer.
   * @param digit The digit (0-9) to add.
   */
  void AddDigit(u32 digit);

  /**
   * @brief Removes the last character from the input buffer.
   */
  void RemoveLastDigit();

  /**
   * @brief Helper to convert UTF-16 string to integer.
   * @param str The source string.
   * @return The converted integer.
   */
  static u32 UnicodeToInteger(const c16 *str);

  Button buttons_[kButtonMax];  ///< Array of interactive buttons.
  c16 input_[16];               ///< Internal UTF-16 buffer for digits.
  s8 cursor_;                   ///< Current position in the buffer.
};

}  // namespace menu

#endif  // SANGO_PLUGIN_MENU_NUMPAD_H