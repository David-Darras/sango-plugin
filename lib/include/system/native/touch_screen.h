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
 * @file touch_screen.h
 * @brief The touch screen of the console.
 */

#pragma once

#include "common.h"
#include "system/native/device.h"

namespace sys {

/// Reads the touch screen on the channel of the plugin.
class TouchScreen {
  SINGLETON(TouchScreen)

public:
  STATIC_INLINE TouchScreen& GetInstance() {
    return Device::GetInstance().GetTouchScreen();
  }

  /// Returns the X position of the touch, from 0 to 319.
  INLINE s32 GetX() {
    return ((s32 (*)(TouchScreen*, u8))address::kTouchscreenGetX)(
        this, Device::kCustomChannel);
  }

  /// Returns the Y position of the touch, from 0 to 239.
  INLINE s32 GetY() {
    return ((s32 (*)(TouchScreen*, u8))address::kTouchscreenGetY)(
        this, Device::kCustomChannel);
  }

  /// Returns true at the frame when the touch ends.
  INLINE bool IsReleased() {
    return ((bool (*)(TouchScreen*, u8))address::kTouchscreenIsReleased)(
        this, Device::kCustomChannel);
  }

  /// Returns true while the screen is touched.
  INLINE bool IsDown() {
    return ((bool (*)(TouchScreen*, u8))address::kTouchscreenIsDown)(
        this, Device::kCustomChannel);
  }
};

} // namespace sys
