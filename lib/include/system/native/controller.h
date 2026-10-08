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
 * @file controller.h
 * @brief The buttons of the console, as the game reads them.
 */

#pragma once

#include "common.h"
#include "system/constant/key.h"
#include "system/native/device.h"

namespace sys {

/**
 * @brief Reads the buttons.
 *
 * The plugin reads the buttons on its own channel (Device::kCustomChannel):
 * it sees the buttons also when core::DevicePatch hides them from the game.
 */
class Controller {
  SINGLETON(Controller)

public:
  STATIC_INLINE Controller& GetInstance() {
    return Device::GetInstance().GetController();
  }

  /// Returns true at the frame when the button goes down.
  INLINE bool IsKeyPressed(Key key) {
    return ((bool (*)(Controller*, Key, u8))address::kControllerIsKeyPressed)(
        this, key, Device::kCustomChannel);
  }

  /// Returns true at the frame when the button goes up.
  INLINE bool IsKeyReleased(Key key) {
    return ((bool (*)(Controller*, Key, u8))address::kControllerIsKeyReleased)(
        this, key, Device::kCustomChannel);
  }

  /// Returns true when the button goes down, then again at a regular interval while it stays down.
  INLINE bool IsKeyRepeated(Key key) {
    return ((bool (*)(Controller*, Key, u8))address::kControllerIsKeyRepeated)(
        this, key, Device::kCustomChannel);
  }

  /// Returns true while the button is down.
  INLINE bool IsKeyDown(Key key) {
    return ((bool (*)(Controller*, Key, u8))address::kControllerIsKeyDown)(
        this, key, Device::kCustomChannel);
  }

  /// Returns the buttons that IsKeyRepeated() would accept.
  INLINE Key GetRepeatedKey() {
    return ((Key (*)(Controller*, u8))address::kControllerGetRepeatedKey)(
        this, Device::kCustomChannel);
  }
};

} // namespace sys
