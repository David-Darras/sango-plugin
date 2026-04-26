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

#ifndef SANGO_PLUGIN_OVERWORLD_ENCOUNTER_H
#define SANGO_PLUGIN_OVERWORLD_ENCOUNTER_H

#include "core/core.h"
#include "core/game_data_manager.h"

namespace overworld {

struct Encounter {
  static void LoadMenu(menu::PluginMenu& menu, void* args);
  static Encounter& GetInstance() {
    return GameDataManager::GetInstance().GetEncounter();
  }

  u32 walk_count;
  u16 _0;
  u16 encounter_rate;
  u32 last_tile_type;
  u16 fishing_chain_count;
};

}  // namespace overworld

#endif  // SANGO_PLUGIN_OVERWORLD_ENCOUNTER_H
