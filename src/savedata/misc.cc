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

#include "savedata/misc.h"

#include "menu/plugin_menu.h"

namespace savedata {

void Misc::LoadMenu(menu::PluginMenu& menu) {
  Misc& data = GetInstance();
  menu.Add("Money", data.money)
      .Add("Battle Points", data.battle_points)
      .Add("Rival Nickname", data.rival_nickname, kNicknameLength)
      .Add("Badge 1", &data.badges, 0, 1)
      .Add("Badge 2", &data.badges, 1, 1)
      .Add("Badge 3", &data.badges, 2, 1)
      .Add("Badge 4", &data.badges, 3, 1)
      .Add("Badge 5", &data.badges, 4, 1)
      .Add("Badge 6", &data.badges, 5, 1)
      .Add("Badge 7", &data.badges, 6, 1)
      .Add("Badge 8", &data.badges, 7, 1)
      .Add("Unlock Pokémon League Wallpapers", &data.flags, 0, 1)
      .Add("Keyboard Layout", &data.flags, 2, 1)
      .Add("Exp. Share Enabled", &data.flags, 3, 1)
      .Add("PSS Tutorial Seen", &data.flags, 5, 1)
      .Add("Pokemon-Amie Tutorial Seen", &data.flags, 6, 1)
      .Add("Super Training Tutorial Seen", &data.flags, 7, 1)
      .Add("Vs. Recorder Tutorial Seen", &data.flags, 9, 1)
      .Add("Skip Long Sky Trip Animation", &data.flags, 11, 1)
      .Add("TV Navi Tutorial Seen", &data.flags, 14, 1);
}

}  // namespace savedata
