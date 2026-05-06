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

#include "core/core.h"
#include "menu/plugin_menu.h"
#include "overworld/map_manager.h"

namespace overworld {

static u32 choice = 0;

void DoFieldMove(void*) {
  MapManager& map_manager = MapManager::GetInstance();

  struct {
    u16 zone_id;
    u16 team_index;
    MapManager* map_manager;
  } context = {map_manager.GetMapId(), 0, &map_manager};

  ((void (*)(void*, u32))ADDRESS_DO_FIELD_MOVE)(&context, choice);
}

void FieldMove_LoadMenu(menu::PluginMenu& menu, void* args) {
  static const c8* MOVES[] = {"Cut",         "Surf",       "Waterfall",
                              "Strength",    "Rock Smash", "Fly",
                              "Flash",       "Teleport",   "Dig",
                              "Sweet Scent", "Dive",       "Secret Power"};

  menu.Add("Choice", choice)
      .WithArray(MOVES, SIZE(MOVES))
      .Add("Execute", DoFieldMove);
}

}  // namespace overworld