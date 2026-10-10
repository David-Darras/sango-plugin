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

/**
 * @file page_battle.cc
 * @brief The menu pages of the Battle family.
 */

#include <math.h>

#include <cstring>

#include "battle/native/graphics.h"
#include "battle/native/manager.h"
#include "battle/native/team.h"
#include "battle/patch/battle.h"
#include "battle/patch/setup.h"
#include "battle/patch/type_chart.h"
#include "core/native/process_manager.h"
#include "overworld/address.h"
#include "overworld/data/static_encounter.inc"
#include "overworld/native/static_encounter.h"
#include "overworld/patch/camera.h"
#include "ui/free_camera_application.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
#include "battle/data/config.inc"
#ifdef GAME_ORAS
#include "battle/data/trainer.inc"
#endif

// The battle options.

void LoadBattleSettingsPage(MainApplication& app, void* args) {
  auto& ctx = battle::Battle::GetInstance();

  app.AddSection("Rules")
     .Add("Can Use Items", ctx.can_use_item)
     .WithDescription("Off: the player cannot use the items of the Bag in "
                      "battle.")
     .Add("Same Catch Rate For Every Ball", ctx.same_ratio_for_all_pokeball)
     .WithDescription("All the Poke Balls have the same catch rate, the "
                      "Master Ball too.")
     .Add("One Mega Evolution", ctx.mega_restriction)
     .WithDescription("On: the rule of the game, one Mega Evolution in each "
                      "battle. Off: no limit.")
     .Add("Unlimited Mega Evolutions", ctx.unlimited_mega_evolution)
     .WithDescription("When One Mega Evolution is Off: the game always "
                      "accepts a Mega Evolution.")
     .Add("Inverse Stats", ctx.inverse_stats)
     .WithDescription("Exchanges the physical stats and the special stats "
                      "of all the Pokemon.")
     .Add("Metronome Only", ctx.metronome_only)
     .WithDescription("All the moves of all the Pokemon are Metronome.")
     .Add("Shared Team HP", ctx.sync_team_hp)
     .WithDescription("When one Pokemon of the first team faints, all the "
                      "Pokemon of this team faint.")
     .AddSection("Display")
     .Add("Type Helper", ctx.show_type_helper)
     .WithDescription("Shows the effectiveness of each move against the "
                      "opponent.")
     .Add("Real Pokemon Sizes", ctx.fix_pokemon_size)
     .WithDescription("Shows the Pokemon with their real sizes.")
     .Add("Keep Overworld Music", ctx.sync_overworld_music)
     .WithDescription("The battle keeps the music of the overworld.")
     .Add("No Shader", ctx.no_shader)
     .WithDescription("Turns off the shaders of the battle.")
     .AddSection("Animations")
     .Add("Long Mega Evolution Animation", ctx.is_long_mega_evolve_animation)
     .Add("Long Encounter Animation", ctx.is_long_encounter_animation)
     .Add("Show Enemy POV", ctx.show_enemy_pov)
     .Add("Show Trainer Animation", ctx.show_trainer_animation)
     .Add("Show Poke Ball Animation", ctx.show_pokeball_animation)
     .Add("Show Fade In", ctx.show_fade_in)
     .Add("Show Shiny Animation", ctx.show_shiny_animation);
}

void LoadBattleSetupPage(MainApplication& app, void* args) {
  auto& ctx = battle::Setup::GetInstance();

  app.AddSection("Wild Battles")
     .Add("Override Wild Battles", ctx.is_enabled)
     .WithDescription("On: the wild battles use the Pokemon, the battlefield "
                      "and the rules of this page.")
     .Add("Inverse Teams", ctx.inverse_teams)
     .WithDescription("The player battles with the team of the opponent, "
                      "and the opponent with the team of the player.")
     .AddSpecies("Species", ctx.species)
     .Add("Form", ctx.form)
     .AddSection("Trainer Battles")
     .Add("Replace The Trainer", ctx.trainer_id)
#ifdef GAME_ORAS
     .WithArray(TRAINER_NAMES, SIZE(TRAINER_NAMES))
#endif
     .WithDescription("All the trainer battles use this trainer and its "
                      "team. None: the trainer of the game.")
     .AddSection("Battlefield")
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
     .AddSection("Rules")
     .Add("Long Animation", &ctx.flags, 16, 1)
     .Add("Is Deoxys Event", &ctx.flags, 19, 1)
     .Add("Is Sky Battle", ctx.is_sky_battle)
     .Add("Is Inverse Battle", ctx.is_inverse_battle)
     .WithDescription("The type chart is inverted: super effective moves "
                      "become not very effective.")
     .Add("Is Capture Forced", ctx.is_capture_forced)
     .Add("No Money", ctx.no_money)
     .Add("Money Rate", ctx.money_rate)
     .WithFactor(0.1f)
     .WithDescription("Multiplies the money that the player wins.");
}

namespace {
struct TypeChartEdit {
  TypeId attacking = TypeId::kNormal;
  TypeId defending = TypeId::kNormal;
  u8 multiplier = 2; ///< The index in kMultipliers.
  // The types when the page loaded: the multiplier is for these types.
  TypeId loaded_attacking = TypeId::kNormal;
  TypeId loaded_defending = TypeId::kNormal;
};

TypeChartEdit& GetTypeChartEdit() {
  static TypeChartEdit edit;
  return edit;
}

const battle::TypeMultiplier kMultipliers[] = {
    battle::TypeMultiplier::k0, battle::TypeMultiplier::k05,
    battle::TypeMultiplier::k1, battle::TypeMultiplier::k2,
};

void ApplyTypeChart(void*) {
  auto& edit = GetTypeChartEdit();
  // A change of a type selects a different pair: the page then loads the
  // multiplier of this pair.
  if (edit.attacking != edit.loaded_attacking ||
      edit.defending != edit.loaded_defending) {
    return;
  }
  battle::TypeChart::Set(edit.attacking, edit.defending,
                         kMultipliers[edit.multiplier]);
}
} // namespace

void LoadTypeChartPage(MainApplication& app, void* args) {
  static const c8* MULTIPLIERS[] = {"x0", "x0.5", "x1", "x2"};

  auto& edit = GetTypeChartEdit();
  edit.loaded_attacking = edit.attacking;
  edit.loaded_defending = edit.defending;
  const auto current = battle::TypeChart::Get(edit.attacking, edit.defending);
  for (u32 i = 0; i < SIZE(kMultipliers); i++) {
    if (kMultipliers[i] == current) edit.multiplier = i;
  }

  const u32 last_type = static_cast<u32>(TypeId::kCount) - 1;
  app.OnChange(ApplyTypeChart)
     .AddType("Attacking Type", edit.attacking)
     .WithBounds(0, last_type)
     .WithRefresh()
     .AddType("Defending Type", edit.defending)
     .WithBounds(0, last_type)
     .WithRefresh()
     .AddSeparator()
     .Add("Multiplier", edit.multiplier)
     .WithArray(MULTIPLIERS, SIZE(MULTIPLIERS))
     .WithDescription("The damage of a move of the attacking type against a "
                      "Pokemon of the defending type. It applies at once.");
}

#ifdef GAME_ORAS
static u32 static_encounter_id = 0;

static void StartStaticEncounter(void*) {
  if (!core::ProcessManager::IsOverworldActive()) return;
  ((void(*)(core::GameManager*, u32, u32, s32))
       overworld::address::kCallStaticEncounter)(
      &core::GameManager::GetInstance(), static_encounter_id, 0, -1);
}

void LoadStaticEncounterPage(MainApplication& app, void* args) {
  static const c8* SHINY[] = {"Random", "Shiny", "Not Shiny"};
  static const c8* GENDERS[] = {"Random", "Male", "Female"};
  static const c8* ABILITIES[] = {"Random", "First", "Second", "Hidden"};
  static const c8* KINDS[] = {"Normal",          "Visible Pokemon",
                              "Legendary",       "Legendary (Again)",
                              "Rescue",          "Legendary (No Loss)"};

  auto& encounter = overworld::StaticEncounter::GetInstance(
      static_cast<overworld::StaticEncounterId>(static_encounter_id));
  // The 16 bits after the held item: shiny, gender, ability and kind.
  void* bits = (void*)((uptr)&encounter + 4);

  app.AddSection("Encounter")
     .Add("Encounter", static_encounter_id)
     .WithArray(STATIC_ENCOUNTER_NAMES, SIZE(STATIC_ENCOUNTER_NAMES))
     .WithRefresh()
     .Add("Start The Battle", StartStaticEncounter)
     .WithDescription("Starts the battle of this encounter now. Use it in "
                      "the overworld.")
     .AddSection("Pokemon")
     .AddSpecies("Species", encounter.species)
     .Add("Form", encounter.form)
     .Add("Level", encounter.level)
     .WithBounds(1, 100)
     .AddItem("Held Item", encounter.item)
     .Add("Shiny", bits, 16, 2)
     .WithArray(SHINY, SIZE(SHINY))
     .Add("Gender", bits, 18, 2)
     .WithArray(GENDERS, SIZE(GENDERS))
     .Add("Ability", bits, 20, 3)
     .WithArray(ABILITIES, SIZE(ABILITIES))
     .Add("Kind", bits, 23, 3)
     .WithArray(KINDS, SIZE(KINDS))
     .WithDescription("Legendary: a special message and one battle only. "
                      "Again: the Pokemon stays after the battle.")
     .AddSection("Battlefield")
     .Add("Background", encounter.background)
     .WithArray(BACKGROUNDS, SIZE(BACKGROUNDS))
     .Add("Ground", encounter.ground)
     .WithArray(GROUNDS, SIZE(GROUNDS))
     .Add("Encounter Animation", encounter.animation)
     .WithArray(ENCOUNTER_ANIMATIONS, SIZE(ENCOUNTER_ANIMATIONS));
}
#endif

// The current battle.

static battle::Pokemon* pkm_server = nullptr;
static battle::Pokemon* pkm_client = nullptr;
static u8 team_idx = 0;
static u8 pokemon_idx = 0;

// The battle uses two copies of each Pokemon: the server copy (the rules)
// and the client copy (the screen). The page changes the server copy, then
// this function copies it into the client copy.
static void SavePokemon(void*) {
  if (pkm_server == nullptr || pkm_client == nullptr) return;
  for (u32 i = 0; i < 4; i++) {
    memcpy(&pkm_server->moves[i].core, &pkm_server->moves[i].view,
           sizeof(pkm_server->moves[i].view));
  }
  memcpy(pkm_client, pkm_server, sizeof(battle::Pokemon));
}

void LoadBattlePokemonDataPage(MainApplication& app, void* args) {
  if (app.CheckProcess(battle::address::kVtable)) return;
  if (pkm_server == nullptr) {
    app.Add("There is no Pokemon in this slot.");
    return;
  }

  auto& pkm = *pkm_server;

  app.OnChange(SavePokemon)
     .AddSection("Pokemon")
     .AddSpecies("Species", pkm.species)
     .Add("Form", pkm.form)
     .Add("Level", pkm.level)
     .WithBounds(1, 100)
     .Add("Gender", pkm.gender)
     .WithArray(kGenderNames, SIZE(kGenderNames))
     .AddAbility("Ability", pkm.ability)
     .AddItem("Held Item", pkm.item)
     .Add("Friendship", pkm.friendship)
     .Add("Experience", pkm.experience)
     .Add("Weight", pkm.weight)
     .WithDescription("The weight in tenths of a kilogram: 60 is 6.0 kg.")
     .AddSection("HP and Stats")
     .Add("HP", pkm.hp)
     .Add("Max HP", pkm.max_hp)
     .Add("Attack", pkm.attack)
     .Add("Defense", pkm.defense)
     .Add("Sp. Atk", pkm.special_attack)
     .Add("Sp. Def", pkm.special_defense)
     .Add("Speed", pkm.speed)
     .AddSection("Stat Stages")
     .Add("Attack Stage", pkm.stat_attack)
     .WithArray(kStatStageNames, SIZE(kStatStageNames))
     .Add("Defense Stage", pkm.stat_defense)
     .WithArray(kStatStageNames, SIZE(kStatStageNames))
     .Add("Sp. Atk Stage", pkm.stat_special_attack)
     .WithArray(kStatStageNames, SIZE(kStatStageNames))
     .Add("Sp. Def Stage", pkm.stat_special_defense)
     .WithArray(kStatStageNames, SIZE(kStatStageNames))
     .Add("Speed Stage", pkm.stat_speed)
     .WithArray(kStatStageNames, SIZE(kStatStageNames))
     .Add("Accuracy Stage", pkm.stat_accuracy)
     .WithArray(kStatStageNames, SIZE(kStatStageNames))
     .Add("Evasion Stage", pkm.stat_evasion)
     .WithArray(kStatStageNames, SIZE(kStatStageNames))
     .AddSection("Moves")
     .AddMove("Move 1", pkm.moves[0].view.id)
     .Add("PP 1", pkm.moves[0].view.pp)
     .AddMove("Move 2", pkm.moves[1].view.id)
     .Add("PP 2", pkm.moves[1].view.pp)
     .AddMove("Move 3", pkm.moves[2].view.id)
     .Add("PP 3", pkm.moves[2].view.pp)
     .AddMove("Move 4", pkm.moves[3].view.id)
     .Add("PP 4", pkm.moves[3].view.pp)
     .AddSection("Types")
     .AddType("Type 1", pkm.types[0])
     .AddType("Type 2", pkm.types[1])
     .AddType("Type 3", pkm.types[2])
     .WithDescription("The third type comes from a move, for example "
                      "Forest's Curse.")
     .AddSection("Effort Values")
     .Add("EV HP", pkm.ev_hp)
     .Add("EV Attack", pkm.ev_attack)
     .Add("EV Defense", pkm.ev_defense)
     .Add("EV Sp. Atk", pkm.ev_special_attack)
     .Add("EV Sp. Def", pkm.ev_special_defense)
     .Add("EV Speed", pkm.ev_speed)
     .Add("EV Total", pkm.ev_sum)
     .Add("Battle ID", pkm.uid)
     .WithReadOnly();
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
     .AddSection("Position")
     .Add("Position X", model.position.x)
     .WithFactor(kPositionFactor)
     .Add("Position Y", model.position.y)
     .WithFactor(kPositionFactor)
     .Add("Position Z", model.position.z)
     .WithFactor(kPositionFactor)
     .AddSection("Rotation")
     .Add("Rotation X", model.rotation.x)
     .WithFactor(kRotationFactor)
     .WithDescription("In radians. One step is 4 degrees.")
     .Add("Rotation Y", model.rotation.y)
     .WithFactor(kRotationFactor)
     .WithDescription("In radians. One step is 4 degrees.")
     .Add("Rotation Z", model.rotation.z)
     .WithFactor(kRotationFactor)
     .WithDescription("In radians. One step is 4 degrees.")
     .AddSection("Scale")
     .Add("Scale X", model.scale.x)
     .WithFactor(kScaleFactor)
     .Add("Scale Y", model.scale.y)
     .WithFactor(kScaleFactor)
     .Add("Scale Z", model.scale.z)
     .WithFactor(kScaleFactor);
}

static const c8* kCameraStates[] = {"Idle", "Third Person", "Rotate",
                                    "Top",  "First Person", "Free"};

void LoadBattleCameraPage(MainApplication& app, void* args) {
  if (app.CheckProcess(battle::address::kVtable)) return;

  auto& ctx = overworld::Camera::GetInstance();

  app.WithNoBackground()
     .Add("Free Camera", [](void*) { FreeCameraApplication::Open(); })
     .WithDescription("Moves the camera with the buttons. The bottom screen "
                      "shows the controls.")
     .Add("State", ctx.battle_state)
     .WithArray(kCameraStates, SIZE(kCameraStates))
     .Add("Target Pokemon Slot", ctx.battle_target_pokemon_slot)
     .WithBounds(0, 5)
     .AddSection("Free Position")
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
     .AddSection("Third Person")
     .Add("Distance", ctx.tps_dist)
     .Add("Height", ctx.tps_height)
     .Add("Shoulder Offset", ctx.tps_offset)
     .AddSection("Orbit")
     .Add("Orbit Radius", ctx.radius)
     .WithFactor(3.0f)
     .Add("Orbit Height", ctx.height)
     .WithFactor(3.0f)
     .Add("Orbit Rotation Speed", ctx.theta_speed)
     .WithFactor(0.01f);
}

void LoadBattleLivePage(MainApplication& app, void* args) {
  if (app.CheckProcess(battle::address::kVtable)) return;

  // The teams of the battle: the player, the opponent, then the partner and
  // the second opponent of a Multi Battle.
  static const c8* SIDES[] = {"Player", "Opponent", "Partner", "Opponent 2"};

  // The species of each slot of the team, for the icons.
  static u16 species[6];
  for (u32 i = 0; i < 6; i++) {
    const battle::Pokemon* pokemon =
        battle::Manager::GetPokemon(true, team_idx, i);
    species[i] = pokemon != nullptr ? static_cast<u16>(pokemon->species) : 0;
  }

  pkm_server = battle::Manager::GetPokemon(true, team_idx, pokemon_idx);
  pkm_client = battle::Manager::GetPokemon(false, team_idx, pokemon_idx);

  app.AddSection("Pokemon")
     .Add("Team", team_idx)
     .WithArray(SIDES, SIZE(SIDES))
     .WithRefresh()
     .Add("Slot", pokemon_idx)
     .WithArray(GetNumberedNames("Slot %u", 6), 6)
     .WithIcons(IconKind::kPokemon, species)
     .WithRefresh();
  if (pkm_server != nullptr) {
    app.AddSpecies("Selected", pkm_server->species)
       .WithReadOnly()
       .Add("Pokemon Data", LoadBattlePokemonDataPage)
       .WithDescription("Species, stats, stat stages and moves. The changes "
                        "apply at once.")
       .Add("Pokemon Model", LoadBattlePokemonModelPage)
       .WithDescription("The position, the rotation and the size of the "
                        "model.");
  } else {
    app.Add("There is no Pokemon in this slot.");
  }
  app.AddSection("View")
     .Add("Camera", LoadBattleCameraPage);
}

// The top page of the battle pages.

void LoadBattlePage(MainApplication& app, void* args) {
  app.AddSection("Current Battle")
     .Add("Live Battle", LoadBattleLivePage)
     .WithDescription("The Pokemon, the models and the camera of the "
                      "current battle.")
     .AddSection("Options")
     .Add("Settings", LoadBattleSettingsPage)
     .WithDescription("The rules, the display and the animations of all "
                      "the battles.")
     .Add("Type Chart", LoadTypeChartPage)
     .WithDescription("Changes the effectiveness of one type against "
                      "another type.")
     .AddSection("Start a Battle")
     .Add("Wild Battle Setup", LoadBattleSetupPage)
     .WithDescription("The Pokemon, the battlefield and the rules of the "
                      "next wild battles.")
#ifdef GAME_ORAS
     .Add("Static Encounters", LoadStaticEncounterPage)
     .WithDescription("The legendary Pokemon and the other fixed "
                      "encounters. You can start their battles.")
#endif
     ;
}
} // namespace ui
