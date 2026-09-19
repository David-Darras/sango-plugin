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
#include "overworld/constant/map.h"
#include "overworld/constant/model.h"
#include "overworld/native/draw_model.h"
#include "overworld/native/model_resource.h"
#include "overworld/native/position.h"

namespace overworld {

struct Model {
  static constexpr u32 kSize = GAME_CONSTANT(0xAA8, 0xAB0);

  INLINE DrawModel& GetDrawModel() {
    return *((DrawModel * (*)(Model*))address::kGetDrawModel)(this);
  }

  INLINE ModelResource& GetObjCodeParam() {
    return *(ModelResource*)((uptr)this + GAME_CONSTANT(0x87C, 0x884));
  }

  INLINE bool IsUsed() const { return (flags[0] & 1) == 1; }

  void* vtable;
  u32 flags[2];

  u16 id; // 0xFF = player
  MapId map_id;
  ModelId model_id;
  u16 move_id;
  u16 _0[4];

  Vec3 facing_direction;
  Vec3 movement_direction;
  Vec3 prev_facing_direction;
  Vec3 prev_movement_direction;

  u16 _2[12];

  Position init_pos;
  Position prev_pos;
  Position map_pos;
  Position world_pos;
  Vec3 draw_pos;
  Vec3 draw_offset;
};

} // namespace overworld
