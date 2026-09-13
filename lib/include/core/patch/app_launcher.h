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
#include "core/constant/app_id.h"
#include "overworld/constant/map.h"
#include "pokemon/constant/move.h"

namespace core {
class GameManager;
}

namespace savedata {
struct PokemonParam;
struct BoxManager;
struct PokemonBox;
struct BattleBox;
struct PokemonTeam;
struct TrainerStatus;
struct ItemManager;
struct BagManager;
struct Misc;
}

namespace core {

/// Opens one of the game's applications from the overworld menu, filling in
/// the input the game expects for it.
class AppLauncher {
  MAKE_SINGLETON(AppLauncher)

public:
  struct MoveInput {
    savedata::PokemonParam* pokemon;
    MoveId move_id;
    bool delete_move;
    u8 move_index;
  };

  struct BoxInput {
    savedata::BoxManager* box_manager;
    savedata::PokemonBox* pokemon_box;
    savedata::BattleBox* battle_box;
    savedata::PokemonTeam* team;
    savedata::TrainerStatus* trainer_status;
    savedata::ItemManager* item_manager;
    savedata::BagManager* bag_manager;
    void* bag_data;
    savedata::Misc* misc;

    u32 mode;

    u16 trade_species;
    u8 trade_sex;
    u8 _0;
    u32 trade_level;

    u16 exit_type;
    u8 box_index;
    u8 slot_index;
  };

  struct TownMapInput {
    bool is_fly_mode;
    u8 _0[0x20 - 1];
    u8 result;
    u8 _1;
    MapId map_id;
    u16 pokemon_index;
  };

  static void Initialize();
  void TriggerApp(AppId id);
  static void DoFly();

private:
  static bool CheckAppRequestHook(uptr menu, u32 id);
  static void MoveDeleterCallback(uptr* data, GameManager* manager);
  static void MoveTutorCallback(uptr* data, GameManager* manager);
  static void TownMapCallback(uptr* data, GameManager* manager);
  static void CallAppHook(uptr self, GameManager* manager);

  bool open_app = false;
  AppId app_id = AppId::kBox;
};
} // namespace core
