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
 * @file wild_encounter.h
 * @brief Lets a product change the wild Pokémon.
 *
 * @see docs/tutorials/07-change-wild-pokemon.md
 */

#pragma once

#include "common.h"
#include "overworld/constant/encounter_method.h"
#include "overworld/constant/map.h"
#include "overworld/native/wild_pokemon.h"

namespace overworld {
struct EncounterData;
struct PokeInfoOnAction;

/// One wild Pokémon of an encounter table: its species and its levels.
struct EncounterSlot {
  SpeciesId species;
  u8 min_level;
  u8 max_level;
  FormId form;

  EncounterSlot()
    : species(SpeciesId::kNone), min_level(1), max_level(1),
      form(FormId::kNormal) {}
  EncounterSlot(SpeciesId species, u8 min_level, u8 max_level,
                FormId form = FormId::kNormal)
    : species(species), min_level(min_level), max_level(max_level),
      form(form) {}
};

/// The wild Pokémon of a map for one method (tall grass, surf...).
struct EncounterTable {
  MapId map_id;
  EncounterMethod method;
  const EncounterSlot* slots;
  u32 slot_count;
};

/**
 * @brief Changes the wild Pokémon: tables for each map, and callbacks.
 *
 * The simple way: give a table of Pokémon for a map and a method.
 *
 * @code
 * const overworld::EncounterSlot kRoute101[] = {
 *     {SpeciesId::kPikachu, 3, 5},
 *     {SpeciesId::kEevee, 3, 5},
 *     {SpeciesId::kRiolu, 4, 6},
 * };
 * overworld::WildEncounter::SetTable(MapId::kRoute101,
 *                                    EncounterMethod::kWalk, kRoute101);
 * @endcode
 *
 * The game has a fixed number of places in each table (12 in the tall
 * grass, 5 on the water...), and each place has its own chance. The library
 * puts your Pokémon in the places in turn: the first Pokémon goes in the
 * first place, the second in the second place... and it starts again at the
 * start of your list. The game then makes the moves, the ability and the
 * gender of the species, and the DexNav shows your Pokémon.
 */
struct WildEncounter {
  MAKE_SINGLETON(WildEncounter)

  /// The number of tables (one map and one method each).
  static constexpr u32 kMaxTables = 256;

  /**
   * @brief Sets the wild Pokémon of a map for one method.
   *
   * The map must already have wild Pokémon for this method: tall grass for
   * EncounterMethod::kWalk, water for kSurf... A second call for the same
   * map and the same method replaces the table.
   *
   * @param slots The Pokémon. The table must stay in memory: use a global
   *        `const` array.
   * @param count The number of Pokémon in `slots`.
   * @return false when the list of tables is full.
   */
  static bool SetTable(MapId map_id, EncounterMethod method,
                       const EncounterSlot* slots, u32 count);

  /// SetTable() with an array: the library counts the Pokémon.
  template <u32 N>
  static bool SetTable(MapId map_id, EncounterMethod method,
                       const EncounterSlot (&slots)[N]) {
    return SetTable(map_id, method, slots, N);
  }

  /// Removes all the tables of SetTable().
  static void ClearTables();

  /// Called when the game reads the encounter table of a map. Change the
  /// table: the DexNav then shows the new Pokémon.
  typedef void (*EncounterTableCallback)(EncounterData* data);
  /// Called after the game selects the wild Pokémon of a battle. `count` is
  /// 1 for a normal battle and 5 for a horde.
  typedef void (*WildPokemonCallback)(MapId map_id, WildPokemon* pokemons,
                                      u32 count);

  EncounterTableCallback on_encounter_table = nullptr;
  WildPokemonCallback on_wild_pokemon = nullptr;

  /// How often the wild Pokémon appear: normal, never or at each step.
  enum class Mode : u8 { kNormal, kNever, kAlways };
  Mode mode = Mode::kNormal;

  static void Initialize();

private:
  /// Writes the tables of SetTable() for the current map into `data`.
  static void ApplyTables(EncounterData* data);
  /// Returns the places of a method in `data`, and their number.
  static PokeInfoOnAction* GetPlaces(EncounterData* data,
                                     EncounterMethod method, u32* count);
  static u16* GetDexNavTable(EncounterData* data, s32 data_size,
                              u32* count, void* heap,
                              u8 p4, bool p5);
  static u32 GetEncounterChanceHook(void* work, const void* pattern);
  static bool GetEncounterPokemonHook(u32 p0, u32 p1);

  EncounterTable tables_[kMaxTables] = {};
  u32 table_count_ = 0;
};

} // namespace overworld
