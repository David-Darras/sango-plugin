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

#include "savedata/play_time.h"
namespace savedata {

void PlayTime::LoadMenu(menu::PluginMenu& menu, void* args) {
  PlayTime& data = GetInstance();

  menu.Add("Hour", data.hour)
      .Add("Minute", data.minute)
      .Add("Second", data.second);
}

}  // namespace savedata