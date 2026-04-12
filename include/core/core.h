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

#ifndef SANGO_PLUGIN_CORE_H
#define SANGO_PLUGIN_CORE_H

#include "common.h"

/** * @name Entrypoint & Main Addresses
 * @{ */
#define ADDRESS_RENDER_HOME_BUTTON_BLOCKED (ADDRESS_SOURCE)
#define ADDRESS_CORE (0x0063106C)
#define ADDRESS_GAME_MANAGER (0x08C69094)
/** @} */

/** * @name HID (Human Interface Device) Addresses
 * @{ */
#define ADDRESS_DEVICE_GET_CONTROLLER (0x00117550)
#define ADDRESS_CONTROLLER_IS_KEY_PRESSED (0x004C86EC)
#define ADDRESS_CONTROLLER_IS_KEY_RELEASED (0x004C86B4)
#define ADDRESS_CONTROLLER_IS_KEY_REPEATED (0x004C863C)
#define ADDRESS_CONTROLLER_IS_KEY_DOWN (0x004C8604)

#define ADDRESS_DEVICE_GET_DPAD (0x004C80E0)
#define ADDRESS_DPAD_IS_PRESSED (0x004C8030)
#define ADDRESS_DPAD_IS_REPEATED (0x004C7F50)
#define ADDRESS_DPAD_IS_DOWN (0x004C7EF8)
#define ADDRESS_DPAD_IS_DOWN_2 (0x004C7BEC)
#define ADDRESS_DPAD_IS_RELEASED_2 (0x004C7C0C)

#define ADDRESS_DEVICE_GET_TOUCHSCREEN (0x004C810C)
#define ADDRESS_TOUCHSCREEN_IS_PRESSED (0x004C77D4)
#define ADDRESS_TOUCHSCREEN_IS_REPEATED (0x004C7774)
#define ADDRESS_TOUCHSCREEN_IS_RELEASED (0x004C77AC)
#define ADDRESS_TOUCHSCREEN_IS_DOWN (0x004C79F8)
#define ADDRESS_TOUCHSCREEN_GET_X (0x004C794C)
#define ADDRESS_TOUCHSCREEN_GET_Y (0x004C7974)
/** @} */

/** * @name Graphics Addresses
 * @{ */
#define ADDRESS_GRAPHICS_GET_FRAMEBUFFER (0x0012FA94)
#define ADDRESS_GRAPHICS_BIND_FRAMEBUFFER (0x003717E0)
#define ADDRESS_GRAPHICS_ENABLE_SCISSOR (0x003A30A8)
#define ADDRESS_GRAPHICS_DISABLE_SCISSOR (0x003A3284)
#define ADDRESS_GRAPHICS_BEGIN_RENDER (0x00175A58)
#define ADDRESS_GRAPHICS_SET_TEXT_SCALE (0x003A3498)
#define ADDRESS_GRAPHICS_DRAW_TEXT (0x003A44C4)
#define ADDRESS_GRAPHICS_DRAW_RECT (0x00175BDC)
/** @} */

/** * @name Sound & Audio Addresses
 * @{ */
#define ADDRESS_SOUND_PLAY_POKEMON_CRY (0x004500C0)
#define ADDRESS_SOUND_CHANGE_POKEMON_CRY_VOLUME (0x0044EB4C)
#define ADDRESS_SOUND_PLAY_SOUND_EFFECT (0x0044FC88)
#define ADDRESS_SOUND_PLAY_BACKGROUND_MUSIC (0x044E6CC)
/** @} */

/** * @name Filesystem Addresses
 * @{ */
#define ADDRESS_FS_MOUNT_SDMC (0x001F7CFC)
#define ADDRESS_FS_CREATE_FILE (0x001F5C50)
#define ADDRESS_FS_DELETE_FILE (0x001F5CCC)

#define ADDRESS_FILE_OPEN (0x00123FA8)
#define ADDRESS_FILE_READ (0x00145CF0)
#define ADDRESS_FILE_WRITE (0x001F5BE0)
/** @} */

/** * @name OS Information
 * @{ */
#define ADDRESS_OS_READ_ONLY_KERNEL_INFO (0x1FF80000)
#define ADDRESS_HEAP_TABLE (0x08000000)
#define ADDRESS_ALLOC (0x001235A8)
/** @} */

/** * @name Standard Library Internal Addresses
 * @{ */
#define ADDRESS_STD_WCSLEN \
  (0x001003E8 | 1)  ///< Standard wcslen function (Thumb mode)
#define ADDRESS_STD_VSWPRINTF \
  (0x001004B8 | 1)         ///< Standard vswprintf function (Thumb mode)
#define BUFFER_SIZE (128)  ///< Default buffer size for formatted strings
/** @} */

#define ADDRESS_POKEMON_DATA_ACCESSOR_ENCRYPT (0x00168DCC)
#define ADDRESS_POKEMON_DATA_ACCESSOR_DECRYPT (0x00168E3C)
#define ADDRESS_POKEMON_DATA_ACCESSOR_INITIALIZE (0x0012C2BC)

#define ADDRESS_POKEMON_UTILS_LOAD_POKEMON_EXPERIENCE_TABLE (0x0016886C)
#define ADDRESS_POKEMON_UTILS_GET_POKEMON_MINIMUM_EXPERIENCE (0x00168A28)
#define ADDRESS_POKEMON_UTILS_GET_POKEMON_LEVEL_FROM_EXPERIENCE (0x0014FDAC)
#define ADDRESS_POKEMON_UTILS_IS_SHINY (0x00168F48)
#define ADDRESS_POKEMON_UTILS_TO_SHINY (0x00168F6C)
#define ADDRESS_POKEMON_UTILS_TO_NORMAL (0x00168FCC)

#define ADDRESS_STEREO_CAMERA_UPDATE_MATRICES (0x00377C10)
#define ADDRESS_STEREO_CAMERA_UPDATE_LOOK_AT (0x001483DC)
#define ADDRESS_STEREO_CAMERA_SET_PROJECTION_MATRIX (0x003939CC)

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
  static Core& GetInstance() { return *(Core*)ADDRESS_CORE; }

  /**
   * @brief Accesses the hardware device manager (Input/HID).
   * @return A reference to the Device manager.
   */
  Device& GetDevice() const { return *device_; }

  /**
   * @brief Accesses the global graphics manager.
   * @return A reference to the Graphics manager.
   */
  Graphics& GetGraphics() const { return *graphics_; }

 private:
  /** @brief Private constructor to prevent manual instantiation. */
  Core() = default;

  Device* device_;      ///< Pointer to the internal Device manager instance.
  Graphics* graphics_;  ///< Pointer to the internal Graphics manager instance.
};

#endif  // SANGO_PLUGIN_CORE_H
