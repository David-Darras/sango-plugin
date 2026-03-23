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

#include "core.h"

class GameEventManager;
class GameProcessManager;
class GameDataManager;
class GameTimeManager;
class WeatherManager;

/**
 * @brief The primary singleton controller for the game engine.
 * Acts as the central hub for memory heaps, frame timing, and
 * high-level managers (Process and Event).
 */
class GameManager {
 public:
  static GameManager& GetInstance() {
    return *(GameManager*)ADDRESS_GAME_MANAGER;
  }

  GameProcessManager& GetGameProcessManager() const {
    return *game_process_manager_;
  }

  GameEventManager& GetGameEventManager() const { return *game_event_manager_; }

  GameDataManager& GetGameData() const { return *game_data_; }

  GameTimeManager& GetGameTimeManager() const { return *game_time_manager_; }

  WeatherManager& GetWeatherManager() const { return *weather_manager_; }

 private:
  /** @brief Private constructor to enforce singleton pattern. */
  GameManager() = default;

  // Memory Heaps
  void* system_heap_;        ///< Main system memory heap.
  void* device_heap_;        ///< GPU/Hardware-specific memory heap.
  void* process_cell_heap_;  ///< Heap dedicated to process-cell allocations.

  // Frame Management
  u8 frame_mode_requested_;  ///< The frame mode requested for the next update.
  u8 frame_mode_;            ///< Current active frame mode.
  u8 frame_count_;           ///< Global frame counter.
  u8 reserved_;              ///< Padding for memory alignment.
  u32 unknow0;

  // Sub-Managers
  GameProcessManager*
      game_process_manager_;  ///< Pointer to the GameProcManager instance.
  GameEventManager*
      game_event_manager_;  ///< Pointer to the GameEventManager instance.
  GameDataManager*
      game_data_;  ///< Pointer to global game-specific data structures.
  GameTimeManager* game_time_manager_;

  void* unknow1;
  void* unknow2;

  WeatherManager* weather_manager_;
};

#endif  // SANGO_PLUGIN_GAME_MANAGER_H