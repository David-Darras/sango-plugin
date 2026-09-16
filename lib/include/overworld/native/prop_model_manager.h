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

#include "overworld/native/map_manager.h"
#include "overworld/native/prop_model.h"

namespace overworld {

struct PropModelManager {
  SINGLETON(PropModelManager)
  STATIC_INLINE PropModelManager& GetInstance() {
    return MapManager::GetInstance().GetPropModelManager();
  }

  u32 _0[9 + 16];
  u32 count;
  u32 _1[1 + 8];
  PropModel* prop_models;
  u32 capacity;
};
}
