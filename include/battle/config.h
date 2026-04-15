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

#ifndef SANGO_PLUGIN_BATTLE_CONFIG_H
#define SANGO_PLUGIN_BATTLE_CONFIG_H

#include "common.h"

namespace savedata {
struct PokemonTeam;
struct TrainerStatus;
}  // namespace savedata

namespace battle {

struct Config {
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  u8 _0;
  u8 battle_format;
  u16 _1;
  u32 _2;
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
  void* trainer_data[4];

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
};

}  // namespace battle

#endif  // SANGO_PLUGIN_BATTLE_CONFIG_H
