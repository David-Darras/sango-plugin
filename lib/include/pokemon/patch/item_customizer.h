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
 * @file item_customizer.h
 * @brief Lets a product change the data of the items.
 *
 * @see docs/tutorials/11-change-items-and-shops.md
 */

#pragma once

#include "common.h"

namespace pokemon {
struct ItemData;
}

namespace pokemon {

/// Calls a callback of the product each time the game reads the data of an item.
class ItemCustomizer {
  MAKE_SINGLETON(ItemCustomizer)

public:
  /// true: removes the EV limits of the items (100 for one stat with a
  /// vitamin, 510 in total). The change applies the next time that the game
  /// reads the data of an item.
  bool remove_limit = false;

  /// Called each time the game reads the data of an item. Change the data.
  typedef void (*ItemDataCallback)(ItemData* item);
  ItemDataCallback on_item_data = nullptr;

  static void Initialize();

  /// The number of game instructions that the EV limit patch changes.
  static constexpr u32 kLimitPatchCount = 9;

private:
  /// Writes or removes the EV limit patch to match `remove_limit`.
  static void UpdateLimitPatch();

  // The first instructions of the game function read the pc register: a
  // normal hook cannot move them, so the hook rewrites the full function.
  static u32 GetParamHook(ItemData* item, u32 param_id);
  static s32 GetParam(const ItemData* item, s32 param_id);

  u32 original_limits_[kLimitPatchCount] = {};
  bool is_limit_removed_ = false;
};

} // namespace pokemon
