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
#include "utils.h"
#include "game/constant/map.h"
#include "game/constant/species.h"
#include "game/overworld/map_manager.h"
#include "game/savedata/savedata_encounter.h"
#include "game/savedata/event_table.h"

#define ADDRESS_ENCOUNTER_SET_POKEMON (0x0078DB30)

namespace feature {
struct EncounterCheatCode {
  MAKE_SINGLETON(EncounterCheatCode)
  STATIC_INLINE void Initialize() {
    CheatCodeManager::Initialize(CheatCodeId::kNoEncounter, AddMaxRepel,
                                 RemoveMaxRepel,
                                 true);
    HookManager::Initialize(HookID::kEncounterSetPokemon,
                            ADDRESS_ENCOUNTER_SET_POKEMON,
                            (uptr)SetPokemon, false);
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

  static bool SetPokemon(u32 p0, u32 p1) {
    bool result = HookManager::Call<bool>(HookID::kEncounterSetPokemon, p0, p1);

    u32 count = READ(u32, p0 + 108);
    for (u32 i = 0; i < count; i++) {
      PokemonData& data = READ(PokemonData, p0 + i * sizeof(PokemonData));
      data.species = GetSpecies(data.species);
    }

    return result;
  }

  static u16 GetSpecies(u16 default_species) {
    static const u16 ROUTE_101[] = {
        SPECIES_CATERPIE, SPECIES_WEEDLE, SPECIES_PIDGEY, SPECIES_RATTATA,
        SPECIES_SPEAROW, SPECIES_ABRA, SPECIES_MACHOP, SPECIES_BELLSPROUT,
        SPECIES_TENTACOOL, SPECIES_GEODUDE, SPECIES_PONYTA
    };

    static const u16 ROUTE_102[] = {
        SPECIES_EKANS, SPECIES_PIKACHU, SPECIES_SANDSHREW, SPECIES_NIDORAN_F,
        SPECIES_NIDORAN_M, SPECIES_CLEFAIRY, SPECIES_VULPIX, SPECIES_JIGGLYPUFF,
        SPECIES_SLOWPOKE, SPECIES_MAGNEMITE, SPECIES_FARFETCHD, SPECIES_DODUO,
        SPECIES_SEEL, SPECIES_GRIMER, SPECIES_SHELLDER, SPECIES_GASTLY
    };

    static const u16 ROUTE_103[] = {
        SPECIES_ZUBAT, SPECIES_ODDISH, SPECIES_PARAS, SPECIES_VENONAT,
        SPECIES_DIGLETT,
        SPECIES_MEOWTH, SPECIES_PSYDUCK, SPECIES_MANKEY, SPECIES_GROWLITHE,
        SPECIES_POLIWAG
    };

    u16 species = 0;
    u32& map_id = overworld::MapManager::GetInstance().GetMapId();

    switch (map_id) {
      case MAP_ROUTE_101:
        species = ROUTE_101[Utils::GetRandomValue(SIZE(ROUTE_101))];
        break;
      case MAP_ROUTE_102:
        species = ROUTE_102[Utils::GetRandomValue(SIZE(ROUTE_102))];
        break;
      case MAP_ROUTE_103:
        species = ROUTE_103[Utils::GetRandomValue(SIZE(ROUTE_103))];
        break;
    }

    if (species == 0) {
      species = default_species;
    }

    return species;
  }
};
} // namespace feature