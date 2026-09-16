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

#include "battle/patch/setup.h"
#include "core/hook_manager.h"
#include "battle/native/config.h"
#include "savedata/native/pokemon_team.h"

namespace battle {

void Setup::Initialize() {
  core::HookManager::Initialize(HookId::kBattleConfigSetupWild,
                          address::kConfigSetupWild,
                          (uptr)SetupWildHook);

  core::HookManager::Initialize(HookId::kBattleConfigSetupTrainer,
                          address::kConfigSetupTrainer,
                          (uptr)SetupTrainerHook);
}

void Setup::SetupTrainerHook(Config* config, void* game_manager,
                             TrainerId trainer_id, void* p1, Format format,
                             void* p2) {
  const TrainerId forced = GetInstance().trainer_id;
  if (forced != TrainerId::kNone) trainer_id = forced;
  core::HookManager::Call<void>(HookId::kBattleConfigSetupTrainer,
                          config, game_manager, trainer_id, p1,
                          format,
                          p2);
  auto& feat = GetInstance();
  if (feat.on_trainer_battle != nullptr) {
    feat.on_trainer_battle(*config, trainer_id);
  }
}

void Setup::SetupWildHook(Config* config,
                                 void* game_manager,
                                 void* opponent_team,
                                 void* p1, Format format,
                                 u32 effect_id,
                                 void* p2) {
  core::HookManager::Call<void>(HookId::kBattleConfigSetupWild, config,
                          game_manager,
                          opponent_team, p1,
                          format, effect_id, p2);

  auto& ctx = GetInstance();

  if (ctx.inverse_teams) {
    savedata::PokemonTeam* team = config->pokemon_teams[0];
    config->pokemon_teams[0] = config->pokemon_teams[1];
    config->pokemon_teams[1] = team;

    team = config->pokemon_teams[2];
    config->pokemon_teams[2] = config->pokemon_teams[3];
    config->pokemon_teams[3] = team;
  }

  if (ctx.is_enabled) {
    config->format = ctx.format;
    config->background = ctx.background;
    config->platform = ctx.platform;
    config->ground = ctx.ground;
    config->encounter_animation = ctx.encounter_animation;
    config->background_music = ctx.background_music;
    config->money_rate = ctx.money_rate;
    config->flags = ctx.flags;
    config->use_skybox = ctx.use_skybox;
    config->is_sky_battle = ctx.is_sky_battle;
    config->is_inverse_battle = ctx.is_inverse_battle;
    config->is_capture_forced = ctx.is_capture_forced;
    config->no_money = ctx.no_money;

    config->pokemon_teams[1]->pokemons[0]->accessor->Decrypt();
    config->pokemon_teams[1]->pokemons[0]->core->species = ctx.species;
    config->pokemon_teams[1]->pokemons[0]->core->form = ctx.form;
    config->pokemon_teams[1]->pokemons[0]->accessor->Encrypt();
  }
}

} // namespace battle
