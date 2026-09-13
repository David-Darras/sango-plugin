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

#include <initializer_list>

#include "common.h"
#include "pokemon/constant/ability.h"
#include "battle/constant/background.h"
#include "battle/constant/encounter_animation.h"
#include "battle/constant/format.h"
#include "battle/constant/ground.h"
#include "battle/constant/platform.h"
#include "battle/constant/trainer.h"
#include "battle/constant/weather.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/nature.h"
#include "pokemon/constant/species.h"

namespace pokemon {
struct CoreData;
}

namespace battle {
struct Config;

struct TrainerOpponent {
  SpeciesId species;
  ItemId item;
  AbilityId ability;
  Nature nature;
  bool is_shiny;

  u8 ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense, ev_speed;

  MoveId move1, move2, move3, move4;

  Form form;
  const c16* nickname;
  u8 forced_level;

  TrainerOpponent()
    : species(SpeciesId::kNone), item(ItemId::kNone), ability(AbilityId::kNone),
      nature(Nature::kHardy), is_shiny(false), ev_hp(0), ev_attack(0),
      ev_defense(0), ev_sp_attack(0), ev_sp_defense(0), ev_speed(0),
      move1(MoveId::kNone), move2(MoveId::kNone), move3(MoveId::kNone),
      move4(MoveId::kNone), form(Form::kNormal), nickname(nullptr),
      forced_level(0) {
  }

  TrainerOpponent(SpeciesId species, ItemId item, AbilityId ability, Nature nature,
                  bool is_shiny, u8 ev_hp, u8 ev_attack, u8 ev_defense,
                  u8 ev_sp_attack, u8 ev_sp_defense, u8 ev_speed, MoveId move1,
                  MoveId move2, MoveId move3, MoveId move4,
                  Form form = Form::kNormal, const c16* nickname = nullptr,
                  u8 forced_level = 0)
    : species(species), item(item), ability(ability), nature(nature),
      is_shiny(is_shiny), ev_hp(ev_hp), ev_attack(ev_attack),
      ev_defense(ev_defense), ev_sp_attack(ev_sp_attack),
      ev_sp_defense(ev_sp_defense), ev_speed(ev_speed), move1(move1),
      move2(move2), move3(move3), move4(move4), form(form),
      nickname(nickname), forced_level(forced_level) {
  }

  void ApplyTo(pokemon::CoreData& pkm) const;
};

struct TrainerTeam {
  u8 opponent_count;
  Format format;
  BackgroundId background;
  GroundId ground;
  PlatformId platform;
  EncounterAnimationId encounter_animation;
  Weather weather;
  bool force_wild_battle_type;
  bool keeps_the_scene;
  TrainerOpponent opponents[6];

  TrainerTeam(u8 opponent_count, Format format,
              std::initializer_list<TrainerOpponent> opponent_list)
    : opponent_count(opponent_count), format(format),
      background(BackgroundId::kAquaBoss), ground(GroundId::kAquaBoss),
      platform(PlatformId::kWater),
      encounter_animation(EncounterAnimationId::kKyogre),
      weather(Weather::kInvalid), force_wild_battle_type(false),
      keeps_the_scene(true) {
    u32 i = 0;
    for (const auto& opponent : opponent_list) {
      opponents[i++] = opponent;
    }
  }

  TrainerTeam(u8 opponent_count, Format format,
              BackgroundId background, GroundId ground,
              PlatformId platform,
              EncounterAnimationId encounter_animation,
              Weather weather,
              std::initializer_list<TrainerOpponent> opponent_list,
              bool force_wild_battle_type = false)
    : opponent_count(opponent_count), format(format), background(background),
      ground(ground), platform(platform),
      encounter_animation(encounter_animation), weather(weather),
      force_wild_battle_type(force_wild_battle_type), keeps_the_scene(false) {
    u32 i = 0;
    for (const auto& opponent : opponent_list) {
      opponents[i++] = opponent;
    }
  }

  void ApplyTo(Config& config) const;
};

struct TrainerTeamEntry {
  TrainerId id;
  const TrainerTeam* team;
};

class TrainerTeams {
  MAKE_SINGLETON(TrainerTeams)

public:
  static constexpr u32 kMaxTeams = 16;

  static bool Add(TrainerId id, const TrainerTeam* team);
  static void SetTable(const TrainerTeamEntry* table, u32 count);
  static void Clear();
  static const TrainerTeam* Find(TrainerId id);
  static void Apply(Config& config, TrainerId trainer_id);

private:
  TrainerTeamEntry teams_[kMaxTeams];
  u32 count_ = 0;
  const TrainerTeamEntry* table_ = nullptr;
  u32 table_count_ = 0;
};
} // namespace battle
