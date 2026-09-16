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
#include "overworld/constant/map.h"

namespace overworld {

struct WarpEvent {
  MapId map_id;
  u16 exit_id;
  Facing exit_facing;
  u8 exit_kind;
  u16 position_kind;
  union {
    struct {
      s16 x;
      s16 y;
      s16 z;
      u16 width;
      u16 depth;
    } world;
    u8 raw[16];
  };
};

static_assert(sizeof(WarpEvent) == 24,
              "WarpEvent must match the game's event data layout");

} // namespace overworld
