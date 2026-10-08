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

/**
 * @file common_resource.h
 * @brief The resources that all the maps share.
 */

#pragma once

#include "common.h"
#include "core/native/data_manager.h"

namespace overworld {
struct EncounterData;

/// The resources that all the maps share.
struct CommonResource {
  SINGLETON(CommonResource)
  STATIC_INLINE CommonResource& GetInstance() {
    return core::DataManager::GetInstance().GetCommonResource();
  }

  /// Returns the DexNav encounter table of a map.
  INLINE EncounterData& GetDexNavData(u16 map_id) {
    return *(EncounterData*)dex_nav_pack->GetResource(map_id);
  }

  void* graphics_buffer;
  Bundle* graphics_pack;

  void* dex_nav_buffer;
  Bundle* dex_nav_pack;

};
} // namespace overworld