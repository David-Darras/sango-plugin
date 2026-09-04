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

#include "game/constant/item.h"
#include "game/savedata/item_manager.h"
#include "kaizo/kaizo_item_catalog.h"

namespace kaizo {
void PatchBag() {
  auto& manager = savedata::ItemManager::GetInstance();
  {
    auto* slot = manager.GetTMsHMs();
    for (u32 i = 0; i < savedata::ItemManager::kMaxTMsHMs - 16; i++) {
      slot->count = 1;
      slot->id = static_cast<ItemID>(static_cast<u16>(ItemID::kTm01) + i);
      slot++;
    }
    for (u32 i = savedata::ItemManager::kMaxTMsHMs - 16;
         i < savedata::ItemManager::kMaxTMsHMs; i++) {
      slot->count = 0;
      slot->id = ItemID::kNone;
    }
  }
  {
    auto* slot = manager.GetMedicine();
    for (u32 i = 0; i < savedata::ItemManager::kMaxMedicine; i++) {
      slot->count = 0;
      slot->id = ItemID::kNone;
      slot++;
    }
  }
  auto* slot = manager.GetNormalItems();
  {
    for (u32 i = 0; i < SIZE(kStrategicItems); i++) {
      slot->count = 100;
      slot->id = kStrategicItems[i];
      slot++;
    }
  }
  {
    for (u32 i = 0; i < SIZE(kEvolutionStones); i++) {
      slot->count = 100;
      slot->id = kEvolutionStones[i];
      slot++;
    }
  }
  {
    for (u32 i = 0; i < SIZE(kBalls); i++) {
      slot->count = 100;
      slot->id = kBalls[i];
      slot++;
    }
  }
}
}