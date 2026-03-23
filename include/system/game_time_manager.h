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

#ifndef SANGO_PLUGIN_GAME_TIME_MANAGER_H_
#define SANGO_PLUGIN_GAME_TIME_MANAGER_H_

#include "common.h"
#include "game_manager.h"

struct GameTimeManager {
  static void LoadMenu(menu::PluginMenu& menu);

  static GameTimeManager& GetInstance() {
    return GameManager::GetInstance().GetGameTimeManager();
  }

  bool is_enabled;
  u64 last_tick;
  u64 accumulated_seconds;
  u64 first_tick;
  ///< Throttles calls to svcGetSystemTick()
  ///< (only once every 20 frames to reduce CPU usage).
  u32 frame_counter;
};

#endif  // SANGO_PLUGIN_GAME_TIME_MANAGER_H_