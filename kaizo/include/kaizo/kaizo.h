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
 * @file kaizo.h
 * @brief The functions and the data of Pokémon Sango Kaizo.
 *
 * Kaizo is a complete ROM hack made with the library. Use it as an example.
 */

#pragma once
#include "common.h"
#include "battle/constant/trainer.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "pokemon/constant/species.h"
#include "savedata/native/pss_photo.h"

namespace pokemon {
struct ItemData;
}

namespace battle {
struct Team;
struct Config;
}

namespace overworld {
struct EncounterData;
}

namespace ui {
class MainApplication;
}

namespace kaizo {
struct EncounterEntry {
  const MapId map_id;
  const u16 size;
  const SpeciesId* species;
};

/// Returns the map of the current battle: the last map of the overworld.
extern MapId GetBattleMap();
/// Remembers the current map. Call it one time for each frame.
extern void UpdateBattleMap();

/// Remembers the maps where the player caught a Pokémon (Nuzlocke rule).
/// It uses one bit for each map in the photo bits of the PSS save data.
struct CapturedEvent {
  /// Returns true when the player caught a Pokémon on the map of the battle.
  STATIC_INLINE bool Check() { return Check(GetBattleMap()); }

  /// Remembers that the player caught a Pokémon on the map of the battle.
  STATIC_INLINE void Set() { Set(GetBattleMap()); }

  /// Returns true when the player caught a Pokémon on a map.
  STATIC_INLINE bool Check(MapId map) {
    const u32 id = static_cast<u32>(map);
    return (GetBits()[id / 8] & (1U << (id % 8))) != 0;
  }

  /// Remembers that the player caught a Pokémon on a map.
  STATIC_INLINE void Set(MapId map) {
    const u32 id = static_cast<u32>(map);
    GetBits()[id / 8] |= (1U << (id % 8));
  }

  /// Forgets the capture of a map.
  STATIC_INLINE void Reset(u32 id) {
    GetBits()[id / 8] &= ~(1U << (id % 8));
  }

private:
  STATIC_INLINE u8* GetBits() {
    return &savedata::PssPhoto::GetInstance().photo[0];
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
extern void PatchEncounterTable(overworld::EncounterData* data);
extern const EncounterEntry* GetEncounterEntry(MapId map_id);
extern void PatchTrainerData(battle::Config& config, TrainerId& trainer_id);
extern void InitializeTrainerTeams();
extern ModelId PatchOverworldModels(ModelId model, bool is_real_overworld);
extern void ApplyLevelCaps(battle::Team* team, void* data);
extern void PatchItemData(pokemon::ItemData* item);
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

extern void LoadMenuPage(ui::MainApplication& app, void* args);
} // namespace kaizo
