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
#include "feature/core/cheat_code_manager.h"
#include "common.h"
#include "feature/overworld/feature_encounter.h"
#include "feature/core/hook_manager.h"
#include "game/constant/map.h"
#include "utils.h"
#include "game/core/process_manager.h"
#include "game/overworld/map_data.h"
#include "game/savedata/pokemon_team.h"
#include "game/savedata/savedata_encounter.h"
#include "game/constant/ability.h"
#include "game/constant/form.h"
#include "game/constant/gender.h"
#include "game/constant/item.h"
#include "game/constant/move.h"
#include "game/constant/species.h"

namespace ui {
class LogApplication;
}

namespace feature {
struct Encounter {
  MAKE_SINGLETON(Encounter)
  struct PokemonData;
  /// Lets a product rewrite an encounter table as the game reads it.
  typedef void (*EncounterTableCallback)(overworld::EncounterData* data);
  /// Lets a product rewrite the Pokémon the game just rolled for a wild
  /// battle on the current map.
  typedef void (*WildPokemonCallback)(MapId map_id, PokemonData* pokemons,
                                      u32 count);
  EncounterTableCallback on_encounter_table = nullptr;
  WildPokemonCallback on_wild_pokemon = nullptr;

  STATIC_INLINE void Initialize() {
    CheatCodeManager::Initialize(CheatCodeId::kNoEncounter, AddMaxRepel,
                                 RemoveMaxRepel,
                                 true);
    HookManager::Initialize(HookID::kGetEncounterPokemon,
                            ADDRESS_ENCOUNTER_SET_POKEMON,
                            (uptr)GetEncounterPokemonHook, false);
    HookManager::Initialize(HookID::kGetNaviDexTable,
                            ADDRESS_GET_NAVI_DEX_TABLE,
                            (uptr)GetNaviDexTable, true);
  }

  static u16* GetNaviDexTable(overworld::EncounterData* data, s32 data_size,
                              u32* count, void* heap,
                              u8 p4, bool p5) {
    if (data_size <= 0 || data == nullptr) return nullptr;

    auto& feat = GetInstance();
    if (feat.on_encounter_table != nullptr) feat.on_encounter_table(data);

    return HookManager::Call<u16*>(HookID::kGetNaviDexTable, data,
                                   data_size, count,
                                   heap, p4, p5);;
  }

  static void AddMaxRepel() {
    savedata::Encounter::GetInstance().spray_count = 100;
  }

  static void RemoveMaxRepel() {
    savedata::Encounter::GetInstance().spray_count = 0;
  }

  struct PokemonData {
    Species species;
    ItemId item;
    u8 level;
    Form form;
    bool is_shiny;
    Ability ability;
    MoveId moves[4];
    Gender gender;
    u8 ivs;
  };

  static bool GetEncounterPokemonHook(u32 p0, u32 p1) {
    bool result = HookManager::Call<bool>(HookID::kGetEncounterPokemon, p0, p1);

    auto& feat = GetInstance();
    if (feat.on_wild_pokemon == nullptr) return result;
    if (!game::ProcessManager::GetInstance().IsCurrentProcess(
        ADDRESS_OVERWORLD_VTABLE))
      return result;
    u32& map_id = overworld::MapManager::GetInstance().GetMapId();
    feat.on_wild_pokemon(static_cast<MapId>(map_id), (PokemonData*)p0,
                         READ32(p0 + 108));

    return result;
  }
};
} // namespace feature