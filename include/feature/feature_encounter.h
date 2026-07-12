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

#define ADDRESS_ENCOUNTER_SET_POKEMON (0x0078DB30)
#define ADDRESS_GET_NAVI_DEX_TABLE (0x004013A4)
#define ADDRESS_GET_ENCOUNTER_CONTACT_ACTION (0x0076E10C)

namespace feature {
struct Encounter {
  MAKE_SINGLETON(Encounter)
  STATIC_INLINE void Initialize() {
    CheatCodeManager::Initialize(CheatCodeId::kNoEncounter, AddMaxRepel,
                                 RemoveMaxRepel,
                                 true);
    HookManager::Initialize(HookID::kEncounterSetPokemon,
                            ADDRESS_ENCOUNTER_SET_POKEMON,
                            (uptr)SetPokemon, false);
    // HookManager::Initialize(HookID::kGetNaviDexTable,
    //                         ADDRESS_GET_NAVI_DEX_TABLE,
    //                         (uptr)GetNaviDexTable, true);
  }

  // static u16* GetNaviDexTable(void* data, u32 data_size, u32* count, void* heap,
  //                             u8 p4, bool p5) {
  //   u16* species_table = HookManager::Call<u16*>(HookID::kGetNaviDexTable, data,
  //                                                data_size, count,
  //                                                heap, p4, p5);
  //   for (u32 i = 0; i < *count; i++) {
  //     species_table[i] = SPECIES_BULBASAUR + i;
  //   }
  //
  //   return species_table;
  // }

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
        SPECIES_CATERPIE,
        SPECIES_WEEDLE,
        SPECIES_MAGIKARP,
        SPECIES_ZUBAT,
        SPECIES_PIDGEY,
        SPECIES_RATTATA,
        SPECIES_SPEAROW,
        SPECIES_DIGLETT,
        SPECIES_JIGGLYPUFF,
        SPECIES_NIDORAN_M,
        SPECIES_NIDORAN_F,
    };

    static const u16 ROUTE_102[] = {
        SPECIES_HORSEA,
        SPECIES_VULPIX,
        SPECIES_MEOWTH,
        SPECIES_PARAS,
        SPECIES_DITTO,
        SPECIES_EKANS,
        SPECIES_SANDSHREW,
        SPECIES_POLIWAG,
        SPECIES_BELLSPROUT,
        SPECIES_GEODUDE,
        SPECIES_DRATINI,
    };
    static const u16 ROUTE_103[] = {
        SPECIES_VENONAT,
        SPECIES_MANKEY,
        SPECIES_MACHOP,
        SPECIES_SHELLDER,
        SPECIES_EEVEE,
        SPECIES_MAGNEMITE,
        SPECIES_SEEL,
        SPECIES_GRIMER,
        SPECIES_EXEGGCUTE,
        SPECIES_KRABBY,
    };

    static const u16 ROUTE_104_SOUTH[] = {
        SPECIES_DROWZEE,
        SPECIES_GOLDEEN,
        SPECIES_ODDISH,
        SPECIES_PIKACHU,
        SPECIES_PSYDUCK,
        SPECIES_CUBONE,
        SPECIES_SLOWPOKE,
        SPECIES_ABRA,
        SPECIES_DODUO,
        SPECIES_GASTLY,
    };

    static const u16 PETALBURG_WOODS[] = {
        SPECIES_KOFFING,
        SPECIES_STARYU,
        SPECIES_RHYHORN,
        SPECIES_GROWLITHE,
        SPECIES_FARFETCHD,
        SPECIES_OMANYTE,
        SPECIES_KABUTO,
        SPECIES_KANGASKHAN,
        SPECIES_TANGELA,
        SPECIES_WIGGLYTUFF,
    };

    static const u16 ROUTE_104_NORTH[] = {
        SPECIES_PONYTA,
        SPECIES_CHANSEY,
        SPECIES_GOLBAT,
        SPECIES_HITMONLEE,
        SPECIES_HITMONCHAN,
        SPECIES_JYNX,
        SPECIES_MR_MIME,
        SPECIES_SCYTHER,
        SPECIES_ELECTABUZZ,
        SPECIES_TAUROS,
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

      case MAP_ROUTE_104_SOUTH:
        species = ROUTE_104_SOUTH[Utils::GetRandomValue(SIZE(ROUTE_104_SOUTH))];
        break;

      case MAP_PETALBURG_WOODS:
        species = PETALBURG_WOODS[Utils::GetRandomValue(SIZE(PETALBURG_WOODS))];
        break;

      case MAP_ROUTE_104_NORTH:
        species = ROUTE_104_NORTH[Utils::GetRandomValue(SIZE(ROUTE_104_NORTH))];
        break;

      default:
        species = default_species;
        break;
    }

    return species;
  }
};
} // namespace feature