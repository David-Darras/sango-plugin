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
class Button {
public:
  Button();

  void Initialize(u32 x, u32 y, u32 width, u32 height);

  /**
* @brief Renders the button and its label on the screen.
* @param label The text to display (UTF-16).
* @param offset_x Horizontal padding for the text.
* @param offset_y Vertical padding for the text.
*/
  void Draw(const c16* label, u32 offset_x, u32 offset_y) const;

  bool IsDown() const;

  /**
* @brief Checks if the button was just released.
* @note Resets the internal state to IDLE after being called.
* @return True if the button transition from HOLD to RELEASED occurred.
*/
  bool IsReleased() const;

  /**
* @brief Updates the internal state machine based on touch input.
* Must be called once per frame.
*/
  void Update();

private:
  enum State : u8 {
    kIdle,
    kHold,
    kReleased,
  };

  u32 x_ : 9;
  u32 width_ : 9;
  u32 y_ : 8;
  u32 state_ : 2;
  u32  : 4;

  u32 height_ : 8;
  u32  : 24;
};
} // namespace ui