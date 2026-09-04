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
#include "process.h"
#include "game/constant/ability.h"
#include "game/constant/form.h"
#include "game/constant/gender.h"
#include "game/constant/item.h"
#include "game/constant/move.h"
#include "game/constant/pokemon_type.h"
#include "game/constant/species.h"
#include "game/renderer/h3d_model.h"

class Device;

namespace game {
class Manager;
} // namespace game

struct PokemonCoreData;

namespace savedata {
class PokemonTeam;
struct PokemonParam;
} // namespace savedata

namespace overworld {
class StereoCamera;
} // namespace overworld

namespace battle {
struct Config;
class Manager;
class Graphics;

struct Pokemon {
  PokemonCoreData* core_data;
  u32 _0;
  u32 experience;
  Species species;
  u16 max_hp;
  u16 hp;
  ItemID item;
  ItemID used_item;
  // NOTE: left raw on purpose. It is 16 bits wide while `Ability` is a
  // u8 enum, and retyping it would mean changing the field width.
  u16 base_ability;
  u8 level;
  u8 uid;
  u8 _1[2];
  u32 status_condition[38];
  u8 status_condition_count[38];
  u8 _2[4 * 6 + 1];

  Species species_2;
  u16 attack;
  u16 defense;
  u16 special_attack;
  u16 special_defense;
  u16 speed;

  PokemonType types[3];
  Gender gender;

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
      MoveID id;
      u8 pp;
      u8 max_pp;
      u8 _0[2];
    } core, view;

    u8 _0;
  } moves[4];

  Ability ability;
  u8 padding;
  u16 weight;
  u8 move_count;
  Form form;
  u8 friendship;
  u8 _4[3];
  u16 turn_count;
  u8 _7[0xB6];

  void InverseStats() {
    u16 tmp = attack;
    attack = special_attack;
    special_attack = tmp;

    tmp = defense;
    defense = special_defense;
    special_defense = tmp;
  }

  void SetMetronome() {
    for (u32 j = 0; j < move_count; j++) {
      moves[j].core.id = MoveID::kMetronome;
      moves[j].core.pp = 99;
      moves[j].core.max_pp = 99;
      moves[j].view.id = MoveID::kMetronome;
      moves[j].view.pp = 99;
      moves[j].view.max_pp = 99;
    }
  }
};

struct Team {
  Pokemon* pokemon[6];
  u8 count;
  u8 _0[3];
};

class Manager {
  SINGLETON(Manager)
public:
  STATIC_INLINE Manager& GetInstance() {
    return Process::GetInstance().GetManager();
  }

  INLINE Graphics& GetGraphics() { return *graphics_; }

  static Pokemon* GetPokemon(bool is_server, u32 team_idx, u32 pkm_idx) {
    if (is_server) {
      return (GetInstance().server_.teams[team_idx].pokemon[pkm_idx]);
    }
    return (GetInstance().client_.teams[team_idx].pokemon[pkm_idx]);
  }

  static Team& GetTeam(bool is_server, u32 team_idx) {
    if (is_server) {
      return (GetInstance().server_.teams[team_idx]);
    }
    return (GetInstance().client_.teams[team_idx]);
  }

private:
  void* heaps_[4];
  Config* config_;
  Graphics* graphics_;

  u32 _0[8];
  game::Manager* game_manager_;
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

struct BaseModel {
  void* vtable;
  Vec3 position;
  Vec3 position_offset;
  Vec3 rotation;
  Vec3 rotation_offset;
  Vec3 scale;
  Vec3 scale_offset;
  bool update;
  bool _0[3];

  u32 _1[1 + 3 + 1 + 8 + 1 + 1 + 3 + 3 + 1 + 1 + 1 + 2 + 6 + 3 * 3];

  renderer::H3dModel* h3d_model;
};

struct Environnement {
  uptr vtable;
  uptr _0[11];
  renderer::H3dModel* ground;
  uptr _1[4 + 4 + 1];
  renderer::H3dModel* landscape;
  void* _2;
  renderer::H3dModel* unknow_0;
  void* _3[2];
  renderer::H3dModel* unknow_1;
};

class Graphics {
  SINGLETON(Graphics)
public:
  STATIC_INLINE Graphics& GetInstance() {
    return Manager::GetInstance().GetGraphics();
  }

  overworld::StereoCamera& GetStereoCamera() {
    return *(overworld::StereoCamera*)((uptr)this + 0x1F0);
  }

  BaseModel& GetPokemonModel(u32 index) const {
    if (index >= 6) index = 5;
    return *pokemon_model[index];
  }

  BaseModel& GetTrainerModel(u32 index) const {
    if (index >= 4) index = 3;
    return *trainer_model[index];
  }

  u8 _0[0x100];
  BaseModel* pokemon_model[6];
  BaseModel* trainer_model[4];
  Environnement* environnement;
  u32 _1;
  ResourcePack* platform_pack_0;
  ResourcePack* platform_pack_1;
  void* platform_resource[7];
  BaseModel* platform_model[6];
  void* big_platform_resource[2];
  BaseModel* big_platform_model[4];
  u32 _2;
  BaseModel* prop_model;
  BaseModel* unknow;
};
} // namespace battle