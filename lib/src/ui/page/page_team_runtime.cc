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
 * @file page_team_runtime.cc
 * @brief The page "Team Runtime Data": changes the HP, the stats and the status
 * condition of the Pokémon of the team, refreshes them and heals them.
 *
 * The runtime data is the data that the game calculates from the saved data of a
 * Pokémon: the level, the HP, the stats and the status condition. It is
 * encrypted like the rest of the Pokémon, so the page reads and writes it
 * between a Decrypt and an Encrypt.
 */

#include "pokemon/edit_pokemon.h"
#include "pokemon/native/runtime_data.h"
#include "savedata/native/pokemon_param.h"
#include "savedata/native/pokemon_team.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"

namespace ui {
namespace {

// The status condition of a Pokemon of the party is one number: 1 to 7 is
// a sleep (the number of turns), and one bit for each other condition.
constexpr u32 kStatusValues[] = {0, 3, 8, 16, 32, 64, 128};
const c8* kStatusNames[] = {"None",   "Sleep",     "Poison",       "Burn",
                            "Freeze", "Paralysis", "Badly Poisoned"};

// The values that the page shows. "Read Slot" fills them, "Apply To Slot"
// writes them.
u8 slot = 0;
u16 hp = 0;
u16 max_hp = 0;
u16 attack = 0;
u16 defense = 0;
u16 special_attack = 0;
u16 special_defense = 0;
u16 speed = 0;
u8 status_index = 0;

savedata::PokemonParam* GetSlot() {
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return nullptr;
  if (slot >= team.count) slot = static_cast<u8>(team.count - 1);
  return team.pokemons[slot];
}

// Runs a function on the runtime data of a Pokemon. The game encrypts it with
// the saved data.
template <typename F>
void WithRuntime(savedata::PokemonParam* pokemon, F function) {
  pokemon::EditPokemon edit(*pokemon, false);
  function(*edit.GetRuntimeData());
}

u8 FindStatusIndex(u32 status) {
  for (u32 i = 0; i < SIZE(kStatusValues); i++) {
    if (kStatusValues[i] == status) return static_cast<u8>(i);
  }
  // A sleep of another number of turns, or a combination: the closest name.
  if (status != 0 && status < 8) return 1;
  return 0;
}

void ReadSlot(void*) {
  savedata::PokemonParam* pokemon = GetSlot();
  if (pokemon == nullptr) return;
  WithRuntime(pokemon, [](pokemon::RuntimeData& runtime) {
    hp = runtime.hp;
    max_hp = runtime.max_hp;
    attack = runtime.attack;
    defense = runtime.defense;
    special_attack = runtime.special_attack;
    special_defense = runtime.special_defense;
    speed = runtime.speed;
    status_index = FindStatusIndex(runtime.status_condition);
  });
  MainApplication::GetInstance().Refresh();
}

void ApplyToSlot(void*) {
  savedata::PokemonParam* pokemon = GetSlot();
  if (pokemon == nullptr) return;
  WithRuntime(pokemon, [](pokemon::RuntimeData& runtime) {
    runtime.max_hp = max_hp;
    runtime.hp = hp > max_hp ? max_hp : hp;
    runtime.attack = attack;
    runtime.defense = defense;
    runtime.special_attack = special_attack;
    runtime.special_defense = special_defense;
    runtime.speed = speed;
    runtime.status_condition = kStatusValues[status_index];
  });
  MainApplication::GetInstance().ShowToast("The slot has the new values.");
}

// Calculates the level, the HP maximum and the stats again from the saved data.
// The HP stay the same, between 0 and the new maximum.
void RefreshSlot(void*) {
  savedata::PokemonParam* pokemon = GetSlot();
  if (pokemon == nullptr) return;
  u16 old_hp = 0;
  WithRuntime(pokemon, [&](pokemon::RuntimeData& runtime) { old_hp = runtime.hp; });
  pokemon->UpdateRuntimeData();
  WithRuntime(pokemon, [&](pokemon::RuntimeData& runtime) {
    runtime.hp = old_hp > runtime.max_hp ? runtime.max_hp : old_hp;
  });
  ReadSlot(nullptr);
  MainApplication::GetInstance().ShowToast("The slot is refreshed.");
}

void RefreshTeam(void*) {
  auto& team = savedata::PokemonTeam::GetInstance();
  for (u32 i = 0; i < team.count; i++) {
    savedata::PokemonParam* pokemon = team.pokemons[i];
    u16 old_hp = 0;
    WithRuntime(pokemon,
                [&](pokemon::RuntimeData& runtime) { old_hp = runtime.hp; });
    pokemon->UpdateRuntimeData();
    WithRuntime(pokemon, [&](pokemon::RuntimeData& runtime) {
      runtime.hp = old_hp > runtime.max_hp ? runtime.max_hp : old_hp;
    });
  }
  ReadSlot(nullptr);
  MainApplication::GetInstance().ShowToast("The team is refreshed.");
}

void HealSlot(void*) {
  savedata::PokemonParam* pokemon = GetSlot();
  if (pokemon == nullptr) return;
  WithRuntime(pokemon, [](pokemon::RuntimeData& runtime) {
    runtime.hp = runtime.max_hp;
    runtime.status_condition = 0;
  });
  ReadSlot(nullptr);
  MainApplication::GetInstance().ShowToast("The slot is healed.");
}

void HealTeam(void*) {
  savedata::PokemonTeam::GetInstance().HealAllPokemons();
  ReadSlot(nullptr);
  MainApplication::GetInstance().ShowToast(
      "The team is healed (HP, PP and status).");
}

void SetSlotHp(u16 value) {
  savedata::PokemonParam* pokemon = GetSlot();
  if (pokemon == nullptr) return;
  WithRuntime(pokemon, [value](pokemon::RuntimeData& runtime) {
    runtime.hp = value > runtime.max_hp ? runtime.max_hp : value;
  });
  ReadSlot(nullptr);
}

void CureStatusOfTeam(void*) {
  auto& team = savedata::PokemonTeam::GetInstance();
  for (u32 i = 0; i < team.count; i++) {
    WithRuntime(team.pokemons[i],
                [](pokemon::RuntimeData& runtime) { runtime.status_condition = 0; });
  }
  ReadSlot(nullptr);
  MainApplication::GetInstance().ShowToast("The status conditions are removed.");
}

void SetAllToOneHp(void*) {
  auto& team = savedata::PokemonTeam::GetInstance();
  for (u32 i = 0; i < team.count; i++) {
    WithRuntime(team.pokemons[i], [](pokemon::RuntimeData& runtime) {
      if (runtime.hp > 1) runtime.hp = 1;
    });
  }
  ReadSlot(nullptr);
  MainApplication::GetInstance().ShowToast("All the Pokemon have 1 HP.");
}

} // namespace

void LoadTeamRuntimePage(MainApplication& app, void* args) {
  ReadSlot(nullptr);
  const u32 last = savedata::PokemonTeam::GetInstance().count > 0
                       ? savedata::PokemonTeam::GetInstance().count - 1
                       : 0;

  app.AddSection("Pokemon")
     .Add("Slot (0 = first)", slot)
     .WithBounds(0, last)
     .WithCallback([](void*) { ReadSlot(nullptr); })
     .WithDescription("The Pokemon of the team that the entries below "
                      "change. The values update when you change the slot.")
     .Add("Read Slot", ReadSlot)
     .WithDescription("Reads the values of the slot again.")
     .Add("Apply To Slot", ApplyToSlot)
     .WithDescription("Writes the values below in the slot. A refresh "
                      "calculates the stats again and replaces them.")
     .AddSection("Values")
     .Add("HP", hp)
     .WithBounds(0, 9999)
     .Add("Max HP", max_hp)
     .WithBounds(1, 9999)
     .Add("Attack", attack)
     .WithBounds(1, 9999)
     .Add("Defense", defense)
     .WithBounds(1, 9999)
     .Add("Sp. Atk", special_attack)
     .WithBounds(1, 9999)
     .Add("Sp. Def", special_defense)
     .WithBounds(1, 9999)
     .Add("Speed", speed)
     .WithBounds(1, 9999)
     .Add("Status Condition", status_index)
     .WithArray(kStatusNames, SIZE(kStatusNames))
     .AddSection("Slot")
     .Add("Refresh Slot", RefreshSlot)
     .WithDescription("Calculates the level, the stats and the maximum HP "
                      "again from the saved data. The HP stay the same.")
     .Add("Heal Slot", HealSlot)
     .WithDescription("Full HP and no status condition (not the PP).")
     .Add("Set 1 HP", [](void*) { SetSlotHp(1); })
     .WithDescription("The slot has 1 HP: to test a move that knocks out.")
     .Add("Faint Slot", [](void*) { SetSlotHp(0); })
     .WithConfirm()
     .WithDescription("The slot has 0 HP. Hold {A}.")
     .AddSection("Whole Team")
     .Add("Refresh Team", RefreshTeam)
     .WithDescription("Calculates the stats of all the team again.")
     .Add("Heal Team", HealTeam)
     .WithDescription("Restores the HP and the PP, and removes the status "
                      "conditions of all the team.")
     .Add("Cure All Status Conditions", CureStatusOfTeam)
     .Add("Set All To 1 HP", SetAllToOneHp)
     .WithConfirm()
     .WithDescription("All the Pokemon have 1 HP at most. Hold {A}.");
}

} // namespace ui
