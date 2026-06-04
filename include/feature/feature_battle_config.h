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
#include "hook_manager.h"
#include "battle/config.h"
#include "core/core.h"

namespace feature {
struct BattleConfigHookContext {
  MAKE_SINGLETON(BattleConfigHookContext)
  bool is_enabled = false;
  u8 battle_format = 0; // single
  u8 background = 87;
  u8 platform = 4;
  u8 ground = 87;
  u32 encounter_animation = 22; // rayquaza
  u32 background_music = (1 << 16) + 2;
  f32 money_rate = 1.0f;
  u32 flags = 0;
  bool use_skybox = true;
  bool is_sky_battle = false;
  bool is_inverse_battle = false;
  bool is_capture_forced = false;
  bool no_money = false;
  bool inverse_teams = false;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kSetupBattleConfig,
                            ADDRESS_BATTLE_SETUP_CONFIG,
                            (uptr)SetupBattleConfigHook);
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
      config->battle_format = ctx.battle_format;
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

      // config->pokemon_teams[0]->pokemons[0]->accessor->Decrypt();
      // config->pokemon_teams[0]->pokemons[0]->core->species = 94;
      // config->pokemon_teams[0]->pokemons[0]->accessor->Encrypt();
      //
      // config->pokemon_teams[1]->pokemons[0]->accessor->Decrypt();
      // config->pokemon_teams[1]->pokemons[0]->core->species = 487;
      // config->pokemon_teams[1]->pokemons[0]->core->form = 1;
      // config->pokemon_teams[1]->pokemons[0]->accessor->Encrypt();
    }
  }
};
} // namespace feature