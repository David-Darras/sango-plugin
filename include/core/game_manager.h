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

#ifndef SANGO_PLUGIN_GAME_MANAGER_H
#define SANGO_PLUGIN_GAME_MANAGER_H

#include "core/core.h"

class GameEventManager;
class GameProcessManager;
class GameDataManager;
class GameTimeManager;
class PssManager;

namespace overworld {
class MapManager;
class WeatherManager;
} // namespace overworld

/**
 * @brief The primary singleton controller for the game engine.
 * Acts as the central hub for memory heaps, frame timing, and
 * high-level managers (Process and Event).
 */
class GameManager {
public:
  static FORCE_INLINE GameManager& GetInstance() {
    return *(GameManager*)ADDRESS_GAME_MANAGER;
  }

  FORCE_INLINE GameProcessManager& GetGameProcessManager() const {
    return *game_process_manager_;
  }

  FORCE_INLINE GameEventManager& GetGameEventManager() const {
    return *game_event_manager_;
  }

  FORCE_INLINE GameDataManager& GetGameData() const { return *game_data_; }

  FORCE_INLINE GameTimeManager& GetGameTimeManager() const {
    return *game_time_manager_;
  }

  FORCE_INLINE overworld::WeatherManager& GetWeatherManager() const {
    return *weather_manager_;
  }

  FORCE_INLINE overworld::MapManager& GetOverworldMapManager() const {
    return *overworld_map_manager_;
  }

  FORCE_INLINE PssManager& GetPssManager() const {
    return *pss_manager_;
  }

  FORCE_INLINE void* GetSystemHeap() const {
    return system_heap_;
  }

private:
  GameManager() = default;

  // Memory Heaps
  void* system_heap_; ///< Main system memory heap.
  void* device_heap_; ///< GPU/Hardware-specific memory heap.
  void* process_cell_heap_; ///< Heap dedicated to process-cell allocations.

  u8 frame_mode_requested_;
  u8 frame_mode_;
  u8 frame_count_;
  u8 reserved_;
  u32 unknow0;

  // Sub-Managers
  GameProcessManager* game_process_manager_;
  GameEventManager* game_event_manager_;
  GameDataManager* game_data_;
  GameTimeManager* game_time_manager_;
  void* _0;
  void* _1;
  overworld::WeatherManager* weather_manager_;
  void* _2[2];
  PssManager* pss_manager_;
  void* _3[3];
  overworld::MapManager* overworld_map_manager_;
};

#endif  // SANGO_PLUGIN_GAME_MANAGER_H