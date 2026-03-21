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

#include "menu/button.h"

#include "system/device.h"
#include "system/graphics.h"

namespace menu {

Button::Button() : x_(0), width_(0), y_(0), state_(kIdle), height_(0) {}

void Button::Initialize(u32 x, u32 y, u32 width, u32 height) {
  x_ = x;
  y_ = y;
  width_ = width;
  height_ = height;
  state_ = kIdle;
}

void Button::Draw(const c16 *label, u32 offset_x, u32 offset_y) const {
  Color foreground_color{1, 1, 1, 1};
  if (IsDown()) foreground_color = {0, 1, 0, 1};

  Graphics::DrawText(x_ + offset_x, y_ + offset_y, label, foreground_color);
}

bool Button::IsDown() const {
  TouchScreen &touch_screen = TouchScreen::GetInstance();

  if (!touch_screen.IsDown()) return false;

  const s32 x = touch_screen.GetX();
  const s32 y = touch_screen.GetY();

  return (x >= x_ && x <= x_ + width_ && y >= y_ && y <= y_ + height_);
}

bool Button::IsReleased() const { return state_ == kReleased; }

void Button::Update() {
  TouchScreen &ts = TouchScreen::GetInstance();
  const s32 x = ts.GetX();
  const s32 y = ts.GetY();
  const bool is_down = ts.IsDown();

  switch (state_) {
    case kIdle:
      if (is_down && x >= x_ && x <= x_ + width_ && y >= y_ &&
          y <= y_ + height_) {
        state_ = kHold;
      }
      break;

    case kHold:
      if (!is_down) {
        state_ = kReleased;
      }
      break;

    case kReleased:
      state_ = kIdle;
      break;

    default:
      break;
  }
}

}  // namespace menu
