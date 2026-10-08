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
 * @file berry_tree_manager.h
 * @brief The Berry trees.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/item.h"
#include "savedata/native/savedata.h"

namespace savedata {
enum class BerryTreeState {
  kNone = 0, ///< No berry planted
  kSeeded, ///< A seed has been planted
  kSprout, ///< A sprout has emerged
  kTall, ///< The stem has grown tall
  kFlowering, ///< The plant is in bloom
  kBerries, ///< Berries are ready to harvest
  kWithered, ///< The plant has withered
  kCount
};

static const ItemId BerryIdToItemId(u16 berry_id) {
  auto* table = (ItemId*)(overworld::address::kBerryIdTable);
  if (berry_id >= 67) return ItemId::kNone;
  return table[berry_id];
}

/// The state of one Berry tree.
struct BerryTree {
  BerryTreeState state;
  u16 elapsed_minutes;
  u16 moisture_minutes;
  u16 berry_id;
  f32 count;
  bool use_default_berry;
};


/// The Berry trees of the overworld.
struct BerryTreeManager {
  SINGLETON(BerryTreeManager)
  STATIC_INLINE BerryTreeManager& GetInstance() {
    return SaveData::GetInstance().GetBerryTreeManager();
  }

  static constexpr u32 kMaxBerryTrees = 100;

  uptr vtable;
  BerryTree berry_trees[kMaxBerryTrees];
};
} // namespace savedata