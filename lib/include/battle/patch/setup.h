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

#include <type_traits>

#include "common.h"
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

/// What the wild battle setup hook overwrites when `is_enabled` is set.
struct SetupSettings {
  bool is_enabled = false;
  Format format = Format::kSingle;
  BackgroundId background = BackgroundId::kAquaBoss;
  PlatformId platform = PlatformId::kAquaBoss;
  GroundId ground = GroundId::kAquaBoss;
  EncounterAnimationId encounter_animation = EncounterAnimationId::kKyogre;
  SpeciesId species = SpeciesId::kKyogre;
  FormId form = FormId::kKyogreAlpha;
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

struct Setup : public SetupSettings {
  MAKE_SINGLETON(Setup)
  /// Called once the game has set a trainer battle up; the trainer can
  /// still be swapped for another one.
  typedef void (*TrainerBattleCallback)(Config& config, TrainerId& trainer_id);
  TrainerBattleCallback on_trainer_battle = nullptr;

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
