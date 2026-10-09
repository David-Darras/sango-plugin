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
 * @file button.h
 * @brief A touch button of the bottom screen.
 */

#pragma once

#include "common.h"

namespace ui {
/// A touch button of the bottom screen.
class Button {
public:
  Button();

  /// Sets the position and the size of the button, in pixels.
  void Initialize(u32 x, u32 y, u32 width, u32 height);

  /**
   * @brief Draws the button and its label.
   * @param label The text of the button (UTF-16).
   * @param offset_x The horizontal space before the text.
   * @param offset_y The vertical space before the text.
   */
  void Draw(const c16* label, u32 offset_x, u32 offset_y) const;

  /// Returns true while the player touches the button.
  bool IsDown() const;

  /// Returns true at the frame when the player stops to touch the button.
  /// The call resets the state of the button.
  bool IsReleased() const;

  /// Reads the touch screen and updates the state. Call it one time for each
  /// frame.
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
