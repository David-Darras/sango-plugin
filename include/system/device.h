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

#ifndef SANGO_PLUGIN_DEVICE_H
#define SANGO_PLUGIN_DEVICE_H

#include "core/core.h"

class Controller;
class TouchScreen;
class DPad;

/**
 * @brief Bitmask values for physical hardware buttons.
 */
enum class Key {
  kLeft = 1 << 0,     ///< D-Pad Left
  kRight = 1 << 1,    ///< D-Pad Right
  kUp = 1 << 2,       ///< D-Pad Up
  kDown = 1 << 3,     ///< D-Pad Down
  kA = 1 << 4,        ///< A Button
  kB = 1 << 5,        ///< B Button
  kX = 1 << 6,        ///< X Button
  kY = 1 << 7,        ///< Y Button
  kL = 1 << 8,        ///< L Shoulder Button
  kR = 1 << 9,        ///< R Shoulder Button
  kZl = 1 << 10,      ///< ZL Shoulder Button
  kZr = 1 << 11,      ///< ZR Shoulder Button
  kStart = 1 << 12,   ///< Start Button
  kSelect = 1 << 13,  ///< Select Button
  kHome = 1 << 14,    ///< Home Button
};

/**
 * @brief Main hardware device manager.
 * Acts as a hub to access input subsystems like the controller, touchscreen,
 * and D-Pad.
 */
class Device {
  /** @brief Private constructor for singleton pattern. */
  Device() = default;

 public:
  /** @brief Custom input channel identifier used for filtered input polling. */
  static constexpr u8 kCustomChannel = 0x13;

  /**
   * @brief Retrieves the singleton instance of the Device manager.
   * @return Reference to the Device instance.
   */
  static Device& GetInstance() { return Core::GetInstance().GetDevice(); }

  /**
   * @brief Accesses the primary controller subsystem.
   * @return Reference to the Controller instance.
   */
  Controller& GetController() {
    return ((Controller & (*)(Device*, u32)) ADDRESS_DEVICE_GET_CONTROLLER)(
        this, 0);
  }

  /**
   * @brief Accesses the D-Pad subsystem.
   * @return Reference to the DPad instance.
   */
  DPad& GetDPad() {
    return ((DPad & (*)(Device*, u32)) ADDRESS_DEVICE_GET_DPAD)(this, 0);
  }

  /**
   * @brief Accesses the TouchScreen subsystem.
   * @return Reference to the TouchScreen instance.
   */
  TouchScreen& GetTouchScreen() {
    return ((TouchScreen & (*)(Device*, u32)) ADDRESS_DEVICE_GET_TOUCHSCREEN)(
        this, 0);
  }

  /**
   * @brief Initializes system hooks required for input interception.
   */
  static void SetupHooks();
};

/**
 * @brief Handles digital button inputs and state detection.
 */
class Controller {
  Controller() = default;

 public:
  /** @return Singleton reference to the controller. */
  static Controller& GetInstance() {
    return Device::GetInstance().GetController();
  }

  /** @brief Checks if a key was just pressed this frame. */
  bool IsKeyPressed(Key key) {
    return ((bool (*)(Controller*, Key, u8))ADDRESS_CONTROLLER_IS_KEY_PRESSED)(
        this, key, Device::kCustomChannel);
  }

  /** @brief Checks if a key was just released this frame. */
  bool IsKeyReleased(Key key) {
    return ((bool (*)(Controller*, Key, u8))ADDRESS_CONTROLLER_IS_KEY_RELEASED)(
        this, key, Device::kCustomChannel);
  }

  /** @brief Checks if a key is being held down with repeat logic. */
  bool IsKeyRepeated(Key key) {
    return ((bool (*)(Controller*, Key, u8))ADDRESS_CONTROLLER_IS_KEY_REPEATED)(
        this, key, Device::kCustomChannel);
  }

  /** @brief Checks if a key is currently held down. */
  bool IsKeyDown(Key key) {
    return ((bool (*)(Controller*, Key, u8))ADDRESS_CONTROLLER_IS_KEY_DOWN)(
        this, key, Device::kCustomChannel);
  }
};

/**
 * @brief Handles touch input coordinates and states.
 */
class TouchScreen {
  TouchScreen() = default;

 public:
  /** @return Singleton reference to the touchscreen. */
  static TouchScreen& GetInstance() {
    return Device::GetInstance().GetTouchScreen();
  }

  /** @return The current X coordinate of the touch point. */
  s32 GetX() {
    return ((s32 (*)(TouchScreen*, u8))ADDRESS_TOUCHSCREEN_GET_X)(
        this, Device::kCustomChannel);
  }

  /** @return The current Y coordinate of the touch point. */
  s32 GetY() {
    return ((s32 (*)(TouchScreen*, u8))ADDRESS_TOUCHSCREEN_GET_Y)(
        this, Device::kCustomChannel);
  }

  /** @brief Checks if the screen was just released. */
  bool IsReleased() {
    return ((bool (*)(TouchScreen*, u8))ADDRESS_TOUCHSCREEN_IS_RELEASED)(
        this, Device::kCustomChannel);
  }

  /** @brief Checks if the screen is currently being touched. */
  bool IsDown() {
    return ((bool (*)(TouchScreen*, u8))ADDRESS_TOUCHSCREEN_IS_DOWN)(
        this, Device::kCustomChannel);
  }
};

/**
 * @brief Specialized handler for D-Pad specific interactions.
 */
class DPad {
  DPad() = default;

 public:
  /** @return Singleton reference to the D-Pad handler. */
  static DPad& GetInstance() { return Device::GetInstance().GetDPad(); }
};

#endif  // SANGO_PLUGIN_DEVICE_H