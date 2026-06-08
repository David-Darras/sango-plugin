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
#include "hook_manager.h"
#include "game/constant/item.h"

namespace feature {
struct ShopItem {
  u32 id;
  u32 price;
};

struct ShopData {
  void* heap;

  ShopItem items[60];

  u32 type;
  u32 count;
  void* _0[7];
};

class Shop {
  MAKE_SINGLETON(Shop)
#define ADDRESS_LOAD_SHOP_ITEMS (0x00346554)

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kLoadShopItems, ADDRESS_LOAD_SHOP_ITEMS,
                            (uptr)LoadShopItemsHook);
  }

  static void LoadShopItemsHook(ShopData* data, u32 type, u32 id, void* heap,
                                bool for_sale) {
    HookManager::Call<void>(HookID::kLoadShopItems, data, type, id, heap,
                            for_sale);

    static const ShopItem MY_ITEMS[] = {
        // Stats
        {ITEM_HP_UP, 1},
        {ITEM_PROTEIN, 1},
        {ITEM_IRON, 1},
        {ITEM_CARBOS, 1},
        {ITEM_CALCIUM, 1},
        {ITEM_ZINC, 1},
        {ITEM_RARE_CANDY, 1},
        {ITEM_PP_MAX, 1},
        // Evolution Stones
        {ITEM_SUN_STONE, 1},
        {ITEM_MOON_STONE, 1},
        {ITEM_FIRE_STONE, 1},
        {ITEM_THUNDER_STONE, 1},
        {ITEM_WATER_STONE, 1},
        {ITEM_LEAF_STONE, 1},
        {ITEM_SHINY_STONE, 1},
        {ITEM_DUSK_STONE, 1},
        {ITEM_DAWN_STONE, 1},
        {ITEM_OVAL_STONE, 1},
        // Balls (26)
        {ITEM_DREAM_BALL, 1},
        {ITEM_PARK_BALL, 1},
        {ITEM_SPORT_BALL, 1},
        {ITEM_MOON_BALL, 1},
        {ITEM_FRIEND_BALL, 1},
        {ITEM_LOVE_BALL, 1},
        {ITEM_HEAVY_BALL, 1},
        {ITEM_LURE_BALL, 1},
        {ITEM_LEVEL_BALL, 1},
        {ITEM_FAST_BALL, 1},
        {ITEM_CHERISH_BALL, 1},
        {ITEM_QUICK_BALL, 1},
        {ITEM_HEAL_BALL, 1},
        {ITEM_DUSK_BALL, 1},
        {ITEM_PREMIER_BALL, 1},
        {ITEM_LUXURY_BALL, 1},
        {ITEM_TIMER_BALL, 1},
        {ITEM_REPEAT_BALL, 1},
        {ITEM_NEST_BALL, 1},
        {ITEM_DIVE_BALL, 1},
        {ITEM_NET_BALL, 1},
        {ITEM_SAFARI_BALL, 1},
        {ITEM_POKE_BALL, 1},
        {ITEM_GREAT_BALL, 1},
        {ITEM_ULTRA_BALL, 1},
        {ITEM_MASTER_BALL, 1},
        // Strategic Items
        {ITEM_AIR_BALLOON, 1},
        {ITEM_ASSAULT_VEST, 1},
        {ITEM_CHOICE_BAND, 1},
        {ITEM_CHOICE_SCARF, 1},
        {ITEM_CHOICE_SPECS, 1},
        {ITEM_FOCUS_SASH, 1},
        {ITEM_LEFTOVERS, 1},
        {ITEM_LIFE_ORB, 1},
        {ITEM_ROCKY_HELMET, 1},
        {ITEM_POWER_HERB, 1},
        {ITEM_SITRUS_BERRY, 1},
        {ITEM_EJECT_BUTTON, 1},
        {ITEM_FLAME_ORB, 1},
        {ITEM_SCOPE_LENS, 1},
        {ITEM_TOXIC_ORB, 1},
        {ITEM_GRIP_CLAW, 1}
    };
    static const u32 MY_ITEMS_COUNT = SIZE(MY_ITEMS);

    for (u32 i = 0; i < MY_ITEMS_COUNT; i++) {
      data->items[i].id = MY_ITEMS[i].id;
      data->items[i].price = MY_ITEMS[i].price;
    }
    data->count = MY_ITEMS_COUNT;
  }
};
}