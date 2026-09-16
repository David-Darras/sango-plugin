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

#include "core/types.h"
#include "overworld/constant/facing.h"

namespace overworld {

struct SignEvent {
  u16 id;
  u16 kind;
  Facing talk_facing;
  u8 _0;
  u16 position_kind;
  union {
    struct {
      s16 tile_x;
      s16 tile_z;
      u16 width;
      u16 depth;
      s32 height;
    } tiles;
    u8 raw[12];
  };
};

static_assert(sizeof(SignEvent) == 20,
              "SignEvent must match the game's event data layout");

} // namespace overworld
