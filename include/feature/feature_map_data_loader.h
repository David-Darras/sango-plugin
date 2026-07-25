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
#include "feature_nuzlocke.h"
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
      // Disable contact encounter
      auto& data = map_data->GetEncounterData();
      for (u32 i = 0; i < 14; i++) {
        data.rate[i] = 0;
      }
      for (u32 i = 0; i < 61; i++) {
        data.poke_info[i].species = 0;
      }
    }
    return result;
  }
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_MAP_DATA_LOADER_H