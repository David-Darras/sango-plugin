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

#ifndef SANGO_PLUGIN_SAVEDATA_BOX_H
#define SANGO_PLUGIN_SAVEDATA_BOX_H

#include "common.h"
#include "savedata.h"

namespace savedata {
/**
 * @brief Manages the collection of storage boxes and their metadata.
 * * This structure handles the naming, visual appearance (wallpapers),
 * and unlocking progression of the box system.
 */
struct BoxManager {
  static void LoadMenu(menu::PluginMenu& menu);

  static BoxManager& GetInstance() {
    return SaveData::GetInstance().GetBoxManager();
  }

  /** @brief Maximum number of boxes available in the system. */
  static constexpr u32 kMaxBoxes = 31;

  /** @brief Maximum number of wallpapers available in the system. */
  static constexpr u32 kMaxWallpapers = 24;

  /** @brief Maximum length for a box title, including the null terminator. */
  static constexpr u32 kMaxTitleLength = 17;

  void* vtable;

  /** @brief Array of box names stored as UTF-16 strings. */
  c16 titles[kMaxBoxes][kMaxTitleLength];

  /** @brief Array of wallpaper IDs assigned to each box. */
  u8 wallpapers[kMaxBoxes];

  /** @brief Configuration flags for the box system state. */
  u8 flags;

  /** @brief Number of boxes currently accessible by the player. */
  u8 unlocked_count;

  /** @brief Index of the last visited or currently active box. */
  u8 active_box_index;
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_BOX_H
