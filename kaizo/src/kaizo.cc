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

#include "kaizo/kaizo.h"

#include "renderer/patch/lighting.h"
#include "overworld/patch/camera.h"
#include "overworld/patch/map_data_loader.h"
#include "overworld/patch/field.h"
#include "pokemon/patch/shiny.h"
#include "ui/patch/title_screen.h"
#include "battle/native/trainer_model_manager.h"
#include "savedata/native/pokemon_team.h"
#include "savedata/native/settings.h"
#include "savedata/native/trainer_status.h"

namespace kaizo {
void Initialize() {
  PatchTechnicalMoves();
  PatchPokemonData();
  PatchMoveData();
  PatchOutline();
  PatchBattle();
  PatchTrainerModels();
  InitializeOverworldWeather();
  InitializeGiftHook();
  InitializeModelHook();
  InitializeStarterHook();
  InitializeTrainerTeams();

  pokemon::Shiny::GetInstance().rate = pokemon::ShinyRate::k1_8;
  overworld::MapDataLoader::GetInstance().is_contact_enabled = false;
  overworld::Field::GetInstance().freeze_background_music = true;
  overworld::Field::GetInstance().background_music = BackgroundMusicId::kCinema;
  overworld::Camera::GetInstance().overworld_state =
      overworld::CameraState::kTps;

  auto& title = ui::TitleScreen::GetInstance();
  title.is_enabled = true;
  title.no_shadow = true;
  title.no_delay = true;
  title.top_video = VideoId::kPrimoKyogre;
  title.bottom_video = VideoId::kPrimoGroudon;
  title.pokemon_cry_species = SpeciesId::kBeldum;
  title.pokemon_cry_volume = 1.0f;
}

void PatchOutline() {
  auto& light = renderer::Lighting::GetInstance();
  light.use_outline = true;
  light.outline_scale = 0.0f;
}

void PatchTrainerModels() {
  auto& manager = battle::TrainerModelManager::GetInstance();
  manager.Replace(TrainerModelId::kBrendan, TrainerModelId::kSteven);
  manager.Replace(TrainerModelId::kMay, TrainerModelId::kZinnia);
}

void PatchOverworld() {
  // Player's Name
  {
    static const c16* NICKNAME = u"STEVEN";
    auto& status = savedata::TrainerStatus::GetInstance();
    for (u32 i = 0; i < savedata::TrainerStatus::kPlayerNameLen; i++) {
      status.name[i] = status.nickname[i] = NICKNAME[i];
      if (NICKNAME[i] == '\0') break;
    }
  }
  // Config
  {
    auto& data = savedata::Settings::GetInstance();
    data.text_speed = 3; // Instant message
    data.battle_style = 1;
    data.show_battle_animations = 0;
  }
  // After a battle
  {
    auto& team = savedata::PokemonTeam::GetInstance();
    team.ThrowAllDeadPokemons();
    team.HealAllPokemons();
  }
}
}