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
 * Pokémon Sango Kaizo: the ROM hack, built on the library.
 */

#include "feature/battle/feature_battle.h"
#include "feature/battle/feature_battle_config.h"
#include "feature/core/feature_archive.h"
#include "feature/core/feature_process_patch.h"
#include "feature/overworld/feature_encounter.h"
#include "feature/overworld/feature_overworld.h"
#include "feature/pokemon/feature_item.h"
#include "feature/ui/feature_title_screen.h"
#include "feature/ui/feature_app_status.h"
#include "kaizo/kaizo.h"
#include "kaizo/kaizo_painter.h"
#include "plugin.h"
#include "utils.h"

namespace {
/* ---------------------------------------------------------
Where the hack plugs into the library's features.
--------------------------------------------------------- */

bool IsCaptureAllowed() { return !kaizo::CapturedEvent::Check(); }

void OnCaptured() { kaizo::CapturedEvent::Set(); }

u32 OnStreamFile(const u32* archive, u32 file_id) {
  if (feature::ArchivePatch::IsArchive(archive, ArchiveID::kOverworldModel)) {
    return kaizo::PatchOverworldModels(file_id, true);
  }
  return file_id;
}

void OnReadFile(feature::ArchivePatch::Input* input) {
  if (feature::ArchivePatch::IsArchive(input, ArchiveID::kOverworldModel)) {
    input->file_id = kaizo::PatchOverworldModels(input->file_id, false);
  }
  if (feature::ArchivePatch::IsArchive(input, ArchiveID::kPlayerIcon)) {
    input->file_id = 72; // Steven
  }
}

void OnProcessLoad(uptr vtable) {
  kaizo::ShouldReplacePokemonModel(false);
  switch (vtable) {
    case ADDRESS_INTRODUCTION_VTABLE:
    case ADDRESS_CINEMATIC_VTABLE:
      kaizo::ShouldReplacePokemonModel(true);
      break;
    case ADDRESS_TITLE_SCREEN_VTABLE:
      if (feature::TitleScreen::GetInstance().is_enabled) {
        kaizo::ShouldReplacePokemonModel(true);
      }
      break;
    case ADDRESS_SELECT_STARTER_VTABLE:
      kaizo::PatchStarterView();
      break;
    default:
      break;
  }
}

void OnWildPokemon(MapId map_id, feature::Encounter::PokemonData* pokemons,
                   u32 count) {
  const kaizo::EncounterEntry* entry = kaizo::GetEncounterEntry(map_id);
  if (entry == nullptr) return;
  for (u32 i = 0; i < count; i++) {
    pokemons[i].level = kaizo::GetEncounterLevel();
    pokemons[i].species = entry->species[Utils::GetRandomValue(entry->size)];
  }
}

void OnFieldLoad() {
  kaizo::PatchOverworld();
  kaizo::PatchBag();
  kaizo::RestoreTeamAfterBattle();
}

void InstallCallbacks() {
  auto& battle = feature::Battle::GetInstance();
  battle.unlimited_mega_evolution = false;
  battle.is_capture_allowed = IsCaptureAllowed;
  battle.on_captured = OnCaptured;

  feature::BattleConfig::GetInstance().on_trainer_battle =
      kaizo::PatchTrainerData;

  auto& archive = feature::ArchivePatch::GetInstance();
  archive.on_stream_file = OnStreamFile;
  archive.on_read_file = OnReadFile;

  feature::ProcessPatch::GetInstance().on_process_load = OnProcessLoad;

  auto& encounter = feature::Encounter::GetInstance();
  encounter.on_encounter_table = kaizo::PatchEncounterTable;
  encounter.on_wild_pokemon = OnWildPokemon;

  feature::Overworld::GetInstance().on_field_load = OnFieldLoad;
  feature::Item::GetInstance().on_item_data = kaizo::PatchItemData;
  feature::AppStatus::GetInstance().is_restricted = true;
}

void EveryFrame() {
  plugin::UpdateFrame();
  kaizo::UpdateOverworldWeather();
  plugin::DrawFrame();
}
} // namespace

void Initialize() {
  plugin::InitializeEngine();
  InstallCallbacks();
  kaizo::Initialize();

  plugin::LoadConfiguration();
  plugin::OpenMenu(kaizo::Painter::GetInstance(), kaizo::LoadMenuPage);
  plugin::Start(EveryFrame);
}
