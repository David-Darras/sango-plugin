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

namespace overworld {

struct TriggerEvent {
  u16 id;
  u16 param;
  u16 work_id;
  u16 check_kind;
  u32 position_kind;
  union {
    struct {
      u16 tile_x;
      u16 tile_z;
      u16 width;
      u16 depth;
      s16 height;
    } tiles;
    u8 raw[12];
  };
};

static_assert(sizeof(TriggerEvent) == 24,
              "TriggerEvent must match the game's event data layout");

} // namespace overworld
