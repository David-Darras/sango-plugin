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

#ifndef SANGO_PLUGIN_SAVEDATA_PLAY_TIME_H
#define SANGO_PLUGIN_SAVEDATA_PLAY_TIME_H
#include "menu/plugin_menu.h"
#include "core/game_data_manager.h"

namespace savedata {

struct PlayTime {
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  static PlayTime& GetInstance() {
    return GameDataManager::GetInstance().GetPlayTime();
  }

  void* vtable;
  u16 hour;
  u8 minute;
  u8 second;
  u32 save_flags;
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_PLAY_TIME_H
