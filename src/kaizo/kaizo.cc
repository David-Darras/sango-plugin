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

#include "kaizo.h"

#include "feature/feature_light.h"
#include "game/global_data/trainer_model_manager.h"
#include "game/savedata/pokemon_team.h"
#include "game/savedata/settings.h"
#include "game/savedata/trainer_status.h"

namespace kaizo {
void Initialize() {
  PatchShopData();
  PatchPokemonData();
  PatchMoveData();
  PatchOutline();
  PatchTrainerModels();
  InitializeOverworldWeather();
}

void PatchOutline() {
  auto& light = feature::Light::GetInstance();
  light.use_outline = true;
  light.outline_scale = 0.0f;
}

void PatchTrainerModels() {
  auto& manager = TrainerModelManager::GetInstance();
  manager.Replace(TRAINER_MODEL_BRENDAN, TRAINER_MODEL_STEVEN);
  manager.Replace(TRAINER_MODEL_MAY, TRAINER_MODEL_ZINNIA);
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
    team.HealAllPokemons();
  }
}
}