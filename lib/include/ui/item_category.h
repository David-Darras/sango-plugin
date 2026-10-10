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
 * @file item_category.h
 * @brief The categories of the items (the pockets of the Bag), for the tabs
 *        of the grid of items.
 */

#pragma once

#include "common.h"

namespace ui {
/// A category of items: a tab of the grid of items.
enum class ItemCategory : u8 {
  kOther, ///< The Items pocket, without a held effect.
  kBall, ///< The Poké Balls.
  kMedicine, ///< The Medicine pocket.
  kHeld, ///< The Items pocket, with an effect when a Pokémon holds it.
  kBerry, ///< The Berries pocket.
  kMachine, ///< The TMs and HMs pocket.
  kKey, ///< The Key Items pocket.
  kCount,
};

/**
 * @brief The category of each item.
 *
 * The data of each item is a file of the game: the index reads a few items
 * in each frame (Step()), the first time that the menu needs it.
 */
class ItemCategories {
public:
  /// The number of items.
  static constexpr u32 kItemCount = 776;

  /// Starts to read the items (one time only).
  static void Request();
  /// Reads the next items. Call it one time for each frame.
  static void Step();
  /// Returns true when the index knows all the items.
  static bool IsReady();
  /// Returns the number of items that the index knows.
  static u32 GetProgress();
  /// Returns the category of an item. Call it only when IsReady().
  static ItemCategory Get(u32 item);
  /// Returns the name of a category, for the tabs.
  static const c8* GetName(ItemCategory category);
};
} // namespace ui
