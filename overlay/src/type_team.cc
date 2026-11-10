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
 * @file type_team.cc
 * @brief A team to test the new types.
 *
 * Six Pokémon of the game get new types, and some moves of the game get the
 * new types too. The first Pokémon has two new types and four moves of four
 * other new types: one screen shows all of them. The others have a new type and
 * a type of the game. A button of the page "Pokemon"
 * (the top page of the menu) gives the party of the player these six Pokémon,
 * at level 100. ORAS only.
 */

#include "custom_battle.h"
#include "pokemon/edit_pokemon.h"

#include "core/native/process_manager.h"
#include "pokemon/address.h"
#include "pokemon/constant/form.h"
#include "pokemon/native/ability_slot.h"
#include "pokemon/native/move_data.h"
#include "pokemon/native/species_data.h"
#include "savedata/native/item_manager.h"
#include "savedata/native/pokemon_team.h"
#include "ui/main_application.h"

namespace battle {
namespace {
struct TypedMove {
  MoveId move;
  TypeId type;
};

// The moves of the game that get a new type.
constexpr TypedMove kTypedMoves[] = {
    {MoveId::kPowerGem, kTypeCosmic},     {MoveId::kCosmicPower, kTypeCosmic},
    {MoveId::kMeteorMash, kTypeCosmic},   {MoveId::kMoonlight, kTypeCosmic},
    {MoveId::kHyperVoice, kTypeSound},    {MoveId::kBoomburst, kTypeSound},
    {MoveId::kEchoedVoice, kTypeSound},   {MoveId::kScreech, kTypeSound},
    {MoveId::kFlashCannon, kTypeLight},   {MoveId::kSignalBeam, kTypeLight},
    {MoveId::kDazzlingGleam, kTypeLight}, {MoveId::kAuroraBeam, kTypeLight},
    {MoveId::kDischarge, kTypePlasma},    {MoveId::kChargeBeam, kTypePlasma},
    {MoveId::kZapCannon, kTypePlasma},    {MoveId::kElectroBall, kTypePlasma},
    {MoveId::kDreamEater, kTypeDream},    {MoveId::kHypnosis, kTypeDream},
    {MoveId::kNightmare, kTypeDream},     {MoveId::kYawn, kTypeDream},
    {MoveId::kRoarOfTime, kTypeTime},     {MoveId::kFutureSight, kTypeTime},
    {MoveId::kDoomDesire, kTypeTime},
};

// A Pokémon of the team: its types (a new type and a type of the game) and its
// moves.
struct TeamMember {
  SpeciesId species;
  TypeId type_1;
  TypeId type_2;
  MoveId moves[4];
  ItemId item;
};

constexpr TeamMember kTeam[] = {
    // One Pokémon with two new types and four moves of four other new types:
    // all the new types are on one screen (the choice of the move).
    {SpeciesId::kCelebi, kTypeCosmic, kTypeTime,
     {kMovePrismLance, kMoveSonicRequiem, kMoveIonStorm, kMoveDreamfall},
     kItemPlasmaCore},
    // A new type and a type of the game.
    {SpeciesId::kExploud, TypeId::kNormal, kTypeSound,
     {MoveId::kHyperVoice, MoveId::kBoomburst, MoveId::kEchoedVoice,
      MoveId::kScreech},
     kItemDreamPillow},
    {SpeciesId::kLanturn, TypeId::kWater, kTypeLight,
     {MoveId::kFlashCannon, MoveId::kSignalBeam, MoveId::kDazzlingGleam,
      MoveId::kAuroraBeam},
     ItemId::kLeftovers},
    {SpeciesId::kMagnezone, kTypePlasma, TypeId::kSteel,
     {MoveId::kZapCannon, MoveId::kDischarge, MoveId::kElectroBall,
      MoveId::kChargeBeam},
     ItemId::kNone},
    {SpeciesId::kMusharna, TypeId::kPsychic, kTypeDream,
     {MoveId::kDreamEater, MoveId::kHypnosis, MoveId::kNightmare,
      MoveId::kYawn},
     ItemId::kNone},
    {SpeciesId::kDialga, TypeId::kSteel, kTypeTime,
     {MoveId::kRoarOfTime, MoveId::kFutureSight, MoveId::kDoomDesire,
      MoveId::kIronHead},
     ItemId::kNone},
};

#ifdef GAME_ORAS
// Gives a Pokémon of the team to a slot of the party.
void FillSlot(savedata::PokemonParam* param, const TeamMember& member) {
  {
    pokemon::EditPokemon edit(*param, false);
    edit->species = member.species;
    edit->form = FormId::kNormal;
    pokemon::SetAbilitySlot(*edit, 0);
    edit->item = member.item;
    edit->use_nickname = false;
    edit->SetLevel(100);
    edit->SetMaxIVs();
    edit.SetMoves(member.moves[0], member.moves[1], member.moves[2],
                  member.moves[3]);
  }
  param->ResetNickname();
  param->UpdateRuntimeData();
}
#endif
} // namespace

void PatchTypedPokemon() {
#ifdef GAME_ORAS
  for (const TypedMove& entry : kTypedMoves) {
    pokemon::MoveData::GetInstance(entry.move).type = entry.type;
  }
  for (const TeamMember& member : kTeam) {
    auto& data =
        pokemon::SpeciesData::GetInstance(member.species, FormId::kNormal);
    data.type_1 = member.type_1;
    data.type_2 = member.type_2;
  }
#endif
}

// The button of the menu: the party gets the six Pokemon.
void GiveTypeTeamFromMenu(void*) {
  if (!core::ProcessManager::IsOverworldActive() ||
      savedata::PokemonTeam::GetInstance().count == 0) {
    ui::MainApplication::GetInstance().ShowToast(
        "Use it in the overworld, with a Pokemon in the team.");
    return;
  }
  GiveTypeTeam();
  ui::MainApplication::GetInstance().ShowToast(
      "The party has the six Pokemon of the new types.");
}

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

void GiveTypeTeam() {
#ifdef GAME_ORAS
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return;

  // The party has six slots: the game copies the first Pokémon into the new
  // slots, then each slot gets its own Pokémon.
  while (team.count < savedata::PokemonTeam::kMaxSlots) {
    const bool is_added = core::GameFunction<bool(savedata::PokemonTeam*,
                                                  savedata::PokemonParam*)>::Call(
        pokemon::address::kAddPokemonToTeam, &team, team.pokemons[0]);
    if (!is_added) break;
  }
  for (u32 i = 0; i < team.count && i < SIZE(kTeam); i++) {
    FillSlot(team.pokemons[i], kTeam[i]);
  }
  team.HealAllPokemons();
#endif
}

void AddTypeTestEntries(ui::MainApplication& app) {
  app.AddSection("Type Test")
     .Add("Receive The New Type Team", GiveTypeTeamFromMenu)
     .WithConfirm()
     .WithDescription("Replaces the party with six Pokemon at level 100 that "
                      "have the new types (Cosmic, Sound, Light, Plasma, "
                      "Dream, Time) and moves of these types. The first one "
                      "(Celebi) has two new types and four new moves of four "
                      "other new types, and holds a new item (Plasma "
                      "Core). The second one holds Dream Pillow, and the third one holds the "
                      "changed Leftovers. Hold {A}. "
                      "Use it in the overworld.")
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
