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

#include "core/native/game_manager.h"
#include "overworld/native/character_placement.h"
#include "overworld/constant/map.h"

namespace savedata {
struct PokemonTeam;
class SaveData;
struct PlayTime;
} // namespace savedata

namespace overworld {
class ModelManager;
struct EncounterState;
struct MapData;
struct CommonResource;
struct WorldLayout;
} // namespace overworld

namespace core {
class DataManager {
  SINGLETON(DataManager)
public:
  STATIC_INLINE DataManager& GetInstance() {
    return GameManager::GetInstance().GetGameData();
  }

  INLINE savedata::SaveData& GetSavedata() const { return *savedata_; }
  INLINE savedata::PokemonTeam& GetPokemonTeam() const {
    return *pokemon_team_;
  }

  INLINE savedata::PlayTime& GetPlayTime() const { return *play_time_; }

  INLINE overworld::ModelManager& GetOverworldModelManager() const {
    return *overworld_model_manager_;
  }

  INLINE overworld::EncounterState& GetEncounterState() const {
    return *encounter_;
  }

  INLINE overworld::MapData& GetMapData() const { return *map_data_; }

  INLINE overworld::WorldLayout& GetWorldLayout() const {
    return *world_layout_;
  }

  INLINE overworld::CommonResource& GetCommonResource() const {
    return *common_resource_;
  }
  INLINE MapId GetPlayerZone() const { return player_zone_id_; }
  INLINE overworld::Facing& GetPlayerDirection() { return player_direction_; }

private:
  savedata::SaveData* savedata_;
  void* savedata_manager_;

  overworld::CommonResource* common_resource_;
  void* country_name_buffer;

  overworld::WorldLayout* world_layout_;

  savedata::PokemonTeam* pokemon_team_;
  savedata::PlayTime* play_time_;
  void* _1;
  overworld::ModelManager* overworld_model_manager_;
  void* _4;
  overworld::MapData* map_data_;
  void* area_data_;
  u32 season_;
  void* _5[3];
  u8 locations_[4 * 56];
  MapId player_zone_id_;
  overworld::Facing player_direction_;
  u8 _6;
  u8 _2[388 - 0x124];

  overworld::EncounterState* encounter_;

  u32 _3[10 + 4];
};
} // namespace core