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

namespace battle {
struct Team;
struct Config;
}

namespace overworld {
struct EncounterData;
}

namespace kaizo {
struct EncounterEntry {
  const u16 map_id;
  const u16 size;
  const u16* species;
};

extern void PatchOverworld();
extern void PatchShopData();
extern void PatchPokemonData();
extern void PatchMoveData();
extern void PatchOutline();
extern void PatchTrainerModels();
extern void InitializeOverworldWeather();
extern void UpdateOverworldWeather();
extern void InitializeStarterHook();
extern void InitializeGiftHook();
extern void InitializeEvolveHook();
extern void PatchEncounterTable(overworld::EncounterData* data);
extern const EncounterEntry* GetEncounterEntry(u16 map_id);
extern void PatchTrainerData(battle::Config& config, u16& trainer_id);
extern u32 PatchOverworldModels(u32 model, bool is_real_overworld);
extern void InitializeShinyHook();
extern void InitializeArchiveHooks();
extern void ApplyLevelCaps(battle::Team* team, void* data);

extern void Initialize();
} // namespace kaizo