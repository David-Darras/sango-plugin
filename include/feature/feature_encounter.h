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
#include "cheat_code_manager.h"
#include "common.h"
#include "feature_encounter.h"
#include "hook_manager.h"
#include "kaizo.h"
#include "utils.h"
#include "game/overworld/map_data.h"
#include "game/savedata/pokemon_team.h"
#include "game/savedata/savedata_encounter.h"

namespace ui {
class LogApplication;
}

namespace feature {
struct Encounter {
  MAKE_SINGLETON(Encounter)
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

#ifdef KAIZO
    kaizo::PatchEncounterTable(data);
#endif

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
    u16 species;
    u16 item;
    u8 level;
    u8 form;
    bool is_shiny;
    u8 ability;
    u16 moves[4];
    u8 gender;
    u8 ivs;
  };

  static bool GetEncounterPokemonHook(u32 p0, u32 p1) {
    bool result = HookManager::Call<bool>(HookID::kGetEncounterPokemon, p0, p1);

#ifdef KAIZO
    u32& map_id = overworld::MapManager::GetInstance().GetMapId();
    const kaizo::EncounterEntry* entry = kaizo::GetEncounterEntry(map_id);
    if (entry == nullptr) {
      return result;
    }

    u32 count = READ(u32, p0 + 108);
    for (u32 i = 0; i < count; i++) {
      PokemonData& data = READ(PokemonData, p0 + i * sizeof(PokemonData));
      data.level = kaizo::GetEncounterLevel();
      data.species = entry->species[Utils::GetRandomValue(entry->size)];
    }
#endif

    return result;
  }
};
} // namespace feature