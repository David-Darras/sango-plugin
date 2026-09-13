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

namespace global_data {
struct Item;
}

namespace pokemon {

class ItemCustomizer {
  MAKE_SINGLETON(ItemCustomizer)

public:
  bool remove_limit = false;

  /// Lets a product rewrite an item's data every time the game reads it.
  typedef void (*ItemDataCallback)(global_data::Item* item);
  ItemDataCallback on_item_data = nullptr;

  static void Initialize();

private:
  /* The first instructions use the pc register so we have to rewrite all the code */
  static u32 GetParamHook(global_data::Item* item, u32 param_id);
  static s32 GetParam(const global_data::Item* item, s32 param_id);
};

} // namespace pokemon
