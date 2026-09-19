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

#include "core/game.h"
#include "core/math.h"
#include "core/types.h"

namespace renderer {
struct H3dModel;
}

namespace battle {

struct BaseModel {
  void* vtable;
  Vec3 position;
  Vec3 position_offset;
  Vec3 rotation;
  Vec3 rotation_offset;
  Vec3 scale;
  Vec3 scale_offset;
  bool update;
  bool _0[3];

  u32 _1[GAME_CONSTANT(21, 41)];

  renderer::H3dModel* h3d_model;
};

} // namespace battle
