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
#include "battle/native/pokemon.h"
#include "battle/native/process.h"
#include "battle/native/team.h"

namespace sys {
class Device;
}

namespace core {
class GameManager;
} // namespace core

namespace savedata {
struct PokemonTeam;
struct PokemonParam;
} // namespace savedata

namespace battle {
struct Pokemon;
struct Config;
class Graphics;

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

  INLINE void ResetMegaEvolutions() {
    *(u32*)has_mega_evolved_ = 0;
  }

private:
  void* heaps_[4];
  Config* config_;
  Graphics* graphics_;

  u32 _0[8];
  core::GameManager* game_manager_;
  sys::Device* device_;

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

} // namespace battle
