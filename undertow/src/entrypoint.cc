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

#include "feature/core/feature_archive.h"
#include "feature/overworld/feature_run_animation.h"
#include "feature/pokemon/feature_pokemon_model.h"
#include "feature/ui/feature_new_game.h"
#include "feature/ui/feature_title_screen.h"
#include "game/constant/map.h"
#include "game/constant/form.h"
#include "game/constant/model.h"
#include "game/constant/species.h"
#include "game/core/process_manager.h"
#include "plugin.h"
#include "game/savedata/overworld_menu.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "ui/painter.h"
#include "undertow/undertow.h"

namespace {
constexpr MapId kAquaHideout = static_cast<MapId>(93);

u32 PlayerAsAquaGrunt(u32 file_id) {
  if (file_id == static_cast<u32>(ModelId::kBrendan)) {
    return static_cast<u32>(ModelId::kTeamAquaGruntMale);
  }
  return file_id;
}

u32 OnStreamFile(const u32* archive, u32 file_id) {
  if (feature::ArchivePatch::IsArchive(archive, ArchiveID::kOverworldModel)) {
    return PlayerAsAquaGrunt(file_id);
  }
  return file_id;
}

void OnReadFile(feature::ArchivePatch::Input* input) {
  if (feature::ArchivePatch::IsArchive(input, ArchiveID::kOverworldModel)) {
    input->file_id = PlayerAsAquaGrunt(input->file_id);
  }
}

void OnPokemonModel(PokeInfo* info) {
  if (!game::ProcessManager::GetInstance().IsCurrentProcess(
      ADDRESS_TITLE_SCREEN_VTABLE)) {
    return;
  }
  if (info->form != Form::kNormal) return;
  switch (info->species) {
    case Species::kGroudon:
      info->species = Species::kCamerupt;
      info->form = Form::kCameruptMega;
      info->is_shiny = true;
      break;
    case Species::kKyogre:
      info->species = Species::kSharpedo;
      info->form = Form::kSharpedoMega;
      info->is_shiny = false;
      break;
    default:
      break;
  }
}

void EveryFrame() {
  plugin::UpdateFrame();
  plugin::DrawFrame();
}
}

static void MainPage(ui::MainApplication& app, void* args) {
  app.Add("HMs", ui::LoadOverworldFieldMovePage);
}

void Initialize() {
  plugin::InitializeEngine();

  auto& new_game = feature::NewGame::GetInstance();
  new_game.skip_intro = true;
  new_game.player_name = u"Aqua Grunt";
  new_game.player_gender = Gender::kMale;
  new_game.start_zone = kAquaHideout;
  new_game.start_tile_x = 8;
  new_game.start_tile_z = 24;
  new_game.start_facing = overworld::Facing::kUp;
  new_game.on_new_game = []() {
    savedata::OverworldMenu::GetInstance().DisableAll();
  };

  auto& title = feature::TitleScreen::GetInstance();
  title.is_enabled = true;
  title.top_video = Video::kPrimoGroudon;
  title.bottom_video = Video::kPrimoKyogre;
  title.pokemon_cry_species = Species::kSharpedo;
  title.skip_to_frame = 800;
  title.no_shadow = true;
  feature::PokemonModel::GetInstance().on_create = OnPokemonModel;

  auto& archive = feature::ArchivePatch::GetInstance();
  archive.on_stream_file = OnStreamFile;
  archive.on_read_file = OnReadFile;
  feature::RunAnimation::GetInstance().enabled = true;

  undertow::InstallScripts();

  plugin::LoadConfiguration();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(),
                   MainPage);
  plugin::Start(EveryFrame);
}