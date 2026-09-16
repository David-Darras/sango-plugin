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

#include "battle/native/manager.h"
#include "battle/patch/battle.h"
#include "battle/patch/setup.h"
#include "battle/patch/type_chart.h"
#include "overworld/patch/camera.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"

namespace ui {
#include "battle/data/config.inc"

// --- Options ---------------------------------------------------------------

void LoadBattleSettingsPage(MainApplication& app, void* args) {
  auto& ctx = battle::Battle::GetInstance();

  app.Add("Can Use Items", ctx.can_use_item)
     .Add("Same Catch Rate For Every Ball", ctx.same_ratio_for_all_pokeball)
     .Add("Mega Restriction", ctx.mega_restriction)
     .Add("Unlimited Mega Evolutions", ctx.unlimited_mega_evolution)
     .AddSeparator()
     .Add("Long Mega Evolution Animation", ctx.is_long_mega_evolve_animation)
     .Add("Long Encounter Animation", ctx.is_long_encounter_animation)
     .Add("Show Enemy POV", ctx.show_enemy_pov)
     .Add("Show Trainer Animation", ctx.show_trainer_animation)
     .Add("Show Pokeball Animation", ctx.show_pokeball_animation)
     .Add("Show Fade In", ctx.show_fade_in)
     .Add("Show Shiny Animation", ctx.show_shiny_animation)
     .Add("No Shader", ctx.no_shader)
     .AddSeparator()
     .Add("Fix Pokemon Size", ctx.fix_pokemon_size)
     .Add("Sync Overworld Music", ctx.sync_overworld_music)
     .Add("Sync Team HP", ctx.sync_team_hp)
     .Add("Inverse Stats", ctx.inverse_stats)
     .Add("Metronome Only", ctx.metronome_only);
}

void LoadBattleSetupPage(MainApplication& app, void* args) {
  auto& ctx = battle::Setup::GetInstance();

  app.Add("Override Wild Battles", ctx.is_enabled)
     .Add("Inverse Teams", ctx.inverse_teams)
     .Add("Trainer Id", ctx.trainer_id)
     .AddSeparator()
     .Add("Format", ctx.format)
     .WithArray(FORMATS, SIZE(FORMATS))
     .Add("Background", ctx.background)
     .WithArray(BACKGROUNDS, SIZE(BACKGROUNDS))
     .Add("Ground", ctx.ground)
     .WithArray(GROUNDS, SIZE(GROUNDS))
     .Add("Platform", ctx.platform)
     .WithArray(PLATFORMS, SIZE(PLATFORMS))
     .Add("Encounter Animation", ctx.encounter_animation)
     .WithArray(ENCOUNTER_ANIMATIONS, SIZE(ENCOUNTER_ANIMATIONS))
     .Add("Use Skybox", ctx.use_skybox)
     .Add("Background Music", ctx.background_music)
     .AddSeparator()
     .AddSpecies("Species", ctx.species)
     .Add("Form", ctx.form)
     .AddSeparator()
     .Add("Long Animation", &ctx.flags, 16, 1)
     .Add("Is Deoxys Event", &ctx.flags, 19, 1)
     .Add("Is Sky Battle", ctx.is_sky_battle)
     .Add("Is Inverse Battle", ctx.is_inverse_battle)
     .Add("Is Capture Forced", ctx.is_capture_forced)
     .Add("No Money", ctx.no_money)
     .Add("Money Rate", ctx.money_rate)
     .WithFactor(0.1f);
}

namespace {
struct TypeChartEdit {
  TypeId attacking = TypeId::kNormal;
  TypeId defending = TypeId::kNormal;
  u8 multiplier = 2; ///< Index in kMultipliers
};

TypeChartEdit& GetTypeChartEdit() {
  static TypeChartEdit edit;
  return edit;
}

const battle::TypeChart::Multiplier kMultipliers[] = {
    battle::TypeChart::Multiplier::k0, battle::TypeChart::Multiplier::k05,
    battle::TypeChart::Multiplier::k1, battle::TypeChart::Multiplier::k2,
};

void ApplyTypeChart(void*) {
  auto& edit = GetTypeChartEdit();
  battle::TypeChart::Set(edit.attacking, edit.defending,
                         kMultipliers[edit.multiplier]);
}
} // namespace

void LoadTypeChartPage(MainApplication& app, void* args) {
  static const c8* MULTIPLIERS[] = {"x0", "x0.5", "x1", "x2"};

  auto& edit = GetTypeChartEdit();
  const auto current = battle::TypeChart::Get(edit.attacking, edit.defending);
  for (u32 i = 0; i < SIZE(kMultipliers); i++) {
    if (kMultipliers[i] == current) edit.multiplier = i;
  }

  const u32 last_type = static_cast<u32>(TypeId::kCount) - 1;
  app.AddType("Attacking Type", edit.attacking)
     .WithBounds(0, last_type)
     .WithRefresh()
     .AddType("Defending Type", edit.defending)
     .WithBounds(0, last_type)
     .WithRefresh()
     .AddSeparator()
     .Add("Multiplier (A to apply)", edit.multiplier)
     .WithArray(MULTIPLIERS, SIZE(MULTIPLIERS))
     .WithCallback(ApplyTypeChart);
}

// --- Live battle -----------------------------------------------------------

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

void LoadBattlePokemonDataPage(MainApplication& app, void* args) {
  if (app.CheckProcess(battle::address::kVtable)) return;

  auto& pkm = *pkm_server;

  app.Add("Save", SavePokemon)
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

void LoadBattlePokemonModelPage(MainApplication& app, void* args) {
  if (app.CheckProcess(battle::address::kVtable)) return;

  auto& model =
      battle::Manager::GetInstance().GetGraphics().GetPokemonModel(
          pokemon_idx);

  model.update = true;

  constexpr f32 kScaleFactor = 0.05f;
  constexpr f32 kRotationFactor = 4.0f * M_PI / 180.0f;
  constexpr f32 kPositionFactor = 4.0f;

  app.WithNoBackground()
     .Add("Position X", model.position.x)
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

void LoadBattleCameraPage(MainApplication& app, void* args) {
  if (app.CheckProcess(battle::address::kVtable)) return;

  static const c8* STATES[] = {"Idle", "Tps", "Rotate", "Top", "Fpv", "Free"};
  auto& ctx = overworld::Camera::GetInstance();

  app.WithNoBackground()
     .Add("State", ctx.battle_state)
     .WithArray(STATES, SIZE(STATES))
     .Add("Target Pokemon Slot", ctx.battle_target_pokemon_slot)
     .WithBounds(0, 5)
     .AddSeparator()
     .Add("Free Pos X (Left/Right)", ctx.pos.x)
     .WithFactor(5.0f)
     .Add("Free Pos Y (Up/Down)", ctx.pos.y)
     .WithFactor(5.0f)
     .Add("Free Pos Z (Forward/Back)", ctx.pos.z)
     .WithFactor(5.0f)
     .Add("Free Yaw (Turn)", ctx.rot.y)
     .WithFactor(0.05f)
     .Add("Free Pitch (Look)", ctx.rot.x)
     .WithFactor(0.05f)
     .AddSeparator()
     .Add("TPS Distance", ctx.tps_dist)
     .Add("TPS Height", ctx.tps_height)
     .Add("TPS Shoulder Offset", ctx.tps_offset)
     .AddSeparator()
     .Add("Orbit Radius", ctx.radius)
     .WithFactor(3.0f)
     .Add("Orbit Height", ctx.height)
     .WithFactor(3.0f)
     .Add("Orbit Rotation Speed", ctx.theta_speed)
     .WithFactor(0.01f);
}

void LoadBattleLivePage(MainApplication& app, void* args) {
  if (app.CheckProcess(battle::address::kVtable)) return;

  pkm_server = battle::Manager::GetPokemon(true, team_idx, pokemon_idx);
  pkm_client = battle::Manager::GetPokemon(false, team_idx, pokemon_idx);

  app.Add("Team Index", team_idx)
     .WithBounds(0, 3)
     .WithRefresh()
     .Add("Pokemon Index", pokemon_idx)
     .WithBounds(0, 5)
     .WithRefresh()
     .AddSeparator()
     .Add("Pokemon Data", LoadBattlePokemonDataPage)
     .Add("Pokemon Model", LoadBattlePokemonModelPage)
     .Add("Camera", LoadBattleCameraPage);
}

// --- Family root -----------------------------------------------------------

void LoadBattlePage(MainApplication& app, void* args) {
  app.Add("Settings", LoadBattleSettingsPage)
     .Add("Wild Battle Setup", LoadBattleSetupPage)
     .Add("Type Chart", LoadTypeChartPage)
     .AddSeparator()
     .Add("Live Battle", LoadBattleLivePage);
}
} // namespace ui
