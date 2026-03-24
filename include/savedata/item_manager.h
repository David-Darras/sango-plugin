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

#ifndef SANGO_PLUGIN_SAVEDATA_ITEM_MANAGER_H
#define SANGO_PLUGIN_SAVEDATA_ITEM_MANAGER_H

#include "common.h"
#include "savedata.h"

namespace savedata {

/**
 * @brief Manages the player's inventory (Bag) items.
 */
struct ItemManager {
  /** @brief Represents a single item stack in the bag. */
  struct ItemSlot {
    u16 id;     ///< Item ID.
    u16 count;  ///< Quantity held (max 999).
  };

  /** @brief Sub-menu for a specific pocket. */
  static void LoadPocketMenu(menu::PluginMenu& menu, void* args);

  /** @brief Main menu entry point for ItemManager. */
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  /** @brief Accessor for the singleton instance. */
  static ItemManager& GetInstance() {
    return SaveData::GetInstance().GetItemManager();
  }

  /** @brief Maximum quantity allowed for a single item stack. */
  static constexpr u16 kMaxItemCount = 999;

  /** @brief Pocket Size Constants. */
  static constexpr u32 kMaxNormalItems = 400;  ///< "Items" pocket.
  static constexpr u32 kMaxMedicine = 64;      ///< "Medicine" pocket.
  static constexpr u32 kMaxTMsHMs = 108;       ///< "TMs & HMs" pocket.
  static constexpr u32 kMaxBerries = 72;       ///< "Berries" pocket.
  static constexpr u32 kMaxKeyItems = 96;      ///< "Key Items" pocket.

  /** @brief Total number of slots across all pockets. */
  static constexpr u32 kTotalSlots =
      kMaxNormalItems + kMaxKeyItems + kMaxTMsHMs + kMaxMedicine + kMaxBerries;

  void* vtable;

  /** @brief Raw array containing all items sequentially. */
  ItemSlot items[kTotalSlots];

  // Helper methods to get pocket pointers
  ItemSlot* GetNormalItems() { return &items[0]; }
  ItemSlot* GetKeyItems() { return &items[kMaxNormalItems]; }
  ItemSlot* GetTMsHMs() { return &items[kMaxNormalItems + kMaxKeyItems]; }
  ItemSlot* GetMedicine() {
    return &items[kMaxNormalItems + kMaxKeyItems + kMaxTMsHMs];
  }
  ItemSlot* GetBerries() {
    return &items[kMaxNormalItems + kMaxKeyItems + kMaxTMsHMs + kMaxMedicine];
  }
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_ITEM_MANAGER_H