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

#include "menu/plugin_menu.h"
#include "overworld/encounter.h"
#include "overworld/field_move.h"
#include "overworld/map_manager.h"
#include "overworld/model_manager.h"
#include "overworld/renderer.h"
#include "overworld/tile.h"
#include "overworld/weather_manager.h"
#include "overworld/day_care.h"

namespace overworld {
void OverworldMenu(menu::PluginMenu& menu, void* args) {
  MapManager& man = MapManager::GetInstance();
  menu.Add("Reload Map", man.GetMapId())
      .Add("Map Tile", Tile::LoadMenu)
      .Add("Camera", StereoCamera::LoadMenu)
      .Add("Model", ModelManager::LoadMenu)
      .Add("Encounter", Encounter::LoadMenu)
      .Add("Day Care", DayCareMenu);
  menu.AddSeparator();
  FieldMove_LoadMenu(menu, args);
}
}