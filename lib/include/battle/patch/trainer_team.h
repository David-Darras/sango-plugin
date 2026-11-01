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
 * @file trainer_team.h
 * @brief New teams for the trainers.
 *
 * @see docs/tutorials/08-change-trainer-teams.md
 */

#pragma once

#include <initializer_list>

#include "common.h"
#include "core/fixed_list.h"
#include "pokemon/constant/ability.h"
#include "battle/constant/ai.h"
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

/**
 * @brief One Pokémon of a trainer team.
 *
 * Give the species, then the other values with the With functions. A value
 * that you do not give keeps the value of the game.
 *
 * @code
 * battle::TrainerOpponent(SpeciesId::kRiolu)
 *     .WithLevel(12)
 *     .WithAbility(AbilityId::kInnerFocus)
 *     .WithNature(Nature::kAdamant)
 *     .WithEvs(0, 252, 0, 0, 4, 252)
 *     .WithMoves(MoveId::kQuickAttack, MoveId::kForcePalm)
 * @endcode
 */
struct TrainerOpponent {
  SpeciesId species;
  ItemId item;
  AbilityId ability;
  Nature nature;
  bool is_shiny;

  u8 ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense, ev_speed;

  MoveId move1, move2, move3, move4;

  FormId form;
  const c16* nickname;
  u8 forced_level; ///< The level. 0: the level of the game team.
  u8 iv; ///< The six IVs (0 to 31), or kGameIvs.

  /// The value of `iv` that keeps the IVs of the game.
  static constexpr u8 kGameIvs = 0xFF;

  TrainerOpponent()
    : species(SpeciesId::kNone), item(ItemId::kNone), ability(AbilityId::kNone),
      nature(Nature::kHardy), is_shiny(false), ev_hp(0), ev_attack(0),
      ev_defense(0), ev_sp_attack(0), ev_sp_defense(0), ev_speed(0),
      move1(MoveId::kNone), move2(MoveId::kNone), move3(MoveId::kNone),
      move4(MoveId::kNone), form(FormId::kNormal), nickname(nullptr),
      forced_level(0), iv(kGameIvs) {
  }

  /// A Pokémon of this species. Then use the With functions.
  explicit TrainerOpponent(SpeciesId species) : TrainerOpponent() {
    this->species = species;
  }

  TrainerOpponent(SpeciesId species, ItemId item, AbilityId ability, Nature nature,
                  bool is_shiny, u8 ev_hp, u8 ev_attack, u8 ev_defense,
                  u8 ev_sp_attack, u8 ev_sp_defense, u8 ev_speed, MoveId move1,
                  MoveId move2, MoveId move3, MoveId move4,
                  FormId form = FormId::kNormal, const c16* nickname = nullptr,
                  u8 forced_level = 0)
    : species(species), item(item), ability(ability), nature(nature),
      is_shiny(is_shiny), ev_hp(ev_hp), ev_attack(ev_attack),
      ev_defense(ev_defense), ev_sp_attack(ev_sp_attack),
      ev_sp_defense(ev_sp_defense), ev_speed(ev_speed), move1(move1),
      move2(move2), move3(move3), move4(move4), form(form),
      nickname(nickname), forced_level(forced_level), iv(kGameIvs) {
  }

  /// Sets the level (1 to 100).
  TrainerOpponent& WithLevel(u8 level) {
    forced_level = level;
    return *this;
  }
  /// Sets the held item.
  TrainerOpponent& WithItem(ItemId item) {
    this->item = item;
    return *this;
  }
  /// Sets the ability.
  TrainerOpponent& WithAbility(AbilityId ability) {
    this->ability = ability;
    return *this;
  }
  /// Sets the nature.
  TrainerOpponent& WithNature(Nature nature) {
    this->nature = nature;
    return *this;
  }
  /// Makes the Pokémon shiny.
  TrainerOpponent& WithShiny(bool is_shiny = true) {
    this->is_shiny = is_shiny;
    return *this;
  }
  /// Sets the six EVs, in the order of the summary: HP, Attack, Defense,
  /// Sp. Atk, Sp. Def, Speed.
  TrainerOpponent& WithEvs(u8 hp, u8 attack, u8 defense, u8 sp_attack,
                           u8 sp_defense, u8 speed) {
    ev_hp = hp;
    ev_attack = attack;
    ev_defense = defense;
    ev_sp_attack = sp_attack;
    ev_sp_defense = sp_defense;
    ev_speed = speed;
    return *this;
  }
  /// Sets the six IVs to the same value (0 to 31).
  TrainerOpponent& WithIvs(u8 value) {
    iv = value > 31 ? 31 : value;
    return *this;
  }
  /// Sets the moves. A move that you do not give is empty.
  TrainerOpponent& WithMoves(MoveId first, MoveId second = MoveId::kNone,
                             MoveId third = MoveId::kNone,
                             MoveId fourth = MoveId::kNone) {
    move1 = first;
    move2 = second;
    move3 = third;
    move4 = fourth;
    return *this;
  }
  /// Sets the form.
  TrainerOpponent& WithForm(FormId form) {
    this->form = form;
    return *this;
  }
  /// Sets the nickname. Use u"..." text. The text must stay in memory.
  TrainerOpponent& WithNickname(const c16* nickname) {
    this->nickname = nickname;
    return *this;
  }

  /// Writes this Pokémon into the data of a Pokémon.
  void ApplyTo(pokemon::CoreData& pkm) const;
};

/**
 * @brief A trainer team: the format, the place and up to six Pokémon.
 *
 * @code
 * const battle::TrainerTeam kRivalTeam =
 *     battle::TrainerTeam(Format::kSingle)
 *         .Add(battle::TrainerOpponent(SpeciesId::kPichu).WithLevel(7))
 *         .Add(battle::TrainerOpponent(SpeciesId::kRiolu).WithLevel(8));
 * @endcode
 */
struct TrainerTeam {
  /// The number of Pokémon of a team.
  static constexpr u32 kMaxOpponents = 6;

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

  /// @name The trainer
  /// The values that WithAi(), WithItems(), WithName() and WithMoney() set.
  /// @{
  bool has_ai;
  AiFlags ai;
  bool has_items;
  ItemId items[4];
  const c16* name;
  u8 money;
  /// @}

  static constexpr BackgroundId kDefaultBackground =
      GAME_CONSTANT(BackgroundId::kFlareBoss, BackgroundId::kAquaBoss);
  static constexpr GroundId kDefaultGround =
      GAME_CONSTANT(GroundId::kFlareBoss, GroundId::kAquaBoss);
  static constexpr EncounterAnimationId kDefaultEncounterAnimation =
      GAME_CONSTANT(EncounterAnimationId::kFlareBoss, EncounterAnimationId::kKyogre);

  /// An empty team that keeps the place of the game. Then use Add().
  explicit TrainerTeam(Format format = Format::kSingle)
    : opponent_count(0), format(format), background(kDefaultBackground),
      ground(kDefaultGround), platform(PlatformId::kWater),
      encounter_animation(kDefaultEncounterAnimation),
      weather(Weather::kInvalid), force_wild_battle_type(false),
      keeps_the_scene(true) {
    ClearTrainer();
  }

  TrainerTeam(u8 opponent_count, Format format,
              std::initializer_list<TrainerOpponent> opponent_list)
    : opponent_count(opponent_count), format(format),
      background(kDefaultBackground), ground(kDefaultGround),
      platform(PlatformId::kWater),
      encounter_animation(kDefaultEncounterAnimation),
      weather(Weather::kInvalid), force_wild_battle_type(false),
      keeps_the_scene(true) {
    ClearTrainer();
    u32 i = 0;
    for (const auto& opponent : opponent_list) {
      if (i >= kMaxOpponents) break;
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
    ClearTrainer();
    u32 i = 0;
    for (const auto& opponent : opponent_list) {
      if (i >= kMaxOpponents) break;
      opponents[i++] = opponent;
    }
  }

  /// Adds a Pokémon to the team (6 at most).
  TrainerTeam& Add(const TrainerOpponent& opponent) {
    if (opponent_count < kMaxOpponents) opponents[opponent_count++] = opponent;
    return *this;
  }
  /// Sets the place of the battle: the background, the ground and the
  /// platforms of the Pokémon. The encounter animation is optional.
  TrainerTeam& WithPlace(
      BackgroundId background, GroundId ground, PlatformId platform,
      EncounterAnimationId animation = kDefaultEncounterAnimation) {
    this->background = background;
    this->ground = ground;
    this->platform = platform;
    encounter_animation = animation;
    keeps_the_scene = false;
    return *this;
  }
  /// Sets the weather at the start of the battle.
  TrainerTeam& WithWeather(Weather weather) {
    this->weather = weather;
    return *this;
  }
  /// Sets the AI of the trainer, for example
  /// AiFlags::kCasual | AiFlags::kCompetitive | AiFlags::kStrategist.
  TrainerTeam& WithAi(AiFlags ai) {
    has_ai = true;
    this->ai = ai;
    return *this;
  }
  /// Sets the items that the trainer uses in the battle (4 at most).
  TrainerTeam& WithItems(ItemId first, ItemId second = ItemId::kNone,
                         ItemId third = ItemId::kNone,
                         ItemId fourth = ItemId::kNone) {
    has_items = true;
    items[0] = first;
    items[1] = second;
    items[2] = third;
    items[3] = fourth;
    return *this;
  }
  /// Sets the name of the trainer. Use u"..." text. It is translated (see
  /// core::Translations).
  TrainerTeam& WithName(const c16* name) {
    this->name = name;
    return *this;
  }
  /// Sets the prize money: the game multiplies this value by the level of
  /// the last Pokémon (4 for a Youngster, 100 or more for a boss).
  TrainerTeam& WithMoney(u8 money) {
    this->money = money;
    return *this;
  }

  /// Writes the team and the place into the settings of a battle.
  void ApplyTo(Config& config) const;

private:
  void ClearTrainer() {
    has_ai = false;
    ai = AiFlags::kNone;
    has_items = false;
    for (u32 i = 0; i < 4; i++) items[i] = ItemId::kNone;
    name = nullptr;
    money = 0;
  }
};

/// A trainer id and its team.
struct TrainerTeamEntry {
  TrainerId id;
  const TrainerTeam* team;
};

/// The list of the new trainer teams.
class TrainerTeams {
  MAKE_SINGLETON(TrainerTeams)

public:
  static constexpr u32 kMaxTeams = 256;

  /// Adds one team. 256 teams at most. The team must stay in memory.
  static bool Add(TrainerId id, const TrainerTeam* team);
  /// Sets a table of teams. The table must stay in memory.
  static void SetTable(const TrainerTeamEntry* table, u32 count);
  /// Removes all the teams.
  static void Clear();
  /// Removes the teams of Add() for which `is_removed(entry)` is true.
  static void Remove(bool (*is_removed)(const TrainerTeamEntry& entry));
  /// Returns the team of a trainer, or null.
  static const TrainerTeam* Find(TrainerId id);
  /// Replaces the team of the trainer, if it has a new team. The library calls
  /// it before Setup::on_trainer_battle (see Setup::apply_trainer_teams).
  static void Apply(Config& config, TrainerId trainer_id);

private:
  core::FixedList<TrainerTeamEntry, kMaxTeams> teams_;
  const TrainerTeamEntry* table_ = nullptr;
  u32 table_count_ = 0;
};
} // namespace battle
