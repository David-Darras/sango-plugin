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
#include "system/constant/key.h"
#include "system/native/core.h"

namespace sys {
class Controller;
class TouchScreen;
class DPad;

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

} // namespace sys

