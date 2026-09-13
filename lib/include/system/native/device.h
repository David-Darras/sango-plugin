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
#include "system/native/core.h"

namespace sys {
class Controller;
class TouchScreen;
class DPad;

enum class Key : u32 {
  kNone = 0,
  kLeft = 1 << 0,
  kRight = 1 << 1,
  kUp = 1 << 2,
  kDown = 1 << 3,
  kA = 1 << 4,
  kB = 1 << 5,
  kX = 1 << 6,
  kY = 1 << 7,
  kL = 1 << 8,
  kR = 1 << 9,
  kZl = 1 << 10,
  kZr = 1 << 11,
  kStart = 1 << 12,
  kSelect = 1 << 13,
  kHome = 1 << 14,
};
ENABLE_BITMASK_OPERATORS(Key)

class Device {
  SINGLETON(Device)

public:
  static constexpr u8 kCustomChannel = 0x13;

  STATIC_INLINE Device& GetInstance() {
    return Core::GetInstance().GetDevice();
  }

  INLINE Controller& GetController() {
    return ((Controller & (*)(Device*, u32))address::kDeviceGetController)(
        this, 0);
  }

  INLINE DPad& GetDPad() {
    return ((DPad & (*)(Device*, u32))address::kDeviceGetDpad)(this, 0);
  }

  INLINE TouchScreen& GetTouchScreen() {
    return ((TouchScreen & (*)(Device*, u32))address::kDeviceGetTouchscreen)(
        this, 0);
  }
};

class Controller {
  SINGLETON(Controller)

public:
  STATIC_INLINE Controller& GetInstance() {
    return Device::GetInstance().GetController();
  }

  INLINE bool IsKeyPressed(Key key) {
    return ((bool (*)(Controller*, Key, u8))address::kControllerIsKeyPressed)(
        this, key, Device::kCustomChannel);
  }

  INLINE bool IsKeyReleased(Key key) {
    return ((bool (*)(Controller*, Key, u8))address::kControllerIsKeyReleased)(
        this, key, Device::kCustomChannel);
  }

  INLINE bool IsKeyRepeated(Key key) {
    return ((bool (*)(Controller*, Key, u8))address::kControllerIsKeyRepeated)(
        this, key, Device::kCustomChannel);
  }

  INLINE bool IsKeyDown(Key key) {
    return ((bool (*)(Controller*, Key, u8))address::kControllerIsKeyDown)(
        this, key, Device::kCustomChannel);
  }

  INLINE Key GetRepeatedKey() {
    return ((Key (*)(Controller*, u8))address::kControllerGetRepeatedKey)(
        this, Device::kCustomChannel);
  }
};

class TouchScreen {
  SINGLETON(TouchScreen)

public:
  STATIC_INLINE TouchScreen& GetInstance() {
    return Device::GetInstance().GetTouchScreen();
  }

  INLINE s32 GetX() {
    return ((s32 (*)(TouchScreen*, u8))address::kTouchscreenGetX)(
        this, Device::kCustomChannel);
  }

  INLINE s32 GetY() {
    return ((s32 (*)(TouchScreen*, u8))address::kTouchscreenGetY)(
        this, Device::kCustomChannel);
  }

  INLINE bool IsReleased() {
    return ((bool (*)(TouchScreen*, u8))address::kTouchscreenIsReleased)(
        this, Device::kCustomChannel);
  }

  INLINE bool IsDown() {
    return ((bool (*)(TouchScreen*, u8))address::kTouchscreenIsDown)(
        this, Device::kCustomChannel);
  }
};

class DPad {
  SINGLETON(DPad)
public:
  STATIC_INLINE DPad& GetInstance() { return Device::GetInstance().GetDPad(); }
};

} // namespace sys

using sys::Key;
