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

#include "overworld/patch/map_data_loader.h"
#include "core/hook_manager.h"
#include "overworld/patch/map_graft.h"
#include "overworld/native/map_data.h"

namespace overworld {

void MapDataLoader::Initialize() {
  core::HookManager::Initialize(HookId::kLoadMapData, address::kLoadMapData,
                          (uptr)LoadMapData);
}

bool MapDataLoader::LoadMapData(MapData* map_data) {
  bool result = core::HookManager::Call<bool>(HookId::kLoadMapData, map_data);
  if (!result) return result;

  const MapId loaded_map_id = map_data->next_map_id != MapId::kNone
                                  ? map_data->next_map_id
                                  : map_data->map_id;
  s32 dx = 0;
  s32 dz = 0;
  if (MapGraft::GetTileOffset(loaded_map_id, dx, dz)) {
    MapGraft::OffsetSettings(map_data->GetSettings(), dx, dz);
  }

  if (!GetInstance().is_contact_enabled) {
    auto& data = map_data->GetEncounterData();
    for (u8& rate : data.rate) {
      rate = 0;
    }
    for (PokeInfoOnAction& info : data.poke_info) {
      info.species = SpeciesId::kNone;
    }
  }
  return result;
}

} // namespace overworld
