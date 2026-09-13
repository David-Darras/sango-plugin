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
#include "core/utils.h"
#include "overworld/constant/map.h"
#include "pokemon/constant/species.h"
#include "overworld/native/map_data.h"
#include "kaizo/kaizo.h"
#include "overworld/native/map_manager.h"
#include "savedata/native/pokemon_team.h"
#include "ui/log_application.h"

namespace kaizo {
static const SpeciesId ROUTE_101[] = {
    SpeciesId::kGrowlithe,
    SpeciesId::kPoochyena,
    SpeciesId::kRiolu,
    SpeciesId::kFurfrou,
    SpeciesId::kHoundour,
    SpeciesId::kSnubbull,
    SpeciesId::kElectrike,
    SpeciesId::kLillipup,
};

static const SpeciesId ROUTE_102[] = {
    SpeciesId::kMeowth,
    SpeciesId::kEspurr,
    SpeciesId::kPurrloin,
    SpeciesId::kGlameow,
    SpeciesId::kSkitty,
    SpeciesId::kShinx,
    SpeciesId::kSneasel,
    SpeciesId::kEevee,
};

static const SpeciesId ROUTE_103[] = {
    SpeciesId::kDedenne,
    SpeciesId::kRattata,
    SpeciesId::kPichu,
    SpeciesId::kAzurill,
    SpeciesId::kPlusle,
    SpeciesId::kMinun,
};

static const SpeciesId ROUTE_104_SOUTH[] = {
    SpeciesId::kPidgey,
    SpeciesId::kNatu,
    SpeciesId::kTaillow,
    SpeciesId::kWingull,
    SpeciesId::kStarly,
    SpeciesId::kPidove,
    SpeciesId::kVullaby,
    SpeciesId::kFletchling,
};

static const SpeciesId PETALBURG_WOODS[] = {
    SpeciesId::kCaterpie,
    SpeciesId::kWeedle,
    SpeciesId::kWurmple,
    SpeciesId::kBurmy,
    SpeciesId::kSewaddle,
    SpeciesId::kVenipede,
    SpeciesId::kScatterbug,
};

static const SpeciesId ROUTE_104_NORTH[] = {
    SpeciesId::kSpearow,
    SpeciesId::kFarfetchd,
    SpeciesId::kHoothoot,
    SpeciesId::kMurkrow,
    SpeciesId::kSkarmory,
    SpeciesId::kChatot,
    SpeciesId::kDucklett,
    SpeciesId::kRufflet,
    SpeciesId::kHawlucha
};

static const SpeciesId PETALBURG_CITY[] = {
    SpeciesId::kUnown
};

static const SpeciesId ROUTE_116[] = {
    SpeciesId::kMankey,
    SpeciesId::kAipom,
    SpeciesId::kSlakoth,
    SpeciesId::kChimchar,
    SpeciesId::kPansage,
    SpeciesId::kPansear,
    SpeciesId::kPanpour,
    SpeciesId::kDarumaka,
};

static const SpeciesId RUSTURF_TUNNEL[] = {
    SpeciesId::kZubat,
    SpeciesId::kGligar,
    SpeciesId::kWoobat,
    SpeciesId::kNoibat
};

static const EncounterEntry s_table[] = {
    {MapId::kRoute101, SIZE(ROUTE_101), ROUTE_101},
    {MapId::kRoute102, SIZE(ROUTE_102), ROUTE_102},
    {MapId::kRoute103, SIZE(ROUTE_103), ROUTE_103},
    {MapId::kPetalburgCity, SIZE(PETALBURG_CITY), PETALBURG_CITY},
    {MapId::kRoute104South, SIZE(ROUTE_104_SOUTH), ROUTE_104_SOUTH},
    {MapId::kPetalburgWoods, SIZE(PETALBURG_WOODS), PETALBURG_WOODS},
    {MapId::kRoute104North, SIZE(ROUTE_104_NORTH), ROUTE_104_NORTH},
};
static const u32 s_table_size = SIZE(s_table);


const EncounterEntry* GetEncounterEntry(MapId map_id) {
  for (u16 i = 0; i < s_table_size; ++i) {
    if (s_table[i].map_id == map_id) {
      return &s_table[i];
    }
  }
  return nullptr;
}

void PatchEncounterTable(overworld::EncounterData* data) {
  const EncounterEntry* entry =
      GetEncounterEntry(overworld::MapManager::GetInstance().GetMap());
  if (entry == nullptr) return;

  u32 offset = 0;
  for (overworld::PokeInfoOnAction& info : data->poke_info) {
    if (info.species == SpeciesId::kNone) continue;
    info.species = entry->species[offset];
    info.form = Form::kNormal;
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
  s32 level = static_cast<s32>(mean) - spread + core::Utils::GetRandomValue(
                  2 * spread + 1);

  level = level < 1 ? 1 : (level > 100 ? 100 : level);
  return static_cast<u8>(level);
}
}