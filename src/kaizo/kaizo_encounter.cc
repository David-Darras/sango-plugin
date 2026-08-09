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

#include <cmath>

#include "common.h"
#include "utils.h"
#include "game/constant/map.h"
#include "game/constant/species.h"
#include "game/overworld/map_data.h"
#include "kaizo.h"
#include "game/overworld/map_manager.h"
#include "game/savedata/pokemon_team.h"
#include "ui/log_application.h"

namespace kaizo {
static const u16 ROUTE_101[] = {
    SPECIES_GROWLITHE,
    SPECIES_POOCHYENA,
    SPECIES_RIOLU,
    SPECIES_FURFROU,
    SPECIES_HOUNDOUR,
    SPECIES_SNUBBULL,
    SPECIES_ELECTRIKE,
    SPECIES_LILLIPUP,
};

static const u16 ROUTE_102[] = {
    SPECIES_MEOWTH,
    SPECIES_ESPURR,
    SPECIES_PURRLOIN,
    SPECIES_GLAMEOW,
    SPECIES_SKITTY,
    SPECIES_SHINX,
    SPECIES_SNEASEL,
    SPECIES_EEVEE,
};

static const u16 ROUTE_103[] = {
    SPECIES_DEDENNE,
    SPECIES_RATTATA,
    SPECIES_PICHU,
    SPECIES_AZURILL,
    SPECIES_PLUSLE,
    SPECIES_MINUN,
};

static const u16 ROUTE_104_SOUTH[] = {
    SPECIES_PIDGEY,
    SPECIES_NATU,
    SPECIES_TAILLOW,
    SPECIES_WINGULL,
    SPECIES_STARLY,
    SPECIES_PIDOVE,
    SPECIES_VULLABY,
    SPECIES_FLETCHLING,
};

static const u16 PETALBURG_WOODS[] = {
    SPECIES_CATERPIE,
    SPECIES_WEEDLE,
    SPECIES_WURMPLE,
    SPECIES_BURMY,
    SPECIES_SEWADDLE,
    SPECIES_VENIPEDE,
    SPECIES_SCATTERBUG,
};

static const u16 ROUTE_104_NORTH[] = {
    SPECIES_SPEAROW,
    SPECIES_FARFETCHD,
    SPECIES_HOOTHOOT,
    SPECIES_MURKROW,
    SPECIES_SKARMORY,
    SPECIES_CHATOT,
    SPECIES_DUCKLETT,
    SPECIES_RUFFLET,
    SPECIES_HAWLUCHA
};

static const u16 PETALBURG_CITY[] = {
    SPECIES_UNOWN
};

static const EncounterEntry s_table[] = {
    {MAP_ROUTE_101, SIZE(ROUTE_101), ROUTE_101},
    {MAP_ROUTE_102, SIZE(ROUTE_102), ROUTE_102},
    {MAP_ROUTE_103, SIZE(ROUTE_103), ROUTE_103},
    {MAP_PETALBURG_CITY, SIZE(PETALBURG_CITY), PETALBURG_CITY},
    {MAP_ROUTE_104_SOUTH, SIZE(ROUTE_104_SOUTH), ROUTE_104_SOUTH},
    {MAP_PETALBURG_WOODS, SIZE(PETALBURG_WOODS), PETALBURG_WOODS},
    {MAP_ROUTE_104_NORTH, SIZE(ROUTE_104_NORTH), ROUTE_104_NORTH},
};
static const u32 s_table_size = SIZE(s_table);


const EncounterEntry* GetEncounterEntry(u16 map_id) {
  for (u16 i = 0; i < s_table_size; ++i) {
    if (s_table[i].map_id == map_id) {
      return &s_table[i];
    }
  }
  return nullptr;
}

void PatchEncounterTable(overworld::EncounterData* data) {
  auto& map_id = overworld::MapManager::GetInstance().GetMapId();
  const EncounterEntry* entry = GetEncounterEntry(map_id);
  if (entry == nullptr) return;

  u32 offset = 0;
  for (u16 i = 0; i < 61; ++i) {
    if (data->poke_info[i].species == 0) continue;
    data->poke_info[i].species = entry->species[offset];
    data->poke_info[i].form = 0;
    offset = (offset + 1) % 5;
  }
}

u8 GetEncounterLevel() {
  auto& team = savedata::PokemonTeam::GetInstance();
  if (team.count == 0) return 1;

  f32 sum = 0.0f;
  f32 sumSq = 0.0f;
  for (u32 i = 0; i < team.count; i++) {
    auto& pkm = *team.pokemons[i];
    pkm.accessor->Decrypt();
    f32 lvl = (f32)pkm.runtime->level;
    sum += lvl;
    sumSq += lvl * lvl;
    pkm.accessor->Encrypt();
  }

  f32 mean = sum / team.count;
  f32 variance = (sumSq / team.count) - (mean * mean);
  f32 sd = sqrtf(variance > 0.0f ? variance : 0.0f);

  sd = sd < 1.0f ? 1.0f : sd;

  s32 spread = static_cast<s32>(sd);
  s32 level = static_cast<s32>(mean) - spread + Utils::GetRandomValue(
                  2 * spread + 1);

  level = level < 1 ? 1 : (level > 100 ? 100 : level);
  return static_cast<u8>(level);
}
}