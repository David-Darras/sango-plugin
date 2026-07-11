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

#pragma once

#include "common.h"
#include "game/data_manager.h"

namespace overworld {
struct EncounterData {
  u8 rate[14];

  struct {
    u16 species : 11;
    u16 form : 5;
    u8 min_level;
    u8 max_level;
  } pokemon[12 + 12 + 3 + 5 + 5 + 3 + 3 + 3 + 15];

  static constexpr u32 POKEMON_COUNT = sizeof(pokemon) / sizeof(pokemon[0]);
};

struct MapData {
  SINGLETON(MapData)
  STATIC_INLINE MapData& GetInstance() {
    return game::DataManager::GetInstance().GetMapData();
  }

  INLINE EncounterData& GetEncounterData() {
    return *(EncounterData*)resource_pack[choice]->GetResource(3);
  }

  u32 _0;
  void* archive;
  void* _1;
  u16 map_id;
  u16 next_map_id;

  u8 buffer[2][0x4A58];
  ResourcePack* resource_pack[2];
  u8 buffer2[0x4A58];
  u8 choice;

  u16 map_name[64];
};
} // namespace overworld