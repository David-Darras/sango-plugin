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

#ifndef SANGO_PLUGIN_SAVEDATA_BAG_MANAGER_H
#define SANGO_PLUGIN_SAVEDATA_BAG_MANAGER_H

#include "common.h"
#include "savedata.h"

namespace savedata {

/**
 * @brief Manages the player's inventory (Bag) state and metadata.
 * * This structure handles the organization of item pockets, registered
 * shortcut items, and the history of recently used items.
 */
struct BagManager {
  /**
   * @brief Builds the menu interface for Bag-related cheats/modifications.
   * @param menu Reference to the PluginMenu instance.
   */
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  /**
   * @brief Returns the singleton instance of the BagManager from the SaveData.
   * @return Reference to the unique BagManager instance.
   */
  static BagManager& GetInstance() {
    return SaveData::GetInstance().GetBagManager();
  }

  /** @brief Maximum number of inventory pockets (categories). */
  static constexpr u32 kMaxPockets = 5;

  /** @brief Maximum number of registered shortcut items. */
  static constexpr u32 kMaxRegisteredItems = 4;

  /** @brief Maximum number of entries in the item usage history log. */
  static constexpr u32 kMaxUsageHistory = 12;

  void* vtable;

  /** @brief Custom order/sorting of the inventory pockets. */
  u16 pocket_order[kMaxPockets];

  /** @brief Item IDs assigned to the quick-access registered buttons. */
  u16 registered_items[kMaxRegisteredItems];
  u16 padding;

  /** @brief List of recently used item IDs for quick access in the UI. */
  u16 last_items_used[kMaxUsageHistory];
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_BAG_MANAGER_H
