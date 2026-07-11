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

#ifndef SANGO_PLUGIN_FEATURE_MAP_DATA_LOADER_H
#define SANGO_PLUGIN_FEATURE_MAP_DATA_LOADER_H
#include "common.h"
#include "hook_manager.h"
#include "game/constant/form.h"
#include "game/constant/species.h"
#include "game/overworld/common_resource.h"
#include "game/overworld/map_data.h"
#include "game/overworld/map_manager.h"
#include "ui/log_application.h"

namespace ui {
class LogApplication;
}

namespace feature {
class MapDataLoader {
  MAKE_SINGLETON(MapDataLoader)
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kLoadMapData, 0x003D9BD4,
                            (uptr)LoadMapData);
  }

  static bool LoadMapData(overworld::MapData* map_data) {
    bool result = HookManager::Call<bool>(HookID::kLoadMapData, map_data);
    if (result) {
      FixMapData(map_data);
    }
    return result;
  }

  static void FixMapData(overworld::MapData* map_data) {
    auto& data = map_data->GetEncounterData();
    for (u32 i = 0; i < overworld::EncounterData::POKEMON_COUNT; i++) {
      ui::LogApplication::Print(u"pkm=%u", data.pokemon[i].species);
      data.pokemon[i].species = SPECIES_VENUSAUR;
      data.pokemon[i].form = FORM_VENUSAUR_MEGA;
      data.pokemon[i].min_level = 1;
      data.pokemon[i].max_level = 3;
    }
    u16 map_id = overworld::MapManager::GetInstance().GetMapId();
    u16 count = 0;
    overworld::NaviDexData* navi_dex = overworld::CommonResource::GetInstance().
        GetNaviDexData(
            map_id, count);
    for (u32 i = 0; i < count; i++) {
      navi_dex[i].species = SPECIES_VENUSAUR;
      navi_dex[i].form = FORM_VENUSAUR_MEGA;
    }
  }
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_MAP_DATA_LOADER_H