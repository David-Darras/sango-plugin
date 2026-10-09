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
 * @file game_manager.h
 * @brief The main object of the game.
 */

#pragma once

#include "common.h"

namespace savedata {
class PssManager;
}

namespace overworld {
class MapManager;
class WeatherManager;
} // namespace overworld

namespace core {
class ProcessManager;
class EventManager;
struct TimeManager;
class DataManager;

/// The main object of the game. It owns the other managers.
class GameManager {
  SINGLETON(GameManager)

public:
  STATIC_INLINE GameManager& GetInstance() {
    return *(GameManager*)READ32(sys::address::kGameManager);
  }

  /// Returns the manager of the processes.
  INLINE ProcessManager& GetProcessManager() const {
    return *game_process_manager_;
  }

  /// Returns the manager of the game events.
  INLINE EventManager& GetGameEventManager() const {
    return *game_event_manager_;
  }

  /// Returns the main game data.
  INLINE DataManager& GetGameData() const { return *game_data_; }

  /// Returns the manager of the time.
  INLINE TimeManager& GetGameTimeManager() const {
    return *game_time_manager_;
  }

  /// Returns the manager of the overworld weather.
  INLINE overworld::WeatherManager& GetWeatherManager() const {
    return *weather_manager_;
  }

  /// Returns the manager of the overworld maps.
  INLINE overworld::MapManager& GetOverworldMapManager() const {
    return *overworld_map_manager_;
  }

  /// Returns the manager of the PSS.
  INLINE savedata::PssManager& GetPssManager() const {
    return *pss_manager_;
  }

  /// Returns the main memory heap of the game.
  INLINE void* GetSystemHeap() const {
    return system_heap_;
  }

private:
  // The memory heaps.
  void* system_heap_;
  void* device_heap_; ///< The memory heap for the GPU.
  void* process_cell_heap_;

  u8 frame_mode_requested_;
  u8 frame_mode_;
  u8 frame_count_;
  u8 reserved_;
  u32 unknow0;

  // The managers.
  ProcessManager* game_process_manager_;
  EventManager* game_event_manager_;
  DataManager* game_data_;
  TimeManager* game_time_manager_;
  void* _0;
  void* _1;
  overworld::WeatherManager* weather_manager_;
  void* _2[2];
  savedata::PssManager* pss_manager_;
  void* _3[3];
  overworld::MapManager* overworld_map_manager_;
};
} // namespace core