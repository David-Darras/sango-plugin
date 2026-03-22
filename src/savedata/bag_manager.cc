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
#include "savedata/bag_manager.h"

#include "menu/plugin_menu.h"

namespace savedata {

void BagManager::LoadMenu(menu::PluginMenu& menu) {
  static u8 pocket_idx = 0;
  static u8 register_idx = 0;
  static u8 history_idx = 0;
  static const c8* pocket_type[kMaxPockets] = {"Items", "Medecine", "TMs & HMs",
                                               "Berries", "Key Items"};

  BagManager& data = GetInstance();

  menu.Add("Pocket Select", pocket_idx)
      .WithBounds(0, kMaxPockets - 1)
      .WithRefresh()
      .Add("Pocket Type", data.pocket_order[pocket_idx])
      .WithArray(pocket_type, kMaxPockets)
      .Add("Shortcut Select", register_idx)
      .WithBounds(0, kMaxRegisteredItems - 1)
      .WithRefresh()
      .Add("Registered Item", data.registered_items[register_idx])
      .Add("History Select", history_idx)
      .WithBounds(0, kMaxUsageHistory - 1)
      .WithRefresh()
      .Add("Last Item Used", data.last_items_used[history_idx]);
}

}  // namespace savedata