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

#ifndef SANGO_PLUGIN_POKEMON_TEAM_H
#define SANGO_PLUGIN_POKEMON_TEAM_H
#include "common.h"
#include "savedata.h"
#include "savedata/pokemon_core_data.h"
#include "savedata/pokemon_runtime_data.h"

class PokemonDataAccessor;

namespace savedata {

struct PokemonParam {
  void* vtable;
  PokemonCoreData* core;
  PokemonRuntimeData* runtime;
  PokemonDataAccessor* accessor;
};

struct PokemonTeam {
  static void LoadMenu(menu::PluginMenu& menu, void* args);
  static PokemonTeam& GetInstance() {
    return GameDataManager::GetInstance().GetPokemonTeam();
  }
  static constexpr u32 kMaxSlots = 6;

  PokemonParam* pokemons[kMaxSlots];
  u8 count;
  u8 _0[3];
};

// struct PokemonTeam {
//   static void LoadMenu(menu::PluginMenu& menu, void* args);
//   static PokemonTeam& GetInstance() {
//     return SaveData::GetInstance().GetPokemonTeam();
//   }
//   static constexpr u32 kMaxSlots = 6;
//
//   void* vtable;
//   struct PokemonData {
//     PokemonCoreData core;
//     PokemonRuntimeData runtime;
//   } pokemons[kMaxSlots];
//   u8 count;
//   u8 pokemon_amie_index;
// };

}  // namespace savedata

#endif  // SANGO_PLUGIN_POKEMON_TEAM_H
