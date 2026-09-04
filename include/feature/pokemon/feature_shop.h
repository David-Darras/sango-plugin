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
#include "feature/core/hook_manager.h"
#include "game/core/shop.h"

namespace feature {
class Shop {
  MAKE_SINGLETON(Shop)
  u32 count = 0;
  const ShopItem* items = nullptr;

  STATIC_INLINE void Initialize(const ShopItem* items, u32 count) {
    auto& shop = GetInstance();
    shop.items = items;
    shop.count = count;
    HookManager::Initialize(HookID::kLoadShopItems, ADDRESS_LOAD_SHOP_ITEMS,
                            (uptr)LoadShopItemsHook);
  }

  static void LoadShopItemsHook(ShopData* data, u32 type, u32 id, void* heap,
                                bool for_sale) {
    HookManager::Call<void>(HookID::kLoadShopItems, data, type, id, heap,
                            for_sale);

    auto& shop = GetInstance();
    if (shop.items == nullptr) return;

    for (u32 i = 0; i < shop.count; i++) {
      data->items[i].id = shop.items[i].id;
      data->items[i].price = shop.items[i].price;
    }
    data->count = shop.count;
  }
};
}