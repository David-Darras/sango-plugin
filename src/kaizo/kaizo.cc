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

#include "feature/battle/feature_battle.h"
#include "feature/battle/feature_battle_config.h"
#include "feature/pokemon/feature_item.h"
#include "feature/rendering/feature_light.h"
#include "feature/overworld/feature_map_data_loader.h"
#include "feature/overworld/feature_overworld.h"
#include "feature/pokemon/feature_shiny.h"
#include "feature/ui/feature_title_screen.h"
#include "game/global_data/trainer_model_manager.h"
#include "game/savedata/pokemon_team.h"
#include "game/savedata/settings.h"
#include "game/savedata/trainer_status.h"

namespace kaizo {
void Initialize() {
  PatchTechnicalMoves();
  PatchPokemonData();
  PatchMoveData();
  PatchOutline();
  PatchBattle();
  PatchTrainerModels();
  InitializeOverworldWeather();
  InitializeEvolveHook();
  InitializeGiftHook();
  InitializeModelHook();
  InitializeStarterHook();

  // Shiny
  {
    feature::Shiny::GetInstance().rate = feature::Shiny::k1_8;
  }
  // Map Data
  {
    feature::MapDataLoader::GetInstance().is_contact_enabled = false;
  }
  // Item (useless just use in-game pokémon editor)
  // {
  //   feature::Item::GetInstance().remove_limit = true;
  // }
  // Music
  {
    feature::Overworld::GetInstance().freeze_background_music = true;
    feature::Overworld::GetInstance().background_music = 31;
  }
  // Title Screen
  {
    auto& title = feature::TitleScreen::GetInstance();
    title.is_enabled = true;
    title.no_shadow = true;
    title.no_delay = true;
    title.top_video = Video::kPrimoKyogre;
    title.bottom_video = Video::kPrimoGroudon;
    title.pokemon_cry_species = Species::kBeldum;
    title.pokemon_cry_volume = 1.0f;
  }
  // Camera
  {
    auto& camera = feature::Camera::GetInstance();
    camera.overworld_state = feature::Camera::kTps;
  }
}

void PatchOutline() {
  auto& light = feature::Light::GetInstance();
  light.use_outline = true;
  light.outline_scale = 0.0f;
}

void PatchTrainerModels() {
  auto& manager = TrainerModelManager::GetInstance();
  manager.Replace(TrainerModel::kBrendan, TrainerModel::kSteven);
  manager.Replace(TrainerModel::kMay, TrainerModel::kZinnia);
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