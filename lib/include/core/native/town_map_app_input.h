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
#include "core/types.h"
#include "overworld/constant/map.h"

namespace core {

#ifdef GAME_XY
struct TownMapAppInput {
  u8 start_mode;
  u8 result;
  MapId map_id;
  static constexpr u32 kStartModeFly = 1;
  static constexpr u32 kResultFly = 2;
};
#else
struct TownMapAppInput {
  bool is_fly_mode;
  u8 _0[0x20 - 1];
  u8 result;
  u8 _1;
  MapId map_id;
  u16 pokemon_index;
};
#endif

} // namespace core
