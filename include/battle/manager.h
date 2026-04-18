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

#ifndef SANGO_PLUGIN_BATTLE_MANAGER_H
#define SANGO_PLUGIN_BATTLE_MANAGER_H

#include "core/core.h"
#include "process.h"

class GameManager;

namespace savedata {
class PokemonTeam;
struct PokemonParam;
struct PokemonCoreData;
}  // namespace savedata

namespace battle {

struct Config;
class Manager;

struct Pokemon {
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  savedata::PokemonCoreData* core_data;
  u32 _0;
  u32 experience;
  u16 species;
  u16 max_hp;
  u16 hp;
  u16 item;
  u16 used_item;
  u16 base_ability;
  u8 level;
  u8 _1[3];
  u32 status_condition[38];
  u8 status_condition_count[38];
  u8 _2[4 * 6 + 1];

  u16 species_2;
  u16 attack;
  u16 defense;
  u16 special_attack;
  u16 special_defense;
  u16 speed;

  u8 types[3];
  u8 gender;

  u8 stat_attack;
  u8 stat_defense;
  u8 stat_special_attack;
  u8 stat_special_defense;
  u8 stat_speed;
  u8 stat_accuracy;
  u8 stat_evasion;

  u16 ev_sum;
  u8 ev_hp;
  u8 ev_attack;
  u8 ev_defense;
  u8 ev_speed;
  u8 ev_special_attack;
  u8 ev_special_defense;
  bool has_pokerus;
  bool _3;

  struct {
    struct {
      u16 id;
      u8 pp;
      u8 max_pp;
      u8 _0[2];
    } core, view;
    u8 _0;
  } moves[4];

  u8 ability;
  u8 padding;
  u16 weight;
  u8 move_count;
  u8 form;
  u8 friendship;
  u8 _4[3];
  u16 turn_count;
  u8 _7[0xB6];
};

struct Team {
  Pokemon* pokemon[6];
  u8 count;
  u8 _0[3];
};

class Manager {
 public:
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  static Pokemon* GetPokemon(bool is_server, u32 team_idx, u32 pkm_idx) {
    if (is_server) {
      return (Process::GetInstance()
                  .GetManager()
                  .server_.teams[team_idx]
                  .pokemon[pkm_idx]);
    }
    return (Process::GetInstance()
                .GetManager()
                .client_.teams[team_idx]
                .pokemon[pkm_idx]);
  }

 private:
  void* heaps_[4];
  Config* config_;
  void* battle_graphics_;

  u32 _0[8];
  GameManager* game_manager_;
  Device* device_;

  u32 _1[6];
  savedata::PokemonTeam* client_teams_[4];
  savedata::PokemonTeam* server_teams_[4];
  u32 _2[2];
  bool has_exited_[4];
  bool has_mega_evolved_[4];

  struct {
    Manager* manager;
    Team teams[4];
    savedata::PokemonTeam* savedata_teams[4];
    savedata::PokemonParam* pokemon_params[4 * 6];
    u8 _0[0x2BC];
  } client_, server_;
};

}  // namespace battle

#endif  // SANGO_PLUGIN_BATTLE_MANAGER_H
