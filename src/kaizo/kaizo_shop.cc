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


#include "common.h"
#include "feature/feature_shop.h"
#include "game/constant/item.h"
#include "kaizo_item_catalog.h"

namespace kaizo {
// Built once from the shared catalog (see kaizo_item_catalog.h) rather than
// duplicating the item lists here. feature::Shop keeps a raw pointer to
// this array, so it needs static storage duration, hence the fixed-size
// buffer instead of a std::vector.
static ShopItem
    SHOP_ITEMS[SIZE(kStrategicItems) + SIZE(kBalls) + SIZE(kEvolutionStones)];

static void FillShopItems(const u16* ids, u32 count, u32& out_idx) {
  for (u32 i = 0; i < count; i++) {
    SHOP_ITEMS[out_idx].id = ids[i];
    SHOP_ITEMS[out_idx].price = 1;
    out_idx++;
  }
}

void PatchShopData() {
  u32 idx = 0;
  FillShopItems(kStrategicItems, SIZE(kStrategicItems), idx);
  FillShopItems(kBalls, SIZE(kBalls), idx);
  FillShopItems(kEvolutionStones, SIZE(kEvolutionStones), idx);

  feature::Shop::Initialize(SHOP_ITEMS, SIZE(SHOP_ITEMS));
}
} // namespace kaizo