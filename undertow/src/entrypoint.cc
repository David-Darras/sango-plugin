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

#include "core/native/process_manager.h"
#include "core/patch/archive.h"
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "overworld/patch/run_animation.h"
#include "plugin.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/species.h"
#include "pokemon/patch/model_replacement.h"
#include "savedata/native/overworld_menu.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"
#include "ui/painter.h"
#include "ui/patch/new_game.h"
#include "ui/patch/title_screen.h"

namespace undertow {
void InstallScripts();
}

namespace {
ModelId PlayerAsAquaGrunt(ModelId model) {
  if (model == ModelId::kBrendan) return ModelId::kTeamAquaGruntMale;
  return model;
}

u32 PlayerAsAquaGrunt(u32 file_id) {
  return static_cast<u32>(PlayerAsAquaGrunt(static_cast<ModelId>(file_id)));
}

u32 OnStreamFile(const u32* archive, u32 file_id) {
  if (core::Archive::IsArchive(archive, ArchiveId::kOverworldModel)) {
    return PlayerAsAquaGrunt(file_id);
  }
  return file_id;
}

void OnReadFile(core::ArchiveInput* input) {
  if (core::Archive::IsArchive(input, ArchiveId::kOverworldModel)) {
    input->file_id = PlayerAsAquaGrunt(input->file_id);
  }
}

void OnPokemonModel(PokeInfo* info) {
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(
      core::address::kTitleScreenVtable)) {
    return;
  }
  if (info->form != FormId::kNormal) return;
  switch (info->species) {
    case SpeciesId::kGroudon:
      info->species = SpeciesId::kCamerupt;
      info->form = FormId::kCameruptMega;
      info->is_shiny = true;
      break;
    case SpeciesId::kKyogre:
      info->species = SpeciesId::kSharpedo;
      info->form = FormId::kSharpedoMega;
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

  auto& new_game = ui::NewGame::GetInstance();
  new_game.skip_intro = true;
  new_game.player_name = u"Aqua Grunt";
  new_game.player_gender = Gender::kMale;
  new_game.start_zone = MapId::kAquaHideout;
  new_game.start_tile_x = 8;
  new_game.start_tile_z = 24;
  new_game.start_facing = overworld::Facing::kUp;
  new_game.on_new_game = []() {
    savedata::OverworldMenu::GetInstance().DisableAll();
  };

  auto& title = ui::TitleScreen::GetInstance();
  title.is_enabled = true;
  title.top_video = VideoId::kPrimoGroudon;
  title.bottom_video = VideoId::kPrimoKyogre;
  title.pokemon_cry_species = SpeciesId::kSharpedo;
  title.skip_to_frame = 800;
  title.no_shadow = true;
  pokemon::ModelReplacement::GetInstance().on_create = OnPokemonModel;

  auto& archive = core::Archive::GetInstance();
  archive.on_stream_file = OnStreamFile;
  archive.on_read_file = OnReadFile;
  overworld::RunAnimation::GetInstance().enabled = true;

  undertow::InstallScripts();

  plugin::LoadConfiguration();
  plugin::OpenMenu(ui::MainAppPainter::GetInstance(),
                   MainPage);
  plugin::Start(EveryFrame);
}
