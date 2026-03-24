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
#include "savedata/box_manager.h"

#include "menu/plugin_menu.h"

namespace savedata {

void BoxManager::LoadMenu(menu::PluginMenu& menu, void* args) {
  static u8 index = 0;
  BoxManager& data = GetInstance();

  menu.Add("Box Index", index)
      .WithBounds(0, kMaxBoxes)
      .WithRefresh()
      .Add("Title", data.titles[index], kMaxTitleLength)
      .Add("Wallpaper", data.wallpapers[index])
      .WithBounds(0, kMaxWallpapers)
      .Add("Unlocked Boxes Count", data.unlocked_count)
      .Add("Active Box Index", data.active_box_index)
      .WithBounds(0, kMaxBoxes)
      .Add("Special Wallpapers Unlocked", &data.flags, 0, 7)
      .Add("Event Box Open", &data.flags, 7, 1);
}

}  // namespace savedata
