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

#include "battle/config.h"

#include "core/core.h"
#include "hack/hook_manager.h"
#include "menu/log_menu.h"
#include "menu/plugin_menu.h"

namespace battle {

#include "battle/config.inc"

static struct {
  bool is_enabled;
  u8 battle_format;
  u8 background;
  u8 platform;
  u8 ground;
  u32 encounter_animation;
  u32 background_music;
  f32 money_rate;
  u32 flags;
  bool use_skybox;
  bool is_sky_battle;
  bool is_inverse_battle;
  bool is_capture_forced;
  bool no_money;
} ctx = {.is_enabled = false,
         .battle_format = 0,  // single
         .background = 1,
         .platform = 12,
         .ground = 2,
         .encounter_animation = 3,
         .background_music = (1 << 16) + 2,
         .money_rate = 1.0f,
         .flags = 0,
         .use_skybox = true};

void SetupBattleConfig(Config* config, void* game_manager, void* opponent_team,
                       void* p1, u8 battle_format, u32 effect_id, void* p2) {
  HookManager::GetInstance()
      .Get(HookID::kSetupBattleConfig)
      ->CallOriginal<void>(config, game_manager, opponent_team, p1,
                           battle_format, effect_id, p2);

  menu::LogMenu::GetInstance().Add(u"%p", &config->pokemon_teams[0]);
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
  }
}

void Config::LoadMenu(menu::PluginMenu& menu, void* args) {
  HookManager::GetInstance().Add(HookID::kSetupBattleConfig,
                                 ADDRESS_BATTLE_SETUP_CONFIG,
                                 (uptr)SetupBattleConfig);

  menu.Add("Is Enabled", ctx.is_enabled)
      .Add("Battle Format", ctx.battle_format)
      .WithArray(FORMATS, SIZE(FORMATS))
      .Add("Ground", ctx.ground)
      .WithArray(GROUNDS, SIZE(GROUNDS))
      .Add("Background", ctx.background)
      .WithArray(BACKGROUNDS, SIZE(BACKGROUNDS))
      .Add("Platform", ctx.platform)
      .WithArray(PLATFORMS, SIZE(PLATFORMS))
      .Add("Use Skybox", ctx.use_skybox)
      .Add("Encounter Animation", ctx.encounter_animation)
      .WithArray(ENCOUNTER_ANIMATIONS, SIZE(ENCOUNTER_ANIMATIONS))
      .Add("Background Music", ctx.background_music)
      .Add("Is Sky Battle", ctx.is_sky_battle)
      .Add("Is Inverse Battle", ctx.is_inverse_battle)
      .Add("Is Capture Forced", ctx.is_capture_forced)
      .Add("Long animation", &ctx.flags, 16, 1)
      .Add("Is Deoxys Event", &ctx.flags, 19, 1);
}

}  // namespace battle