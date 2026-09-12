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
#include "game/constant/battle_background.h"
#include "game/constant/item.h"
#include "game/constant/battle_encounter_animation.h"
#include "game/constant/battle_format.h"
#include "game/constant/battle_ground.h"
#include "game/constant/battle_platform.h"
#include "game/constant/weather.h"
#include "game/savedata/pokemon_team.h"

namespace savedata {
struct PokemonTeam;
struct TrainerStatus;
} // namespace savedata

namespace battle {
struct TrainerAppearance {
  u32 type;
  u8 battle_model_sequence : 4;
  u8 sex : 2;
  u8 plural : 2;
  u8 group;
  u8 battle_effect_id;
  u8 model_3d;
  u32 cut_in_2d;
  u32 overworld_model;
  u32 encounter_bgm;
};

struct TrainerData {
  u64 local_friend_code;
  u32 id;
  u32 ai_flags;
  u8 battle_effect_id;
  u16 type;
  u8 group;
  u8 gender : 4;
  u8 is_plural_trainers : 4;
  u8 trainer_type_grammar;

  u8 money_calculation_coefficient;
  ItemId items[4];

  u32 message_archive_id;
  u16 win_string_id;
  u16 lose_string_id;

  String* name;
  String* title_name;
  TrainerAppearance* appearance;
};

struct Config {
  u8 battle_type; // wild, trainer
  u8 battle_format; // solo, double, triple, rotation, horde
  u16 _1;
  u16 _2;
  u8 weather;
  u8 __2;
  u8 background;
  bool use_skybox;
  u8 platform;
  u8 ground;
  u32 _3[9];
  u32 encounter_animation;
  u32 _4[2];
  u32 background_music;
  u32 _5[3];

  savedata::PokemonTeam* pokemon_teams[4];
  savedata::TrainerStatus* trainer_status[4];
  TrainerData* trainer_data[4];

  void* game_data_manager;
  void* item_manager;
  void* pokedex_manager;
  void* parameter_manager;

  u32 _6[7];
  u32 flags;
  f32 money_rate;

  bool is_sky_battle;
  bool is_inverse_battle;
  bool is_capture_forced;
  bool no_money;

  void Set(u8 count, BattleFormat format, BattleBackground background,
           BattleGround ground, BattlePlatform platform,
           BattleEncounterAnimation anim, BattleWeather weather) {
    this->pokemon_teams[1]->count = count;
    this->battle_format = static_cast<u8>(format);
    this->background = static_cast<u8>(background);
    this->ground = static_cast<u8>(ground);
    this->platform = static_cast<u8>(platform);
    this->encounter_animation = static_cast<u32>(anim);
    if (weather != BattleWeather::kInvalid)
      this->weather = static_cast<u8>(weather);
  }

  void InverseTeams() {
    savedata::PokemonTeam* team = pokemon_teams[0];
    pokemon_teams[0] = pokemon_teams[1];
    pokemon_teams[1] = team;
  }

  void InverseTypes() {
    is_inverse_battle = true;
  }

  PokemonCoreData& GetOpponent(u8 index) const {
    return *pokemon_teams[1]->pokemons[index]->core;
  }
};
} // namespace battle