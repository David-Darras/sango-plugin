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

#include "renderer.h"
#include "game/overworld/map_manager.h"

namespace renderer {
struct H3dShaderModel;
}

namespace overworld {
struct PropModelManager;

struct PropModel {
  bool is_initialized;
  bool _0[3];
  PropModelManager* manager;
  u32 index;
  u32 _1[2];
  u32 map_index;
  u16 index_2;
  u8 sound_effect;
  u8 _2;
  u32 _3[4];
  renderer::H3dShaderModel* shader;
  u32 _4[38];
  bool is_visible;
  bool _5[3];
  Vec3 _6, _7;
};

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