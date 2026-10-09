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
/// The growth state of a Berry tree.
enum class BerryTreeState {
  kNone = 0, ///< No Berry is planted.
  kSeeded, ///< A seed is planted.
  kSprout, ///< A sprout is visible.
  kTall, ///< The stem is tall.
  kFlowering, ///< The plant has flowers.
  kBerries, ///< The Berries are ready.
  kWithered, ///< The plant is dead.
  kCount
};

static const ItemId BerryIdToItemId(u16 berry_id) {
  auto* table = (ItemId*)(overworld::address::kBerryIdTable);
  if (berry_id >= 67) return ItemId::kNone; ///< No Berry is planted.
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