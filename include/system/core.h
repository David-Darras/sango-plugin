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

class Device;
class Graphics;

/**
 * @brief The Core class acts as the central bridge to the engine's main
 * internal systems. It provides access to low-level hardware devices and
 * high-level graphics managers.
 */
class Core {
public:
  /**
 * @brief Retrieves the singleton instance of the Core bridge.
 * @return A reference to the static Core instance.
 */
  STATIC_INLINE Core& GetInstance() { return *(Core*)ADDRESS_CORE; }

  /**
 * @brief Accesses the hardware device manager (Input/HID).
 * @return A reference to the Device manager.
 */
  INLINE Device& GetDevice() const { return *device_; }

  /**
 * @brief Accesses the global graphics manager.
 * @return A reference to the Graphics manager.
 */
  INLINE Graphics& GetGraphics() const { return *graphics_; }

  INLINE u8& GetLanguageId() { return *language_id; }

private:
  /** @brief Private constructor to prevent manual instantiation. */
  Core() = default;

  Device* device_; ///< Pointer to the internal Device manager instance.
  Graphics* graphics_; ///< Pointer to the internal Graphics manager instance.

  void* _0[21];

  u8* language_id;
};