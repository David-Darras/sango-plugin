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
 * @file stick.h
 * @brief The Circle Pad of the console.
 */

#pragma once

#include "common.h"
#include "system/native/device.h"
#include "system/address.h"

namespace sys {

/// The Circle Pad.
class Stick {
public:
  /**
   * @brief Reads the position of the Circle Pad.
   * @param x Gets the position to the right, from -1 to 1.
   * @param y Gets the position to the top, from -1 to 1.
   * @return false when the device does not exist (the position is then 0).
   *         ORAS only: the function gives 0 in the other game.
   */
  static bool Read(f32* x, f32* y) {
    *x = 0.0f;
    *y = 0.0f;
#ifdef GAME_ORAS
    void* stick = ((void* (*)(Device*, u32))address::kCirclePadDevice)(
        &Device::GetInstance(), 0);
    if (stick == nullptr) return false;
    *x = ((f32 (*)(void*))address::kCirclePadReadX)(stick);
    *y = ((f32 (*)(void*))address::kCirclePadReadY)(stick);
    return true;
#else
    return false;
#endif
  }
};

} // namespace sys
