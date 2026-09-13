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
#include "battle/constant/background.h"
#include "battle/constant/encounter_animation.h"
#include "battle/constant/format.h"
#include "battle/constant/ground.h"
#include "battle/constant/platform.h"
#include "battle/constant/weather.h"
#include "battle/native/trainer_data.h"
#include "savedata/native/pokemon_team.h"

namespace savedata {
struct PokemonTeam;
struct TrainerStatus;
} // namespace savedata

namespace battle {
struct Config {
  bool is_against_trainer; // or wild
  Format format; // solo, double, triple, rotation, horde
  u16 _1;
  u16 _2;
  Weather weather;
  u8 _7;
  BackgroundId background;
  bool use_skybox;
  PlatformId platform;
  GroundId ground;
  u32 _3[9];
  // The game stores the animation in a 32-bit slot; the id itself fits in
  // the low byte, the three others stay zero.
  EncounterAnimationId encounter_animation;
  u8 _8[3];
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

  void Set(u8 count, Format format, BackgroundId background,
           GroundId ground, PlatformId platform,
           EncounterAnimationId anim, Weather weather) {
    this->pokemon_teams[1]->count = count;
    this->format = format;
    this->background = background;
    this->ground = ground;
    this->platform = platform;
    this->encounter_animation = anim;
    if (weather != Weather::kInvalid)
      this->weather = weather;
  }

  void InverseTeams() {
    savedata::PokemonTeam* team = pokemon_teams[0];
    pokemon_teams[0] = pokemon_teams[1];
    pokemon_teams[1] = team;
  }

  void InverseTypes() {
    is_inverse_battle = true;
  }

  pokemon::CoreData& GetOpponent(u8 index) const {
    return *pokemon_teams[1]->pokemons[index]->core;
  }
};
} // namespace battle
