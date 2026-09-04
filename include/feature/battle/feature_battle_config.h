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
#include "feature/core/hook_manager.h"
#include "game/battle/config.h"
#include "common.h"
#include "game/constant/form.h"
#include "ui/log_application.h"
#include "kaizo/kaizo.h"
#include "game/constant/battle_background.h"
#include "game/constant/battle_encounter_animation.h"
#include "game/constant/battle_platform.h"
#include "game/constant/battle_trainer.h"
#include "game/constant/battle_format.h"
#include "game/constant/species.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"

namespace feature {
struct BattleConfig {
  MAKE_SINGLETON(BattleConfig)
  bool is_enabled = false;
  BattleFormat battle_format = BattleFormat::kSingle;
  BattleBackground background = BattleBackground::kAquaBoss;
  BattlePlatform platform = BattlePlatform::kBossAqua;
  BattleEncounterAnimation ground = BattleEncounterAnimation::kAquaTeamBoss;
  BattleEncounterAnimation encounter_animation = BattleEncounterAnimation::kKyogre;
  Species species = Species::kKyogre;
  Form form = Form::kKyogreAlpha;
  u32 background_music = (1 << 16) + 79;
  f32 money_rate = 1.0f;
  u32 flags = 0;
  bool use_skybox = true;
  bool is_sky_battle = false;
  bool is_inverse_battle = false;
  bool is_capture_forced = false;
  bool no_money = false;
  bool inverse_teams = false;
  BattleTrainer trainer_id = BattleTrainer::kNone;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kSetupBattleConfig,
                            ADDRESS_BATTLE_SETUP_CONFIG,
                            (uptr)SetupBattleConfigHook);

    HookManager::Initialize(HookID::kSetupTrainerBattleConfig,
                            ADDRESS_SETUP_TRAINER_BATTLE_CONFIG,
                            (uptr)SetupTrainerBattleConfigHook);
  }

  static void SetupTrainerBattleConfigHook(battle::Config* config,
                                           void* game_manager, u16 trainer_id,
                                           void* p1, u8 battle_format,
                                           void* p2) {
    auto& tid = GetInstance().trainer_id;
    if (tid != BattleTrainer::kNone)
      trainer_id = static_cast<u16>(tid);
    HookManager::Call<void>(HookID::kSetupTrainerBattleConfig,
                            config, game_manager, trainer_id, p1,
                            battle_format,
                            p2);
#ifdef KAIZO
    kaizo::PatchTrainerData(*config, trainer_id);
#endif
  }

  static void SetupBattleConfigHook(battle::Config* config, void* game_manager,
                                    void* opponent_team,
                                    void* p1, u8 battle_format, u32 effect_id,
                                    void* p2) {
    HookManager::Call<void>(HookID::kSetupBattleConfig, config, game_manager,
                            opponent_team, p1,
                            battle_format, effect_id, p2);

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
      config->battle_format = static_cast<u8>(ctx.battle_format);
      config->background = static_cast<u8>(ctx.background);
      config->platform = static_cast<u8>(ctx.platform);
      config->ground = static_cast<u8>(ctx.ground);
      config->encounter_animation = static_cast<u32>(ctx.encounter_animation);
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
};
} // namespace feature