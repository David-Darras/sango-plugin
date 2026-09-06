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
#include "common.h"
#include "game/constant/map.h"
#include "game/constant/species.h"
#include "game/overworld/map_manager.h"
#include "game/savedata/pss_photo.h"
#include "game/savedata/savedata.h"

namespace global_data {
struct Item;
}

namespace battle {
struct Team;
struct Config;
}

namespace overworld {
struct EncounterData;
}

namespace kaizo {
struct EncounterEntry {
  const MapId map_id;
  const u16 size;
  const Species* species;
};

struct CapturedEvent {
  STATIC_INLINE bool Check() {
    // u32 id = overworld::MapManager::last_map_id;
    u32 id = static_cast<u32>(MapId::kRoute101);
    auto& data = savedata::PssPhoto::GetInstance();
    u8* bits = &data.photo[0];
    u32 index = id / 8;
    u32 offset = id % 8;
    return bits[index] & (1U << offset);
  }

  STATIC_INLINE void Set() {
    // u32 id = overworld::MapManager::last_map_id;
    u32 id = static_cast<u32>(MapId::kRoute101);
    auto& data = savedata::PssPhoto::GetInstance();
    u8* bits = &data.photo[0];
    u32 index = id / 8;
    u32 offset = id % 8;
    bits[index] |= (1U << offset);
  }

  STATIC_INLINE void Reset(u32 id) {
    auto& data = savedata::PssPhoto::GetInstance();
    u8* bits = &data.photo[0];
    u32 index = id / 8;
    u32 offset = id % 8;
    bits[index] &= ~(1U << offset);
  }
};

extern void PatchOverworld();
extern void PatchTechnicalMoves();
extern void PatchBag();
extern void PatchPokemonData();
extern void PatchMoveData();
extern void PatchOutline();
extern void PatchTrainerModels();
extern void InitializeOverworldWeather();
extern void UpdateOverworldWeather();
extern void InitializeStarterHook();
extern void InitializeGiftHook();
extern void PatchEncounterTable(overworld::EncounterData* data);
extern const EncounterEntry* GetEncounterEntry(MapId map_id);
extern void PatchTrainerData(battle::Config& config, u16& trainer_id);
extern u32 PatchOverworldModels(u32 model, bool is_real_overworld);
extern void ApplyLevelCaps(battle::Team* team, void* data);
extern void PatchItemData(global_data::Item* item);
extern void InitializeModelHook();
extern void ShouldReplacePokemonModel(bool no_yes);
extern void PatchStarterView();
extern void PatchBattle();
extern void SetFirstEncounter();
extern bool IsNotFirstEncounter();
extern u8 GetEncounterLevel();
extern void SaveTeamBeforeBattle();
extern void RestoreTeamAfterBattle();

extern void Initialize();
} // namespace kaizo
