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

#include "battle/patch/battle.h"
#include "battle/patch/setup.h"
#include "core/patch/archive.h"
#include "core/patch/process_patch.h"
#include "core/utils.h"
#include "kaizo/kaizo.h"
#include "kaizo/kaizo_painter.h"
#include "overworld/patch/field.h"
#include "overworld/patch/gift_pokemon.h"
#include "overworld/patch/static_randomizer.h"
#include "overworld/patch/trade.h"
#include "overworld/patch/wild_encounter.h"
#include "plugin.h"
#include "pokemon/patch/item_customizer.h"
#include "ui/patch/app_status.h"
#include "ui/patch/title_screen.h"

namespace {
bool IsCaptureAllowed() { return !kaizo::CapturedEvent::Check(); }

void OnCaptured() { kaizo::CapturedEvent::Set(); }

u32 OnStreamFile(const u32* archive, u32 file_id) {
  if (core::Archive::IsArchive(archive, ArchiveId::kOverworldModel)) {
    return static_cast<u32>(kaizo::PatchOverworldModels(
        static_cast<ModelId>(file_id), true));
  }
  return file_id;
}

void OnReadFile(core::ArchiveInput* input) {
  if (core::Archive::IsArchive(input, ArchiveId::kOverworldModel)) {
    input->file_id = static_cast<u32>(kaizo::PatchOverworldModels(
        static_cast<ModelId>(input->file_id), false));
  }
  if (core::Archive::IsArchive(input, ArchiveId::kPlayerIcon)) {
    input->file_id = 72; // Steven
  }
}

void OnProcessLoad(uptr vtable) {
  kaizo::ShouldReplacePokemonModel(false);
  switch (vtable) {
    case core::address::kIntroductionVtable:
    case core::address::kCinematicVtable:
      kaizo::ShouldReplacePokemonModel(true);
      break;
    case core::address::kTitleScreenVtable:
      if (ui::TitleScreen::GetInstance().is_enabled) {
        kaizo::ShouldReplacePokemonModel(true);
      }
      break;
    case core::address::kSelectStarterVtable:
      kaizo::PatchStarterView();
      break;
    default:
      break;
  }
}

void OnWildPokemon(MapId map_id, overworld::WildPokemon* pokemons,
                   u32 count) {
  const kaizo::EncounterEntry* entry = kaizo::GetEncounterEntry(map_id);
  if (entry == nullptr) return;
  for (u32 i = 0; i < count; i++) {
    pokemons[i].level = kaizo::GetEncounterLevel();
    pokemons[i].species = entry->species[core::Utils::GetRandomValue(entry->size)];
  }
}

void OnFieldLoad() {
  kaizo::PatchOverworld();
  kaizo::PatchBag();
  kaizo::RestoreTeamAfterBattle();
}

void InstallCallbacks() {
  auto& battle = battle::Battle::GetInstance();
  battle.unlimited_mega_evolution = false;
  battle.is_capture_allowed = IsCaptureAllowed;
  battle.on_captured = OnCaptured;

  battle::Setup::GetInstance().on_trainer_battle =
      kaizo::PatchTrainerData;

  overworld::GiftPokemon::GetInstance().randomize_species = true;
  overworld::StaticRandomizer::GetInstance().randomize_species = true;
  overworld::Trade::GetInstance().randomize_species = true;

  auto& archive = core::Archive::GetInstance();
  archive.on_stream_file = OnStreamFile;
  archive.on_read_file = OnReadFile;

  core::ProcessPatch::GetInstance().on_process_load = OnProcessLoad;

  auto& encounter = overworld::WildEncounter::GetInstance();
  encounter.on_encounter_table = kaizo::PatchEncounterTable;
  encounter.on_wild_pokemon = OnWildPokemon;

  overworld::Field::GetInstance().on_field_load = OnFieldLoad;
  pokemon::ItemCustomizer::GetInstance().on_item_data = kaizo::PatchItemData;
  ui::AppStatus::GetInstance().is_restricted = true;
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
