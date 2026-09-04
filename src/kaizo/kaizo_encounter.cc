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
#include "kaizo/kaizo.h"
#include "game/overworld/map_manager.h"
#include "game/savedata/pokemon_team.h"
#include "ui/log_application.h"

namespace kaizo {
static const Species ROUTE_101[] = {
    Species::kGrowlithe,
    Species::kPoochyena,
    Species::kRiolu,
    Species::kFurfrou,
    Species::kHoundour,
    Species::kSnubbull,
    Species::kElectrike,
    Species::kLillipup,
};

static const Species ROUTE_102[] = {
    Species::kMeowth,
    Species::kEspurr,
    Species::kPurrloin,
    Species::kGlameow,
    Species::kSkitty,
    Species::kShinx,
    Species::kSneasel,
    Species::kEevee,
};

static const Species ROUTE_103[] = {
    Species::kDedenne,
    Species::kRattata,
    Species::kPichu,
    Species::kAzurill,
    Species::kPlusle,
    Species::kMinun,
};

static const Species ROUTE_104_SOUTH[] = {
    Species::kPidgey,
    Species::kNatu,
    Species::kTaillow,
    Species::kWingull,
    Species::kStarly,
    Species::kPidove,
    Species::kVullaby,
    Species::kFletchling,
};

static const Species PETALBURG_WOODS[] = {
    Species::kCaterpie,
    Species::kWeedle,
    Species::kWurmple,
    Species::kBurmy,
    Species::kSewaddle,
    Species::kVenipede,
    Species::kScatterbug,
};

static const Species ROUTE_104_NORTH[] = {
    Species::kSpearow,
    Species::kFarfetchd,
    Species::kHoothoot,
    Species::kMurkrow,
    Species::kSkarmory,
    Species::kChatot,
    Species::kDucklett,
    Species::kRufflet,
    Species::kHawlucha
};

static const Species PETALBURG_CITY[] = {
    Species::kUnown
};

static const Species ROUTE_116[] = {
    Species::kMankey,
    Species::kAipom,
    Species::kSlakoth,
    Species::kChimchar,
    Species::kPansage,
    Species::kPansear,
    Species::kPanpour,
    Species::kDarumaka,
};

static const Species RUSTURF_TUNNEL[] = {
    Species::kZubat,
    Species::kGligar,
    Species::kWoobat,
    Species::kNoibat
};

static const EncounterEntry s_table[] = {
    {MapID::kRoute101, SIZE(ROUTE_101), ROUTE_101},
    {MapID::kRoute102, SIZE(ROUTE_102), ROUTE_102},
    {MapID::kRoute103, SIZE(ROUTE_103), ROUTE_103},
    {MapID::kPetalburgCity, SIZE(PETALBURG_CITY), PETALBURG_CITY},
    {MapID::kRoute104South, SIZE(ROUTE_104_SOUTH), ROUTE_104_SOUTH},
    {MapID::kPetalburgWoods, SIZE(PETALBURG_WOODS), PETALBURG_WOODS},
    {MapID::kRoute104North, SIZE(ROUTE_104_NORTH), ROUTE_104_NORTH},
};
static const u32 s_table_size = SIZE(s_table);


const EncounterEntry* GetEncounterEntry(MapID map_id) {
  for (u16 i = 0; i < s_table_size; ++i) {
    if (s_table[i].map_id == map_id) {
      return &s_table[i];
    }
  }
  return nullptr;
}

void PatchEncounterTable(overworld::EncounterData* data) {
  auto& map_id = overworld::MapManager::GetInstance().GetMapId();
  const EncounterEntry* entry = GetEncounterEntry(static_cast<MapID>(map_id));
  if (entry == nullptr) return;

  u32 offset = 0;
  for (u16 i = 0; i < 61; ++i) {
    if (data->poke_info[i].species == 0) continue;
    data->poke_info[i].species = static_cast<u16>(entry->species[offset]);
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