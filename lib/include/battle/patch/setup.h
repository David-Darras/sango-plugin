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
 * @file setup.h
 * @brief Changes the battles before they start.
 *
 * @see docs/tutorials/08-change-trainer-teams.md
 */

#pragma once

#include <type_traits>

#include "common.h"
#include "core/event.h"
#include "battle/constant/background.h"
#include "battle/constant/encounter_animation.h"
#include "battle/constant/format.h"
#include "battle/constant/ground.h"
#include "battle/constant/platform.h"
#include "battle/constant/trainer.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/species.h"

namespace battle {
struct Config;

/// The settings of battle::Setup. When `is_enabled` is true, they replace
/// the settings of the wild battles.
struct SetupSettings {
  bool is_enabled = false; ///< true: the wild battles use these settings.
  Format format = Format::kSingle;
#ifdef GAME_XY
  BackgroundId background = BackgroundId::kXerneas;
  PlatformId platform = PlatformId::kXerneas;
  GroundId ground = GroundId::kXerneas;
  EncounterAnimationId encounter_animation = EncounterAnimationId::kPokeX;
  SpeciesId species = SpeciesId::kXerneas;
  FormId form = FormId::kNormal;
#else
  BackgroundId background = BackgroundId::kAquaBoss;
  PlatformId platform = PlatformId::kAquaBoss;
  GroundId ground = GroundId::kAquaBoss;
  EncounterAnimationId encounter_animation = EncounterAnimationId::kKyogre;
  SpeciesId species = SpeciesId::kKyogre;
  FormId form = FormId::kKyogreAlpha;
#endif
  u32 background_music = (1 << 16) + 79;
  f32 money_rate = 1.0f;
  u32 flags = 0;
  bool use_skybox = true;
  bool is_sky_battle = false;
  bool is_inverse_battle = false;
  bool is_capture_forced = false;
  bool no_money = false;
  bool inverse_teams = false;
  TrainerId trainer_id = TrainerId::kNone;
};
static_assert(std::is_standard_layout<SetupSettings>::value,
              "SetupSettings must have standard layout");

/// Changes the battles before they start.
struct Setup : public SetupSettings {
  MAKE_SINGLETON(Setup)
  /// true: the teams of battle::TrainerTeams replace the teams of the game,
  /// before on_trainer_battle. A product that applies the teams itself gives
  /// false.
  bool apply_trainer_teams = true;
  /// Called after the game prepares a trainer battle, and after the teams of
  /// battle::TrainerTeams. The functions can change the trainer and the team.
  typedef void (*TrainerBattleCallback)(Config& config, TrainerId& trainer_id);
  core::Event<void(Config&, TrainerId&)> on_trainer_battle;
  /// Called after the game prepares a wild battle. The callback can change the
  /// Pokémon of the second team (the wild Pokémon).
  typedef void (*WildBattleCallback)(Config& config);
  core::Event<void(Config&)> on_wild_battle;

  static void Initialize();

private:
  static void SetupTrainerHook(Config* config, void* game_manager,
                               TrainerId trainer_id, void* p1,
                               Format format, void* p2);
  static void SetupWildHook(Config* config, void* game_manager,
                            void* opponent_team, void* p1, Format format,
                            u32 effect_id, void* p2);
};
} // namespace battle
