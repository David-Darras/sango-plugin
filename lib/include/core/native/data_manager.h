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

/**
 * @file data_manager.h
 * @brief The object of the game that keeps the main game data.
 */

#pragma once

#include <cstddef>

#include "core/native/game_manager.h"
#include "overworld/constant/facing.h"
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
/// Gives access to the save data, the party and the overworld data.
class DataManager {
  SINGLETON(DataManager)
public:
  STATIC_INLINE DataManager& GetInstance() {
    return GameManager::GetInstance().GetGameData();
  }

  /// Returns the save data.
  INLINE savedata::SaveData& GetSavedata() const { return *savedata_; }
  /// Returns the party of the player.
  INLINE savedata::PokemonTeam& GetPokemonTeam() const {
    return *pokemon_team_;
  }

  /// Returns the play time.
  INLINE savedata::PlayTime& GetPlayTime() const { return *play_time_; }

  /// Returns the models of the overworld. Only in the overworld.
  INLINE overworld::ModelManager& GetOverworldModelManager() const {
    return *overworld_model_manager_;
  }

  /// Returns the models of the overworld, or null outside of the overworld.
  INLINE overworld::ModelManager* GetOverworldModelManagerOrNull() const {
    return overworld_model_manager_;
  }

  /// Returns the state of the wild encounters.
  INLINE overworld::EncounterState& GetEncounterState() const {
    // The members before this one have the sizes of the game.
    static_assert(offsetof(DataManager, player_zone_id_) == 0x124,
                  "place of the player");
    static_assert(offsetof(DataManager, encounter_) == GAME_CONSTANT(392, 388),
                  "place of the encounters");
    return *encounter_;
  }

  /// Returns the data of the current map.
  INLINE overworld::MapData& GetMapData() const { return *map_data_; }

  /// Returns the layout of the current world.
  INLINE overworld::WorldLayout& GetWorldLayout() const {
    return *world_layout_;
  }

  /// Returns the shared resources of the overworld.
  INLINE overworld::CommonResource& GetCommonResource() const {
    return *common_resource_;
  }
  /// Returns the map of the player.
  INLINE MapId GetPlayerZone() const { return player_zone_id_; }
  /// Returns the season.
  INLINE u8 GetSeason() const { return static_cast<u8>(season_); }
  /// Returns the direction of the player.
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
  /// The object of the player starts here, with the table of its functions.
  /// Its data follow: the map, then the direction.
  void* player_vtable_;
  MapId player_zone_id_;
  overworld::Facing player_direction_;
  u8 _6;
  u8 _2[GAME_CONSTANT(392, 388) - 0x128];

  overworld::EncounterState* encounter_;

  u32 _3[10 + 4];
};
} // namespace core
