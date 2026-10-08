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
 * @file custom_shop.h
 * @brief Changes the items of the shops, and sells Pokémon in the shops.
 *
 * @see docs/tutorials/11-change-items-and-shops.md
 */

#pragma once

#include "common.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/species.h"
#include "pokemon/native/shop_data.h"

namespace pokemon {

/// One Pokémon that a shop sells.
struct ShopPokemon {
  SpeciesId species;
  FormId form;
  u8 level;
  u32 price;
};

/// The price unit of the default Pokémon list.
constexpr u32 kCoin = 20;

/// The default Pokémon of the Pokémon shop. See CustomShop::EnablePokemonShop().
constexpr ShopPokemon kShopPokemons[] = {
    {SpeciesId::kAbra, FormId::kNormal, 9, 120 * kCoin},
    {SpeciesId::kClefairy, FormId::kNormal, 8, 750 * kCoin},
    {SpeciesId::kDratini, FormId::kNormal, 18, 4600 * kCoin},
    {SpeciesId::kScyther, FormId::kNormal, 25, 6500 * kCoin},
    {SpeciesId::kPorygon, FormId::kNormal, 26, 9999 * kCoin},
};

/// Changes what a shop sells: a list of items, or a list of Pokémon. A
/// Pokémon uses the Poké Ball item; the plugin changes its name and its
/// description.
class CustomShop {
  MAKE_SINGLETON(CustomShop)

public:
  /// A shop id that means "all the shops".
  static constexpr u32 kAnyShop = 0xFFFFFFFF;
  static constexpr u32 kDefaultShopId = kAnyShop;

  /// true: the normal Poké Marts sell Pokémon instead of items. Use
  /// EnablePokemonShop() to change it.
  STATIC_INLINE bool IsPokemonShopEnabled() {
    return GetInstance().is_pokemon_shop_enabled_;
  }

  /// Makes the normal Poké Marts sell Pokémon (true) or items (false).
  /// The cheat code CheatCodeId::kPokemonShop does the same from the menu.
  static void EnablePokemonShop(bool is_enabled);

  /// true: writes the type and the id of each shop that opens to the log.
  bool log_shop_ids = false;

  static void Initialize();
  /**
   * @brief Replaces the items of a shop.
   * @param items The items and their prices. The array must stay in memory.
   * @param count The number of items (60 at most).
   * @param shop_id The shop. The log shows the id when a shop opens.
   */
  static void SetItems(const ShopItem* items, u32 count,
                       u32 shop_id = kAnyShop);
  /**
   * @brief Sells Pokémon in a normal Poké Mart.
   * @param pokemons The Pokémon. The array must stay in memory. Null: sell no Pokémon.
   * @param count The number of Pokémon.
   * @param shop_id The shop, or kAnyShop.
   */
  static void SetPokemons(const ShopPokemon* pokemons, u32 count,
                          u32 shop_id = kDefaultShopId);
  /// Writes the name of the selected Pokémon instead of the item name.
  static bool PatchItemName(ItemId item, String* output);
  /// Gives a level 100 Pokémon that holds `item`.
  static bool GiveMega(SpeciesId species, ItemId item = ItemId::kLifeOrb);
  /// Gives a Pokémon of the list.
  static bool GivePokemon(const ShopPokemon& entry);
  /// Writes the name of a species.
  static void GetSpeciesName(SpeciesId species, String* output);

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

  static bool IsPokemonShop(const ShopData* data);
  static const ShopPokemon* GetPokemon(const ShopData* data, s32 index);
  static const ShopPokemon* GetSelectedPokemon();
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
  u32 items_shop_id = kAnyShop;
  const ShopPokemon* pokemons = kShopPokemons;
  u32 pokemon_count = SIZE(kShopPokemons);
  u32 pokemon_shop_id = kDefaultShopId;
  ShopData* pokemon_data = nullptr;
  bool skip_bag_add = false;
  bool is_pokemon_shop_enabled_ = false;
};
} // namespace pokemon
