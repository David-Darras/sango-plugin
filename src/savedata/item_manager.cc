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

#include "savedata/item_manager.h"

#include "menu/plugin_menu.h"

namespace savedata {

/** @brief Internal state for the pocket menu. */
struct PocketMenuState {
  u32 pocket_id = 0;
  u32 slot_idx = 0;
};

static PocketMenuState state;

void ItemManager::LoadPocketMenu(menu::PluginMenu& menu) {
  ItemManager& data = GetInstance();
  ItemSlot* target_pocket = nullptr;
  u32 max_slots = 0;

  switch (state.pocket_id) {
    case 1:
      target_pocket = data.GetKeyItems();
      max_slots = kMaxKeyItems;
      break;
    case 2:
      target_pocket = data.GetTMsHMs();
      max_slots = kMaxTMsHMs;
      break;
    case 3:
      target_pocket = data.GetMedicine();
      max_slots = kMaxMedicine;
      break;
    case 4:
      target_pocket = data.GetBerries();
      max_slots = kMaxBerries;
      break;
    case 0:
    default:
      target_pocket = data.GetNormalItems();
      max_slots = kMaxNormalItems;
      break;
  }

  if (!target_pocket) return;

  menu.Add("Slot Select", state.slot_idx)
      .WithBounds(0, max_slots - 1)
      .WithRefresh();

  menu.Add("Item ID", target_pocket[state.slot_idx].id);

  menu.Add("Quantity", target_pocket[state.slot_idx].count)
      .WithBounds(0, kMaxItemCount);
}

void ItemManager::LoadMenu(menu::PluginMenu& menu) {
  static const c8* pocket_names[] = {"Items", "Key Items", "TMs & HMs",
                                     "Medicine", "Berries"};

  menu.Add("Pocket Select", state.pocket_id)
      .WithArray(pocket_names, 5)
      .WithRefresh();

  menu.Add("Edit Selected Pocket", LoadPocketMenu);
}

}  // namespace savedata