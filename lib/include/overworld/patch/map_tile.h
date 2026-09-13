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
#include "overworld/native/tile.h"
#include <type_traits>

namespace overworld {

struct MapTileSettings {
  bool is_enabled = false;
  bool is_impassable = false;
  bool is_water = false;
  bool permits_encounters = false;
  bool allows_diagonal_movement = false;
  bool has_reflection_0 = false;
  bool has_shadow = false;
  bool blocks_trainer_movement = false;
  bool can_place_decorations = false;
  bool can_move_decoration_cursor = false;
  bool has_reflection_1 = false;
  bool has_reflection_2 = false;
  bool has_reflection_3 = false;
  u8 foot_step_id = 0;
  u8 battle_background_id = 0;
  u8 ground_id = 0;
};
static_assert(std::is_standard_layout<MapTileSettings>::value,
              "MapTileSettings must have standard layout");

struct MapTile : public MapTileSettings {
  MAKE_SINGLETON(MapTile)

  static void Initialize();

private:
  static Tile GetMapTileHook(void* data, Vec3* pos);
};

} // namespace overworld
