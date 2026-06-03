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

#include "core/game_time_manager.h"
#include "data/move.h"
#include "data/pokemon.h"
#include "menu/plugin_menu.h"

namespace global_data {
void GlobalDataMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Global Pokemon Data", data::Pokemon::LoadMenu)
      .Add("Global Move Data", data::Move::LoadMenu)
      .Add("Global Time", GameTimeManager::LoadMenu)
      .Add("Date Time (ms)", *(s64*)ADDRESS_DATE_TIME);
}
} // namespace global_data