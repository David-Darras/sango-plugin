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

#include "core.h"

namespace overworld {
struct Tile {
  u32 is_impassable : 1;
  u32 is_water : 1;
  u32 permits_encounters : 1;
  u32 allows_diagonal_movement : 1;
  u32 has_reflection_0 : 1;
  u32 has_shadow : 1;
  u32 blocks_trainer_movement : 1;
  u32 can_place_decorations : 1;
  u32 can_move_decoration_cursor : 1;
  u32 has_reflection_1 : 1;
  u32 has_reflection_2 : 1;
  u32 has_reflection_3 : 1;
  u32 foot_step_id : 5;
  u32 battle_background_id : 7;
  u32 ground_id : 8;
};
} // namespace overworld