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

#ifndef SANGO_PLUGIN_FEATURE_TRAINER_TEAM_H
#define SANGO_PLUGIN_FEATURE_TRAINER_TEAM_H
#include <initializer_list>

#include "common.h"
#include "game/battle/config.h"
#include "game/constant/ability.h"
#include "game/constant/battle_background.h"
#include "game/constant/battle_encounter_animation.h"
#include "game/constant/battle_format.h"
#include "game/constant/battle_ground.h"
#include "game/constant/battle_platform.h"
#include "game/constant/battle_trainer.h"
#include "game/constant/form.h"
#include "game/constant/item.h"
#include "game/constant/move.h"
#include "game/constant/nature.h"
#include "game/constant/species.h"
#include "game/constant/weather.h"
#include "game/global_data/pokemon.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"

namespace feature {
struct TrainerOpponent {
  Species species;
  ItemId item;
  Ability ability;
  Nature nature;
  bool is_shiny;

  u8 ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense, ev_speed;

  MoveId move1, move2, move3, move4;

  u8 form;
  const c16* nickname;
  u8 forced_level;

  TrainerOpponent()
    : species(Species::kNone), item(ItemId::kNone), ability(Ability::kNone),
      nature(Nature::kHardy), is_shiny(false), ev_hp(0), ev_attack(0),
      ev_defense(0), ev_sp_attack(0), ev_sp_defense(0), ev_speed(0),
      move1(MoveId::kNone), move2(MoveId::kNone), move3(MoveId::kNone),
      move4(MoveId::kNone), form(0), nickname(nullptr), forced_level(0) {
  }

  TrainerOpponent(Species species, ItemId item, Ability ability, Nature nature,
                  bool is_shiny, u8 ev_hp, u8 ev_attack, u8 ev_defense,
                  u8 ev_sp_attack, u8 ev_sp_defense, u8 ev_speed, MoveId move1,
                  MoveId move2, MoveId move3, MoveId move4, u8 form = 0,
                  const c16* nickname = nullptr, u8 forced_level = 0)
    : species(species), item(item), ability(ability), nature(nature),
      is_shiny(is_shiny), ev_hp(ev_hp), ev_attack(ev_attack),
      ev_defense(ev_defense), ev_sp_attack(ev_sp_attack),
      ev_sp_defense(ev_sp_defense), ev_speed(ev_speed), move1(move1),
      move2(move2), move3(move3), move4(move4), form(form),
      nickname(nickname), forced_level(forced_level) {
  }

  void ApplyTo(PokemonCoreData& pkm) const {
    pkm.Set(species, item, ability, nature, is_shiny);
    if (form != 0) pkm.form = static_cast<Form>(form);
    pkm.SetStats(ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense,
                 ev_speed);
    pkm.SetMoves(move1, move2, move3, move4);
    if (nickname != nullptr) pkm.SetNickname(nickname);
    if (forced_level != 0) pkm.SetLevel(forced_level);
  }
};

struct TrainerTeam {
  u8 opponent_count;
  BattleFormat format;
  BattleBackground background;
  BattleGround ground;
  BattlePlatform platform;
  BattleEncounterAnimation encounter_animation;
  BattleWeather weather;
  bool force_wild_battle_type;
  bool keeps_the_scene;
  TrainerOpponent opponents[6];

  TrainerTeam(u8 opponent_count, BattleFormat format,
              std::initializer_list<TrainerOpponent> opponent_list)
    : opponent_count(opponent_count), format(format),
      background(BattleBackground::kAquaBoss), ground(BattleGround::kAquaBoss),
      platform(BattlePlatform::kWater),
      encounter_animation(BattleEncounterAnimation::kKyogre),
      weather(BattleWeather::kInvalid), force_wild_battle_type(false),
      keeps_the_scene(true) {
    u32 i = 0;
    for (const auto& opponent : opponent_list) {
      opponents[i++] = opponent;
    }
  }

  TrainerTeam(u8 opponent_count, BattleFormat format,
              BattleBackground background, BattleGround ground,
              BattlePlatform platform,
              BattleEncounterAnimation encounter_animation,
              BattleWeather weather,
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

  void ApplyTo(battle::Config& config) const {
    if (force_wild_battle_type) config.battle_type = 0;
    if (keeps_the_scene) {
      config.Set(opponent_count, format,
                 static_cast<BattleBackground>(config.background),
                 static_cast<BattleGround>(config.ground),
                 static_cast<BattlePlatform>(config.platform),
                 static_cast<BattleEncounterAnimation>(
                   config.encounter_animation),
                 BattleWeather::kInvalid);
    } else {
      config.Set(opponent_count, format, background, ground, platform,
                 encounter_animation, weather);
    }
    for (u32 i = 0; i < opponent_count; i++) {
      opponents[i].ApplyTo(config.GetOpponent(i));
    }
  }
};

struct TrainerTeamEntry {
  BattleTrainer id;
  const TrainerTeam* team;
};

class TrainerTeams {
  MAKE_SINGLETON(TrainerTeams)

public:
  static constexpr u32 kMaxTeams = 16;

  static bool Add(BattleTrainer id, const TrainerTeam* team) {
    auto& ctx = GetInstance();
    if (ctx.count_ >= kMaxTeams || team == nullptr) return false;
    ctx.teams_[ctx.count_].id = id;
    ctx.teams_[ctx.count_].team = team;
    ctx.count_++;
    return true;
  }

  static void SetTable(const TrainerTeamEntry* table, u32 count) {
    auto& ctx = GetInstance();
    ctx.table_ = table;
    ctx.table_count_ = count;
  }

  static void Clear() {
    auto& ctx = GetInstance();
    ctx.count_ = 0;
    ctx.table_ = nullptr;
    ctx.table_count_ = 0;
  }

  static const TrainerTeam* Find(BattleTrainer id) {
    auto& ctx = GetInstance();
    for (u32 i = 0; i < ctx.count_; i++) {
      if (ctx.teams_[i].id == id) return ctx.teams_[i].team;
    }
    for (u32 i = 0; i < ctx.table_count_; i++) {
      if (ctx.table_[i].id == id) return ctx.table_[i].team;
    }
    return nullptr;
  }

  static void Apply(battle::Config& config, u16 trainer_id) {
    const TrainerTeam* team = Find(static_cast<BattleTrainer>(trainer_id));
    if (team == nullptr) return;

    savedata::PokemonTeam* opponents = config.pokemon_teams[1];
    if (opponents == nullptr) return;

    u32 count = team->opponent_count;
    if (count > 6) count = 6;

    for (u32 i = 1; i < count; i++) {
      *opponents->pokemons[i]->core = *opponents->pokemons[0]->core;
      *opponents->pokemons[i]->runtime = *opponents->pokemons[0]->runtime;
    }

    for (u32 i = 0; i < count; i++) {
      opponents->pokemons[i]->accessor->Decrypt();
    }

    team->ApplyTo(config);

    for (u32 i = 0; i < count; i++) {
      opponents->pokemons[i]->core->ResetNickname();
      opponents->pokemons[i]->accessor->Encrypt();
    }
    for (u32 i = 0; i < count; i++) {
      opponents->pokemons[i]->UpdateRuntimeData();
    }
    opponents->HealAllPokemons();
  }

private:
  TrainerTeamEntry teams_[kMaxTeams];
  u32 count_ = 0;
  const TrainerTeamEntry* table_ = nullptr;
  u32 table_count_ = 0;
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_TRAINER_TEAM_H