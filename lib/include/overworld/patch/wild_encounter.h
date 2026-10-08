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
#include "overworld/constant/map.h"
#include "overworld/native/wild_pokemon.h"

namespace overworld {
struct EncounterData;

/// Calls the callbacks of the product when the game reads the wild Pokémon.
struct WildEncounter {
  MAKE_SINGLETON(WildEncounter)

  /// Called when the game reads the encounter table of a map. Change the
  /// table: the DexNav then shows the new Pokémon.
  typedef void (*EncounterTableCallback)(EncounterData* data);
  /// Called after the game selects the wild Pokémon of a battle. `count` is
  /// 1 for a normal battle and 5 for a horde.
  typedef void (*WildPokemonCallback)(MapId map_id, WildPokemon* pokemons,
                                      u32 count);

  EncounterTableCallback on_encounter_table = nullptr;
  WildPokemonCallback on_wild_pokemon = nullptr;

  static void Initialize();

private:
  static u16* GetDexNavTable(EncounterData* data, s32 data_size,
                              u32* count, void* heap,
                              u8 p4, bool p5);
  static void AddMaxRepel();
  static void RemoveMaxRepel();
  static bool GetEncounterPokemonHook(u32 p0, u32 p1);
};

} // namespace overworld
