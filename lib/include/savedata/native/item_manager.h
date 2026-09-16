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
#include "pokemon/constant/item.h"
#include "savedata/native/savedata.h"

namespace savedata {
struct ItemManager {
  SINGLETON(ItemManager)
  struct ItemSlot {
    ItemId id;
    u16 count; ///< Quantity held (max 999).
  };

  STATIC_INLINE ItemManager& GetInstance() {
    return SaveData::GetInstance().GetItemManager();
  }

  static constexpr u16 kMaxItemCount = 999;

  static constexpr u32 kMaxNormalItems = 400;
  static constexpr u32 kMaxMedicine = 64;
  static constexpr u32 kMaxTMsHMs = 108;
  static constexpr u32 kMaxBerries = 72;
  static constexpr u32 kMaxKeyItems = 96;

  static constexpr u32 kTotalSlots =
      kMaxNormalItems + kMaxKeyItems + kMaxTMsHMs + kMaxMedicine + kMaxBerries;

  void* vtable;

  ItemSlot items[kTotalSlots];

  ItemSlot* GetNormalItems() { return &items[0]; }
  ItemSlot* GetKeyItems() { return &items[kMaxNormalItems]; }
  ItemSlot* GetTMsHMs() { return &items[kMaxNormalItems + kMaxKeyItems]; }

  ItemSlot* GetMedicine() {
    return &items[kMaxNormalItems + kMaxKeyItems + kMaxTMsHMs];
  }

  ItemSlot* GetBerries() {
    return &items[kMaxNormalItems + kMaxKeyItems + kMaxTMsHMs + kMaxMedicine];
  }

  void ClearAll() {
    for (auto& item : items) {
      item.count = 0;
      item.id = ItemId::kNone;
    }
  }
};
} // namespace savedata