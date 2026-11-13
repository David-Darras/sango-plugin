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
 * @file item_test.cc
 * @brief Buttons to test the new items of the overlay.
 *
 * The section "Item Test" at the end of the page "Pokemon" (the top page of the
 * menu) puts the new items in the Bag, or gives one to the first Pokémon of the
 * party. ORAS only. It is in the plugin only with SANGO_TEST.
 */

#include "custom_battle.h"
#include "pokemon/edit_pokemon.h"

#include "core/native/process_manager.h"
#include "pokemon/native/move_data.h"
#include "savedata/native/item_manager.h"
#include "savedata/native/pokemon_team.h"
#include "ui/main_application.h"

namespace battle {
namespace {
// Gives an item to the first Pokémon of the party.
void GiveItemToFirstPokemon(ItemId item, const char* message) {
#ifdef GAME_ORAS
  auto& team = savedata::PokemonTeam::GetInstance();
  if (!core::ProcessManager::IsOverworldActive() || team.count == 0) {
    ui::MainApplication::GetInstance().ShowToast(
        "Use it in the overworld, with a Pokemon in the team.");
    return;
  }
  pokemon::Edit(*team.pokemons[0],
                [item](pokemon::CoreData& core) { core.item = item; });
  ui::MainApplication::GetInstance().ShowToast(message);
#endif
}

// Puts an item in the first place of the pocket "Items" of the Bag. An item that
// is already in the pocket moves to the first place and keeps its quantity.
void PutItemFirstInBag(ItemId item, u16 count) {
#ifdef GAME_ORAS
  savedata::ItemManager::ItemSlot* slots =
      savedata::ItemManager::GetInstance().GetNormalItems();
  constexpr u32 kSlotCount = savedata::ItemManager::kMaxNormalItems;

  // The place that the item leaves: its own place, else the first empty place,
  // else the last place.
  u32 place = kSlotCount - 1;
  u16 quantity = count;
  for (u32 i = 0; i < kSlotCount; i++) {
    if (slots[i].id == item) {
      place = i;
      quantity = static_cast<u16>(slots[i].count + count);
      break;
    }
    if (slots[i].id == ItemId::kNone) {
      place = i;
      break;
    }
  }
  for (u32 i = place; i > 0; i--) slots[i] = slots[i - 1];
  slots[0].id = item;
  slots[0].count = quantity;
#endif
}

// Empties the pockets Items, Medicine, TMs and HMs, and Berries. The key items
// stay: the game needs some of them (the Bike, the Mega Ring...).
void EmptyBag() {
#ifdef GAME_ORAS
  auto& bag = savedata::ItemManager::GetInstance();
  const auto empty = [](savedata::ItemManager::ItemSlot* slots, u32 count) {
    for (u32 i = 0; i < count; i++) {
      slots[i].id = ItemId::kNone;
      slots[i].count = 0;
    }
  };
  empty(bag.GetNormalItems(), savedata::ItemManager::kMaxNormalItems);
  empty(bag.GetMedicine(), savedata::ItemManager::kMaxMedicine);
  empty(bag.GetTMsHMs(), savedata::ItemManager::kMaxTMsHMs);
  empty(bag.GetBerries(), savedata::ItemManager::kMaxBerries);
#endif
}

void PutItemsInBagFromMenu(void*) {
  EmptyBag();
  PutItemFirstInBag(kItemPhilosopherStone, 99);
  PutItemFirstInBag(kItemEvolutionPrism, 99);
  PutItemFirstInBag(kItemDevolutionStone, 99);
  PutItemFirstInBag(kItemAbilitySwitch, 99);
  PutItemFirstInBag(kItemKnife, 99);
  PutItemFirstInBag(kItemShinySwitch, 99);
  PutItemFirstInBag(kItemDreamPillow, 99);
  PutItemFirstInBag(kItemPlasmaCore, 99);
  ui::MainApplication::GetInstance().ShowToast(
      "The Bag is empty, with the 8 new items.");
}

void GivePlasmaCoreFromMenu(void*) {
  GiveItemToFirstPokemon(kItemPlasmaCore,
                         "The first Pokemon holds the Plasma Core.");
}

void GiveDreamPillowFromMenu(void*) {
  GiveItemToFirstPokemon(kItemDreamPillow,
                         "The first Pokemon holds the Dream Pillow.");
}

void GivePhilosopherStoneFromMenu(void*) {
  GiveItemToFirstPokemon(kItemPhilosopherStone,
                         "The first Pokemon holds the Philosopher Stone.");
}

} // namespace

void AddItemTestEntries(ui::MainApplication& app) {
  app.AddSection("Item Test")
     .Add("Reset The Bag With The New Items", PutItemsInBagFromMenu)
     .WithDescription("Empties the Bag (the key items stay) and puts "
                      "Plasma Core, Dream Pillow, Shiny Switch (makes a "
                      "Pokemon shiny), Ability Switch (goes to the next "
                      "ability), Evolution Prism (makes a Pokemon evolve), Devolution "
                      "Stone (goes back to the earlier species), "
                      "Knife (cuts a Pokemon) and Philosopher Stone in the "
                      "pocket Items. Use them on a Pokemon in the Bag.")
     .Add("Give Plasma Core", GivePlasmaCoreFromMenu)
     .WithDescription("The first Pokemon of the party holds the new item "
                      "Plasma Core: +1 Sp. Atk when it enters a battle. Use "
                      "it in the overworld.")
     .Add("Give Dream Pillow", GiveDreamPillowFromMenu)
     .WithDescription("The first Pokemon of the party holds the new item "
                      "Dream Pillow: it recovers 1/16 of its HP at the end "
                      "of each turn. Use it in the overworld.")
     .Add("Give Philosopher Stone", GivePhilosopherStoneFromMenu)
     .WithDescription("The first Pokemon of the party holds the new item "
                      "Philosopher Stone: no move hurts it or changes its "
                      "status, and it cannot die. Use it in the overworld.");
}

} // namespace battle
