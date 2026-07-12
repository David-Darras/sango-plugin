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
struct EncounterData;

struct CommonResource {
  SINGLETON(CommonResource)
  STATIC_INLINE CommonResource& GetInstance() {
    return game::DataManager::GetInstance().GetCommonResource();
  }

  INLINE EncounterData& GetNaviDexData(u16 map_id) {
    return *(EncounterData*)navi_dex_pack->GetResource(map_id);
  }

  void* graphics_buffer;
  ResourcePack* graphics_pack;

  void* navi_dex_buffer;
  ResourcePack* navi_dex_pack;

  // other data
};
} // namespace overworld