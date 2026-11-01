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
 * @file wild_encounter.cc
 * @brief Lets a product change the wild Pokémon.
 *
 * The declarations are in overworld/patch/wild_encounter.h.
 */

#include "overworld/patch/wild_encounter.h"

#include "core/hook.h"
#include "core/native/process_manager.h"
#include "core/problems.h"
#include "overworld/native/encounter_data.h"
#include "overworld/patch/placed_decorations.h"
#include "overworld/native/map_manager.h"
#include "overworld/address.h"
#include "pokemon/address.h"
#include "ui/log.h"

namespace overworld {

namespace {
core::Hook<bool(u32, u32)> get_encounter_pokemon_hook;
core::Hook<u16*(EncounterData*, s32, u32*, void*, u8, bool)>
    get_dex_nav_table_hook;
core::Hook<u32(void*, const void*)> get_encounter_chance_hook;
} // namespace

void WildEncounter::Initialize() {
#ifdef GAME_ORAS
  get_encounter_chance_hook.Install(address::kGetEncounterChance,
                                    GetEncounterChanceHook, address::kVtable);
#endif
  get_encounter_pokemon_hook.Install(pokemon::address::kEncounterSetPokemon,
                                     GetEncounterPokemonHook, address::kVtable);
  get_dex_nav_table_hook.Install(address::kGetDexNavTable, GetDexNavTable);
}

bool WildEncounter::SetTable(MapId map_id, EncounterMethod method,
                             const EncounterSlot* slots, u32 count) {
  auto& self = GetInstance();
  if (slots == nullptr || count == 0) return false;
  for (u32 i = 0; i < self.table_count_; i++) {
    EncounterTable& table = self.tables_[i];
    if (table.map_id == map_id && table.method == method) {
      table.slots = slots;
      table.slot_count = count;
      return true;
    }
  }
  if (self.table_count_ >= kMaxTables) {
    core::Problems::Add(u"map %u: more than %u encounter tables",
                        static_cast<u32>(map_id), kMaxTables);
    return false;
  }
  self.tables_[self.table_count_++] = {map_id, method, slots, count};
  return true;
}

void WildEncounter::ClearTables() { GetInstance().table_count_ = 0; }

void WildEncounter::RemoveTables(
    bool (*is_removed)(const EncounterTable& table)) {
  auto& self = GetInstance();
  u32 kept = 0;
  for (u32 i = 0; i < self.table_count_; i++) {
    if (is_removed(self.tables_[i])) continue;
    self.tables_[kept++] = self.tables_[i];
  }
  self.table_count_ = kept;
}

PokeInfoOnAction* WildEncounter::GetPlaces(EncounterData* data,
                                           EncounterMethod method,
                                           u32* count) {
  switch (method) {
    case EncounterMethod::kWalk:
      *count = SIZE(data->on_walk);
      return data->on_walk;
    case EncounterMethod::kXXX:
      *count = SIZE(data->on_xxx);
      return data->on_xxx;
    case EncounterMethod::kYYY:
      *count = SIZE(data->on_yyy);
      return data->on_yyy;
    case EncounterMethod::kSurf:
      *count = SIZE(data->on_surf);
      return data->on_surf;
    case EncounterMethod::kRockSmash:
      *count = SIZE(data->on_rock_smash);
      return data->on_rock_smash;
    case EncounterMethod::kOldRod:
      *count = SIZE(data->on_old_rod);
      return data->on_old_rod;
    case EncounterMethod::kGoodRod:
      *count = SIZE(data->on_good_rod);
      return data->on_good_rod;
    case EncounterMethod::kSuperRod:
      *count = SIZE(data->on_super_rod);
      return data->on_super_rod;
    case EncounterMethod::kHorde:
      *count = SIZE(data->on_horde);
      return data->on_horde;
    default:
      *count = 0;
      return nullptr;
  }
}

void WildEncounter::ApplyTables(EncounterData* data) {
  auto& self = GetInstance();
  if (self.table_count_ == 0) return;
  const MapId map_id = MapManager::GetInstance().GetMap();
  for (u32 i = 0; i < self.table_count_; i++) {
    const EncounterTable& table = self.tables_[i];
    if (table.map_id != map_id) continue;
    if (data->GetRate(table.method) == 0) {
      // The map has no wild Pokémon for this method: the game never reads
      // the table.
      ui::Log::Print(u"map %u: no wild Pokemon for method %u",
                     static_cast<u32>(map_id),
                     static_cast<u32>(table.method));
      continue;
    }
    u32 place_count = 0;
    PokeInfoOnAction* places = GetPlaces(data, table.method, &place_count);
    for (u32 place = 0; place < place_count; place++) {
      const EncounterSlot& slot = table.slots[place % table.slot_count];
      places[place].species = slot.species;
      places[place].form = slot.form;
      places[place].min_level = slot.min_level;
      places[place].max_level =
          slot.max_level < slot.min_level ? slot.min_level : slot.max_level;
    }
  }
}

u16* WildEncounter::GetDexNavTable(EncounterData* data, s32 data_size,
                                u32* count, void* heap,
                                u8 p4, bool p5) {
  if (data_size <= 0 || data == nullptr) return nullptr;

  // The tables of the product first: the callback can still change them.
  ApplyTables(data);
  auto& feat = GetInstance();
  feat.on_encounter_table.Emit(data);

  return get_dex_nav_table_hook(data, data_size, count, heap, p4, p5);
}

// The chance of an encounter at each step. The game compares a random number
// from 0 to 99 with it: 0 never meets a Pokémon, 100 or more always does.
u32 WildEncounter::GetEncounterChanceHook(void* work, const void* pattern) {
  switch (GetInstance().mode) {
    case Mode::kNever:
      return 0;
    case Mode::kAlways:
      return 10000;
    default:
      return get_encounter_chance_hook(work, pattern);
  }
}

bool WildEncounter::GetEncounterPokemonHook(u32 p0, u32 p1) {
  bool result = get_encounter_pokemon_hook(p0, p1);

  if (!core::ProcessManager::GetInstance().IsCurrentProcess(address::kVtable))
    return result;
  PlacedDecorations::OnWildPokemonRolled((WildPokemon*)p0, READ32(p0 + 108));

  auto& feat = GetInstance();
  if (feat.on_wild_pokemon.IsEmpty()) return result;

  feat.on_wild_pokemon.Emit(MapManager::GetInstance().GetMap(), (WildPokemon*)p0,
                       READ32(p0 + 108));

  return result;
}

} // namespace overworld
