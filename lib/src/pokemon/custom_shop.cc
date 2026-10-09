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
 * @file custom_shop.cc
 * @brief Changes the items of the shops, and sells Pokémon in the shops.
 *
 * The declarations are in pokemon/patch/custom_shop.h.
 */

#include "pokemon/patch/custom_shop.h"
#include "core/cheat_code_manager.h"
#include "core/hook.h"
#include "core/utils.h"
#include "overworld/patch/gift_pokemon.h"
#include "pokemon/native/gift_pokemon_data.h"
#include "pokemon/native/shop_data.h"
#include "savedata/native/pokemon_team.h"
#include "ui/log_application.h"
#include "ui/native/app_layout_manager.h"

namespace pokemon {

namespace {
core::Hook<void(ShopData*, ShopType, u32, void*, bool)> load_shop_items_hook;
core::Hook<void(ShopData*)> unload_shop_items_hook;
core::Hook<String*(ShopData*, s32)> get_item_name_hook;
core::Hook<String*(ShopData*, s32)> get_item_info_hook;
core::Hook<void(uptr)> disp_item_info_hook;
core::Hook<void(uptr)> purchase_item_hook;
core::Hook<bool(void*, ItemId, u16, void*)> add_item_hook;
} // namespace

void CustomShop::Initialize() {
  core::CheatCodeManager::Initialize(
      CheatCodeId::kPokemonShop,
      []() { GetInstance().is_pokemon_shop_enabled_ = true; },
      []() { GetInstance().is_pokemon_shop_enabled_ = false; }, false);
  load_shop_items_hook.Install(core::address::kLoadShopItems,
                               LoadShopItemsHook);
  unload_shop_items_hook.Install(core::address::kUnloadShopItems,
                                 UnloadShopItemsHook);
  get_item_name_hook.Install(core::address::kShopGetItemName, GetItemNameHook);
  get_item_info_hook.Install(core::address::kShopGetItemInfo, GetItemInfoHook);
  disp_item_info_hook.Install(core::address::kShopDisplayItemInfo,
                              DispItemInfoHook);
  purchase_item_hook.Install(core::address::kShopPurchaseItem,
                             PurchaseItemHook);
  add_item_hook.Install(core::address::kBagAddItem, AddItemHook);
}

void CustomShop::EnablePokemonShop(bool is_enabled) {
  core::CheatCode* code =
      core::CheatCodeManager::GetInstance().Get(CheatCodeId::kPokemonShop);
  if (code != nullptr && code->IsEnabled() != is_enabled) code->Toggle();
  GetInstance().is_pokemon_shop_enabled_ = is_enabled;
}

void CustomShop::SetItems(const ShopItem* items, u32 count, u32 shop_id) {
  auto& shop = GetInstance();
  shop.items = items;
  shop.count = count;
  shop.items_shop_id = shop_id;
}

void CustomShop::SetPokemons(const ShopPokemon* pokemons, u32 count, u32 shop_id) {
  auto& shop = GetInstance();
  shop.pokemons = pokemons;
  shop.pokemon_count = count;
  shop.pokemon_shop_id = shop_id;
}

bool CustomShop::PatchItemName(ItemId item, String* output) {
  if (item != kIcon) return false;
  const ShopPokemon* entry = GetSelectedPokemon();
  if (entry == nullptr) return false;
  GetSpeciesName(entry->species, output);
  return true;
}

void CustomShop::GetSpeciesName(SpeciesId species, String* output) {
  ((void(*)(String*, SpeciesId))address::kGetSpeciesName)(output, species);
}

bool CustomShop::IsPokemonShop(const ShopData* data) {
  return data != nullptr && data == GetInstance().pokemon_data;
}

const ShopPokemon* CustomShop::GetPokemon(const ShopData* data, s32 index) {
  auto& shop = GetInstance();
  if (!IsPokemonShop(data)) return nullptr;
  if (index < 0 || (u32)index >= shop.pokemon_count) return nullptr;
  return &shop.pokemons[index];
}

const ShopPokemon* CustomShop::GetSelectedPokemon() {
  auto& shop = GetInstance();
  if (shop.pokemon_data == nullptr) return nullptr;
  uptr event = (uptr)shop.pokemon_data - kOffsetItems;
  s32 index = READ32(event + kOffsetCursor) +
              READ32(event + kOffsetDispCursor);
  return GetPokemon(shop.pokemon_data, index);
}

bool CustomShop::GivePokemon(const ShopPokemon& entry) {
  using Gift = GiftPokemonData;

  auto& param = Gift::GetInstance(kParamSlot);
  const Gift backup = param;

  param.species = entry.species;
  param.form = entry.form;
  param.level = entry.level;
  param.shiny = ShinyRoll::kRandom;
  param.ability_slot = Gift::kRandomAbility;
  param.nature = Gift::kRandomNature;
  param.item = Gift::kRandomItem;
  param.gender = GenderRoll::kRandom;
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

bool CustomShop::GiveMega(SpeciesId species, ItemId item) {
  using Gift = GiftPokemonData;

  auto& param = Gift::GetInstance(kParamSlot);
  const Gift backup = param;

  param.species = species;
  param.form = FormId::kNormal;
  param.level = 100;
  param.shiny = ShinyRoll::kNotShiny;
  param.ability_slot = Gift::kRandomAbility;
  param.nature = Gift::kRandomNature;
  param.item = static_cast<s32>(item);
  param.gender = GenderRoll::kRandom;
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

bool CustomShop::AddPokemonToTeam(u32 slot) {
  s32 args[3] = {2 * (s32)sizeof(s32), (s32)slot, 0};

  return overworld::GiftPokemon::AddPokemonWithoutRandomizer(
             nullptr, (u32*)args) != 0;
}

void CustomShop::LoadShopItemsHook(ShopData* data, ShopType type, u32 id,
                                   void* heap, bool for_sale) {
  load_shop_items_hook(data, type, id, heap, for_sale);

  auto& shop = GetInstance();
  shop.pokemon_data = nullptr;
  if (shop.log_shop_ids) {
    ui::LogApplication::Print(u"shop: type=%u id=%u", type, id);
  }

  if (shop.is_pokemon_shop_enabled_ && shop.pokemons != nullptr &&
      shop.pokemon_count > 0 &&
      type == ShopType::kNormal &&
      (shop.pokemon_shop_id == kAnyShop || shop.pokemon_shop_id == id)) {
    u32 count = shop.pokemon_count < ShopData::kMaxItems
                  ? shop.pokemon_count
                  : ShopData::kMaxItems;
    for (u32 i = 0; i < count; i++) {
      data->items[i].id = kIcon;
      data->items[i].price = shop.pokemons[i].price;
    }
    data->count = count;
    shop.pokemon_data = data;
    return;
  }

  if (shop.items == nullptr) return;
  if (shop.items_shop_id != kAnyShop && shop.items_shop_id != id) return;

  const u32 count =
      shop.count < ShopData::kMaxItems ? shop.count : ShopData::kMaxItems;
  for (u32 i = 0; i < count; i++) {
    data->items[i].id = shop.items[i].id;
    data->items[i].price = shop.items[i].price;
  }
  data->count = count;
}

void CustomShop::UnloadShopItemsHook(ShopData* data) {
  auto& shop = GetInstance();
  if (shop.pokemon_data == data) shop.pokemon_data = nullptr;
  unload_shop_items_hook(data);
}

String* CustomShop::GetItemNameHook(ShopData* data, s32 index) {
  const ShopPokemon* entry = GetPokemon(data, index);
  if (entry == nullptr) {
    return get_item_name_hook(data, index);
  }
  GetSpeciesName(entry->species, data->name_buffer);
  return data->name_buffer;
}

String* CustomShop::GetItemInfoHook(ShopData* data, s32 index) {
  const ShopPokemon* entry = GetPokemon(data, index);
  if (entry == nullptr) {
    return get_item_info_hook(data, index);
  }
  c16 description[128];
  GetSpeciesName(entry->species, data->info_buffer);
  core::Utils::Format(description, u"%ls / Lv. %u", data->info_buffer->GetBuffer(),
                entry->level);
  data->info_buffer->Set(description);
  return data->info_buffer;
}

void CustomShop::DispItemInfoHook(uptr event) {
  disp_item_info_hook(event);

  if (!IsPokemonShop((ShopData*)(event + kOffsetItems))) return;
  auto* layout = (ui::AppLayoutManager*)READ32(event + kOffsetLayout);
  if (layout != nullptr) layout->HidePane(0, kIconPane);
}

void CustomShop::PurchaseItemHook(uptr event) {
  ShopData* data = (ShopData*)(event + kOffsetItems);
  const ShopPokemon* entry = GetPokemon(data, GetSelectedIndex(event));
  if (entry == nullptr) {
    purchase_item_hook(event);
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
  purchase_item_hook(event);
  shop.skip_bag_add = false;

  for (u32 i = 0; i < bought; i++) {
    GivePokemon(*entry);
  }
}

bool CustomShop::AddItemHook(void* bag, ItemId item, u16 count, void* heap) {
  if (GetInstance().skip_bag_add) return true;
  return add_item_hook(bag, item, count, heap);
}

s32 CustomShop::GetSelectedIndex(uptr event) {
  return READ32(event + kOffsetCursor) + READ32(event + kOffsetDispCursor);
}

} // namespace pokemon
