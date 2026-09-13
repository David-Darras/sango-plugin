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
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/species.h"
#include "pokemon/native/shop.h"

namespace pokemon {

struct ShopPokemon {
  SpeciesId species;
  Form form;
  u8 level;
  u32 price;
};

constexpr u32 kCoin = 20;

constexpr ShopPokemon kShopPokemons[] = {
    {SpeciesId::kAbra, Form::kNormal, 9, 120 * kCoin},
    {SpeciesId::kClefairy, Form::kNormal, 8, 750 * kCoin},
    {SpeciesId::kDratini, Form::kNormal, 18, 4600 * kCoin},
    {SpeciesId::kScyther, Form::kNormal, 25, 6500 * kCoin},
    {SpeciesId::kPorygon, Form::kNormal, 26, 9999 * kCoin},
};

/// Rewrites what a mart sells: its own item list, or Pokémon sold through a
/// placeholder item whose name and description are patched on the fly.
class CustomShop {
  MAKE_SINGLETON(CustomShop)

public:
  static constexpr u32 kAnyShop = 0xFFFFFFFF;
  static constexpr u32 kDefaultShopId = kAnyShop;

  static void Initialize();
  static void SetItems(const ShopItem* items, u32 count);
  static void SetPokemons(const ShopPokemon* pokemons, u32 count,
                          u32 shop_id = kDefaultShopId);
  static bool PatchItemName(ItemId item, String* output);

private:
  enum Offset : u32 {
    kOffsetCursor = 0x18,
    kOffsetDispCursor = 0x1C,
    kOffsetBuyCount = 0x24,
    kOffsetItems = 0x28,
    kOffsetLayout = 0x274,
  };

  static constexpr ItemId kIcon = ItemId::kPokeBall;
  static constexpr u32 kParamSlot = 0;
  static constexpr u32 kIconPane = 62;

  static void GetSpeciesName(SpeciesId species, String* output);
  static bool IsPokemonShop(const ShopData* data);
  static const ShopPokemon* GetPokemon(const ShopData* data, s32 index);
  static const ShopPokemon* GetSelectedPokemon();
  static bool GivePokemon(const ShopPokemon& entry);
  static bool AddPokemonToTeam(u32 slot);
  static void LoadShopItemsHook(ShopData* data, ShopType type, u32 id,
                                void* heap, bool for_sale);
  static void UnloadShopItemsHook(ShopData* data);
  static String* GetItemNameHook(ShopData* data, s32 index);
  static String* GetItemInfoHook(ShopData* data, s32 index);
  static void DispItemInfoHook(uptr event);
  static void PurchaseItemHook(uptr event);
  static bool AddItemHook(void* bag, ItemId item, u16 count, void* heap);
  static s32 GetSelectedIndex(uptr event);

  u32 count = 0;
  const ShopItem* items = nullptr;
  const ShopPokemon* pokemons = kShopPokemons;
  u32 pokemon_count = SIZE(kShopPokemons);
  u32 pokemon_shop_id = kDefaultShopId;
  ShopData* pokemon_data = nullptr;
  bool skip_bag_add = false;
};
} // namespace pokemon
