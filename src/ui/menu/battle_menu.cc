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

#include <math.h>

#include <cstring>

#include "battle/manager.h"
#include "feature/feature_battle_config.h"
#include "menu/plugin_menu.h"

namespace ui {
#include "battle/config.inc"

void LoadBattleConfigMenu(menu::PluginMenu& menu, void* args) {
  auto& ctx = feature::BattleConfigHookContext::GetInstance();

  menu.Add("Inverse Teams", ctx.inverse_teams)
      .Add("Is Enabled", ctx.is_enabled)
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

static battle::Pokemon* pkm_server = nullptr;
static battle::Pokemon* pkm_client = nullptr;
static u8 team_idx = 0;
static u8 pokemon_idx = 0;

static void SavePokemon(void*) {
  for (u32 i = 0; i < 4; i++) {
    memcpy(&pkm_server->moves[i].core, &pkm_server->moves[i].view,
           sizeof(pkm_server->moves[i].view));
  }
  memcpy(pkm_client, pkm_server, sizeof(battle::Pokemon));
}

void LoadBattlePokemonDataMenu(menu::PluginMenu& menu, void* args) {
  if (menu.CheckProcess(PROCESS_NAME_BATTLE)) return;

  auto& pkm = *pkm_server;

  menu.Add("Save", SavePokemon)
      .Add("UID", pkm.uid)
      .AddSpecies("Species", pkm.species)

      .AddSeparator()
      .AddMove("Move 1 - ID", pkm.moves[0].view.id)
      .Add("Move 1 - PP", pkm.moves[0].view.pp)

      .AddMove("Move 2 - ID", pkm.moves[1].view.id)
      .Add("Move 2 - PP", pkm.moves[1].view.pp)

      .AddMove("Move 3 - ID", pkm.moves[2].view.id)
      .Add("Move 3 - PP", pkm.moves[2].view.pp)

      .AddMove("Move 4 - ID", pkm.moves[3].view.id)
      .Add("Move 4 - PP", pkm.moves[3].view.pp)

      .AddSeparator()
      .Add("Hp", pkm.hp)
      .Add("Max Hp", pkm.max_hp)
      .Add("Attack", pkm.attack)
      .Add("Defense", pkm.defense)
      .Add("Sp. Attack", pkm.special_attack)
      .Add("Sp. Defense", pkm.special_defense)
      .Add("Speed", pkm.speed)

      .AddSeparator()
      .Add("Atk Stage", pkm.stat_attack)
      .Add("Def Stage", pkm.stat_defense)
      .Add("SpA Stage", pkm.stat_special_attack)
      .Add("SpD Stage", pkm.stat_special_defense)
      .Add("Spe Stage", pkm.stat_speed)
      .Add("Acc Stage", pkm.stat_accuracy)
      .Add("Eva Stage", pkm.stat_evasion)

      .AddSeparator()
      .Add("EV Total", pkm.ev_sum)
      .Add("EV HP", pkm.ev_hp)
      .Add("EV Atk", pkm.ev_attack)
      .Add("EV Def", pkm.ev_defense)
      .Add("EV Spe", pkm.ev_speed)
      .Add("EV SpA", pkm.ev_special_attack)
      .Add("EV SpD", pkm.ev_special_defense)

      .AddSeparator()
      .AddType("Type 1", pkm.types[0])
      .AddType("Type 2", pkm.types[1])
      .AddType("Type 3", pkm.types[2])

      .Add("Level", pkm.level)
      .Add("Experience", pkm.experience)
      .Add("Friendship", pkm.friendship)
      .Add("Form", pkm.form)
      .Add("Gender", pkm.gender)

      .AddAbility("Ability", pkm.ability)
      .AddItem("Item", pkm.item)
      .Add("Weight", pkm.weight);
}

void LoadBattlePokemonModelMenu(menu::PluginMenu& menu, void* args) {
  if (menu.CheckProcess(PROCESS_NAME_BATTLE)) return;

  auto& model =
      battle::Manager::GetInstance().GetGraphics().GetPokemonModel(
          pokemon_idx);

  model.update = true;

  constexpr f32 kScaleFactor = 0.05f;
  constexpr f32 kRotationFactor = 4.0f * M_PI / 180.0f;
  constexpr f32 kPositionFactor = 4.0f;

  menu.Add("Position X", model.position.x)
      .WithFactor(kPositionFactor)
      .WithRefresh()
      .Add("Position Y", model.position.y)
      .WithFactor(kPositionFactor)
      .WithRefresh()
      .Add("Position Z", model.position.z)
      .WithFactor(kPositionFactor)
      .WithRefresh()
      .AddSeparator()
      .Add("Rotation X", model.rotation.x)
      .WithFactor(kRotationFactor)
      .WithRefresh()
      .Add("Rotation Y", model.rotation.y)
      .WithFactor(kRotationFactor)
      .WithRefresh()
      .Add("Rotation Z", model.rotation.z)
      .WithFactor(kRotationFactor)
      .WithRefresh()
      .AddSeparator()
      .Add("Scale X", model.scale.x)
      .WithFactor(kScaleFactor)
      .WithRefresh()
      .Add("Scale Y", model.scale.y)
      .WithFactor(kScaleFactor)
      .WithRefresh()
      .Add("Scale Z", model.scale.z)
      .WithFactor(kScaleFactor)
      .WithRefresh();
}

void LoadBattleMenu(menu::PluginMenu& menu, void* args) {
  if (menu.CheckProcess(PROCESS_NAME_BATTLE)) return;

  pkm_server = battle::Manager::GetPokemon(true, team_idx, pokemon_idx);
  pkm_client = battle::Manager::GetPokemon(false, team_idx, pokemon_idx);

  menu.Add("Team Index", team_idx)
      .WithBounds(0, 3)
      .WithRefresh()
      .Add("Pokemon Index", pokemon_idx)
      .WithBounds(0, 5)
      .WithRefresh()
      .Add("Pokemon Data", LoadBattlePokemonDataMenu)
      .Add("Pokemon Model", LoadBattlePokemonModelMenu);

  // static u8 trainer_model_id = 11;
  // static const c8* TRAINER_MODELS[] = {
  //     "Yvonne", "Xavier", "Malva", "Hologram", "May",
  //     "Brendan", "Wally", "Wally2", "Steven", "Maxie",
  //     "Archie", "Zinnia", "Shauna", "Tierno", "Trevor"};
  // .Add("Trainer Model", ctx.trainer_model_id)
  // .WithArray(TRAINER_MODELS, SIZE(TRAINER_MODELS))
}
} // namespace ui