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
#include "core/native/data_manager.h"
#include "overworld/constant/map.h"
#include "overworld/native/encounter_data.h"
#include "overworld/native/map_settings.h"

namespace overworld {

struct MapData {
  SINGLETON(MapData)
  STATIC_INLINE MapData& GetInstance() {
    return core::DataManager::GetInstance().GetMapData();
  }

  INLINE MapSettings& GetSettings() {
    return *(MapSettings*)bundle[choice]->GetResource(0);
  }

  INLINE EncounterData& GetEncounterData() {
    return *(EncounterData*)bundle[choice]->GetResource(3);
  }

  u32 _0;
  void* archive;
  void* _1;
  MapId map_id;
  MapId next_map_id;

  u8 buffer[2][0x4A58];
  Bundle* bundle[2];
  u8 buffer2[0x4A58];
  u8 choice;

  u16 map_name[64];
};
} // namespace overworld
