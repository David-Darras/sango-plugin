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

#include "savedata/overworld_menu.h"
#include "menu/plugin_menu.h"

namespace savedata {
void OverworldMenu::LoadMenu(menu::PluginMenu& menu, void* args) {
  OverworldMenu& data = GetInstance();

  menu.Add("Is Pokemon List Visible", &data.flags, 0, 1)
      .Add("Is Pokedex Visible", &data.flags, 1, 1)
      .Add("Is Bag Visible", &data.flags, 2, 1)
      .Add("Is Trainer Card Visible", &data.flags, 3, 1)
      .Add("Is Save Visible", &data.flags, 4, 1)
      .Add("Is Options Visible", &data.flags, 5, 1)

      .Add("Pokemon List Position", &data.flags, 6, 3)
      .Add("Pokedex Position", &data.flags, 9, 3)
      .Add("Bag Position", &data.flags, 12, 3)
      .Add("Trainer Card Position", &data.flags, 15, 3)
      .Add("Save Position", &data.flags, 18, 3)
      .Add("Options Position", &data.flags, 21, 3);
}
} // namespace savedata