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
#include "utils.h"
#include "feature/core/hook_manager.h"
#include "game/constant/form.h"
#include "game/constant/item.h"
#include "game/constant/move.h"
#include "game/constant/species.h"
#include "game/core/shop.h"
#include "game/global_data/gift_pokemon.h"
#include "game/renderer/app_layout_manager.h"
#include "game/savedata/pokemon_team.h"

namespace feature {
struct ShopPokemon {
  Species species;
  Form form;
  u8 level;
  u32 price;
};

constexpr u32 kCoin = 20;

constexpr ShopPokemon kShopPokemons[] = {
    {Species::kAbra, Form::kNormal, 9, 120 * kCoin},
    {Species::kClefairy, Form::kNormal, 8, 750 * kCoin},
    {Species::kDratini, Form::kNormal, 18, 4600 * kCoin},
    {Species::kScyther, Form::kNormal, 25, 6500 * kCoin},
    {Species::kPorygon, Form::kNormal, 26, 9999 * kCoin},
};

class Shop {
  MAKE_SINGLETON(Shop)
  static constexpr u32 kAnyShop = 0xFFFFFFFF;
  static constexpr u32 kDefaultShopId = kAnyShop;
  static constexpr ItemId kIcon = ItemId::kPokeBall;
  static constexpr u32 kParamSlot = 0;

  u32 count = 0;
  const ShopItem* items = nullptr;

  const ShopPokemon* pokemons = kShopPokemons;
  u32 pokemon_count = SIZE(kShopPokemons);
  u32 pokemon_shop_id = kDefaultShopId;

  ShopData* pokemon_data = nullptr;
  bool skip_bag_add = false;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kLoadShopItems, ADDRESS_LOAD_SHOP_ITEMS,
                            (uptr)LoadShopItemsHook);
    HookManager::Initialize(HookID::kUnloadShopItems,
                            ADDRESS_UNLOAD_SHOP_ITEMS,
                            (uptr)UnloadShopItemsHook);
    HookManager::Initialize(HookID::kShopGetItemName,
                            ADDRESS_SHOP_GET_ITEM_NAME,
                            (uptr)GetItemNameHook);
    HookManager::Initialize(HookID::kShopGetItemDescription,
                            ADDRESS_SHOP_GET_ITEM_INFO,
                            (uptr)GetItemInfoHook);
    HookManager::Initialize(HookID::kShopDisplayItemDescription,
                            ADDRESS_SHOP_DISPLAY_ITEM_INFO,
                            (uptr)DispItemInfoHook);
    HookManager::Initialize(HookID::kShopPurchaseItem,
                            ADDRESS_SHOP_PURCHASE_ITEM,
                            (uptr)PurchaseItemHook);
    HookManager::Initialize(HookID::kBagAddItem, ADDRESS_BAG_ADD_ITEM,
                            (uptr)AddItemHook);
  }

  STATIC_INLINE void SetItems(const ShopItem* items, u32 count) {
    auto& shop = GetInstance();
    shop.items = items;
    shop.count = count;
  }

  STATIC_INLINE void
  SetPokemons(const ShopPokemon* pokemons, u32 count,
              u32 shop_id = kDefaultShopId) {
    auto& shop = GetInstance();
    shop.pokemons = pokemons;
    shop.pokemon_count = count;
    shop.pokemon_shop_id = shop_id;
  }

  static bool PatchItemName(u32 item_id, String* output) {
    if (item_id != static_cast<u32>(kIcon)) return false;
    const ShopPokemon* entry = GetSelectedPokemon();
    if (entry == nullptr) return false;
    GetSpeciesName(entry->species, output);
    return true;
  }

private:
  enum Offset : u32 {
    kOffsetCursor = 0x18,
    kOffsetDispCursor = 0x1C,
    kOffsetBuyCount = 0x24,
    kOffsetItems = 0x28,
    kOffsetLayout = 0x274,
  };

  static constexpr u32 kIconPane = 62;

  STATIC_INLINE void GetSpeciesName(Species species, String* output) {
    ((void(*)(String*, u16))ADDRESS_GET_SPECIES_NAME)(
        output, static_cast<u16>(species));
  }

  static bool IsPokemonShop(const ShopData* data) {
    return data != nullptr && data == GetInstance().pokemon_data;
  }

  static const ShopPokemon* GetPokemon(const ShopData* data, s32 index) {
    auto& shop = GetInstance();
    if (!IsPokemonShop(data)) return nullptr;
    if (index < 0 || (u32)index >= shop.pokemon_count) return nullptr;
    return &shop.pokemons[index];
  }

  static const ShopPokemon* GetSelectedPokemon() {
    auto& shop = GetInstance();
    if (shop.pokemon_data == nullptr) return nullptr;
    uptr event = (uptr)shop.pokemon_data - kOffsetItems;
    s32 index = READ32(event + kOffsetCursor) +
                READ32(event + kOffsetDispCursor);
    return GetPokemon(shop.pokemon_data, index);
  }

  static bool GivePokemon(const ShopPokemon& entry) {
    using Gift = global_data::GiftPokemon;

    auto& param = Gift::GetInstance(kParamSlot);
    const Gift backup = param;

    param.species = static_cast<u32>(entry.species);
    param.form = entry.form;
    param.level = entry.level;
    param.shiny = Gift::Shiny::kRandom;
    param.ability_slot = Gift::kRandomAbility;
    param.nature = Gift::kRandomNature;
    param.item = Gift::kRandomItem;
    param.gender = Gift::GenderRoll::kRandom;
    param.egg_place = Gift::kNotAnEgg;
    param.move = MoveId::kNone;
    for (u32 i = 0; i < SIZE(param.iv); i++) {
      param.iv[i] = Gift::kRandomIv;
      param.contest[i] = 0;
    }

    bool given = AddPokemonToTeam(kParamSlot);

    param = backup;
    return given;
  }

  static bool AddPokemonToTeam(u32 slot) {
    s32 args[3] = {2 * (s32)sizeof(s32), (s32)slot, 0};

    Hook* hook = HookManager::GetInstance().Get(
        HookID::kScriptAddPokemonToTeam);
    if (hook != nullptr && hook->IsInitialized()) {
      return hook->CallOriginal<s32, void*, const s32*>(nullptr, args) != 0;
    }
    return ((s32(*)(void*, const s32*))ADDRESS_SCRIPT_ADD_POKEMON_TO_TEAM)(
               nullptr, args) != 0;
  }

  static void LoadShopItemsHook(ShopData* data, u32 type, u32 id, void* heap,
                                bool for_sale) {
    HookManager::Call<void>(HookID::kLoadShopItems, data, type, id, heap,
                            for_sale);

    auto& shop = GetInstance();
    shop.pokemon_data = nullptr;
    ui::LogApplication::Print(u"shop: type=%u id=%u", type, id);

    if (shop.pokemons != nullptr && shop.pokemon_count > 0 &&
        type == static_cast<u32>(ShopType::kNormal) &&
        (shop.pokemon_shop_id == kAnyShop || shop.pokemon_shop_id == id)) {
      u32 count = shop.pokemon_count < ShopData::kMaxItems
                    ? shop.pokemon_count
                    : ShopData::kMaxItems;
      for (u32 i = 0; i < count; i++) {
        data->items[i].id = static_cast<u32>(kIcon);
        data->items[i].price = shop.pokemons[i].price;
      }
      data->count = count;
      shop.pokemon_data = data;
      return;
    }

    if (shop.items == nullptr) return;

    for (u32 i = 0; i < shop.count; i++) {
      data->items[i].id = shop.items[i].id;
      data->items[i].price = shop.items[i].price;
    }
    data->count = shop.count;
  }

  static void UnloadShopItemsHook(ShopData* data) {
    auto& shop = GetInstance();
    if (shop.pokemon_data == data) shop.pokemon_data = nullptr;
    HookManager::Call<void>(HookID::kUnloadShopItems, data);
  }

  static String* GetItemNameHook(ShopData* data, s32 index) {
    const ShopPokemon* entry = GetPokemon(data, index);
    if (entry == nullptr) {
      return HookManager::Call<String*>(HookID::kShopGetItemName, data, index);
    }
    GetSpeciesName(entry->species, data->name_buffer);
    return data->name_buffer;
  }

  static String* GetItemInfoHook(ShopData* data, s32 index) {
    const ShopPokemon* entry = GetPokemon(data, index);
    if (entry == nullptr) {
      return HookManager::Call<String*>(HookID::kShopGetItemDescription, data,
                                        index);
    }
    c16 description[128];
    GetSpeciesName(entry->species, data->info_buffer);
    Utils::Format(description, u"%ls / Lv. %u", data->info_buffer->GetBuffer(),
                  entry->level);
    data->info_buffer->Set(description);
    return data->info_buffer;
  }

  static void DispItemInfoHook(uptr event) {
    HookManager::Call<void>(HookID::kShopDisplayItemDescription, event);

    if (!IsPokemonShop((ShopData*)(event + kOffsetItems))) return;
    auto* layout = (AppLayoutManager*)READ32(event + kOffsetLayout);
    if (layout != nullptr) layout->HidePane(0, kIconPane);
  }

  static void PurchaseItemHook(uptr event) {
    ShopData* data = (ShopData*)(event + kOffsetItems);
    const ShopPokemon* entry = GetPokemon(data, GetSelectedIndex(event));
    if (entry == nullptr) {
      HookManager::Call<void>(HookID::kShopPurchaseItem, event);
      return;
    }

    auto& team = savedata::PokemonTeam::GetInstance();
    u32 free_slots = savedata::PokemonTeam::kMaxSlots - team.count;
    if (free_slots == 0) return;

    u32 bought = READ32(event + kOffsetBuyCount);
    if (bought > free_slots) {
      bought = free_slots;
      WRITE32(event + kOffsetBuyCount, bought);
    }

    auto& shop = GetInstance();
    shop.skip_bag_add = true;
    HookManager::Call<void>(HookID::kShopPurchaseItem, event);
    shop.skip_bag_add = false;

    for (u32 i = 0; i < bought; i++) {
      GivePokemon(*entry);
    }
    ui::LogApplication::Print(u"shop: bought %u x %u",
                              static_cast<u32>(entry->species), bought);
  }

  static bool AddItemHook(void* bag, u16 item_id, u16 count, void* heap) {
    if (GetInstance().skip_bag_add) return true;
    return HookManager::Call<bool>(HookID::kBagAddItem, bag, item_id, count,
                                   heap);
  }

  static s32 GetSelectedIndex(uptr event) {
    return READ32(event + kOffsetCursor) + READ32(event + kOffsetDispCursor);
  }
};
} // namespace feature