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

/**
 * @file item_category.cc
 * @brief The categories of the items (the pockets of the Bag).
 *
 * The declarations are in ui/item_category.h.
 */

#include "ui/item_category.h"

#include "pokemon/native/item_data.h"

namespace ui {
namespace {
// The pockets of the Bag in the data of an item.
constexpr u8 kPocketItems = 0;
constexpr u8 kPocketMedicine = 1;
constexpr u8 kPocketMachines = 2;
constexpr u8 kPocketBerries = 3;
constexpr u8 kPocketKeyItems = 4;

// The items that the index reads in one frame (one file each).
constexpr u32 kItemsPerFrame = 4;

// The Poké Balls (ItemId).
const u16 kBalls[] = {1,   2,   3,   4,   5,   6,   7,   8,   9,
                      10,  11,  12,  13,  14,  15,  16,  492, 493,
                      494, 495, 496, 497, 498, 499, 500, 576};

u8 g_categories[ItemCategories::kItemCount];
u32 g_progress = 0;
bool g_is_requested = false;

bool IsBall(u32 item) {
  for (u32 ball : kBalls) {
    if (ball == item) return true;
  }
  return false;
}
} // namespace

void ItemCategories::Request() { g_is_requested = true; }

void ItemCategories::Step() {
  if (!g_is_requested) return;
  for (u32 n = 0; n < kItemsPerFrame && g_progress < kItemCount; n++) {
    const u32 item = g_progress++;
    ItemCategory category = ItemCategory::kOther;
    if (IsBall(item)) {
      category = ItemCategory::kBall;
    } else if (item != 0) {
      pokemon::ItemData data((ItemId)item);
      switch (data.overworld_pocket) {
        case kPocketMedicine:
          category = ItemCategory::kMedicine;
          break;
        case kPocketMachines:
          category = ItemCategory::kMachine;
          break;
        case kPocketBerries:
          category = ItemCategory::kBerry;
          break;
        case kPocketKeyItems:
          category = ItemCategory::kKey;
          break;
        case kPocketItems:
        default:
          category = data.hold_effect != 0 ? ItemCategory::kHeld
                                           : ItemCategory::kOther;
          break;
      }
    }
    g_categories[item] = (u8)category;
  }
}

bool ItemCategories::IsReady() { return g_progress >= kItemCount; }

u32 ItemCategories::GetProgress() { return g_progress; }

ItemCategory ItemCategories::Get(u32 item) {
  if (item >= kItemCount) return ItemCategory::kOther;
  return (ItemCategory)g_categories[item];
}

const c8* ItemCategories::GetName(ItemCategory category) {
  switch (category) {
    case ItemCategory::kBall:
      return "Balls";
    case ItemCategory::kMedicine:
      return "Medicine";
    case ItemCategory::kHeld:
      return "Held";
    case ItemCategory::kBerry:
      return "Berries";
    case ItemCategory::kMachine:
      return "TMs";
    case ItemCategory::kKey:
      return "Key";
    default:
      return "Other";
  }
}
} // namespace ui
