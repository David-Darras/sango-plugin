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

#include "core/types.h"

namespace savedata {
struct BagManager;
struct BattleBox;
struct BoxManager;
struct ItemManager;
struct Misc;
struct PokemonBox;
struct PokemonTeam;
struct TrainerStatus;
}

namespace core {

/// What the game's box app reads on start.
struct BoxAppInput {
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

} // namespace core
