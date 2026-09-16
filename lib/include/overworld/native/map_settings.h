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

struct MapSettings {
  u8 terrain_kind;
  u8 model_set_id;
  u16 area_id;
  u16 layout_id;
  u16 message_id;
  u32 background_music_id[4];
  u16 event_data_id;
  u16 group_id;
  u16 place_name_bits;
  u16 weather_bits;
  u16 map_change_bits;
  u16 _0;
  u16 camera_area_id;
  u16 unique_sequence_id;
  u32 flag_bits;
  s16 fly_x;
  s16 fly_y;
  s16 fly_z;
  s16 start_x;
  s16 start_y;
  s16 start_z;
};

static_assert(sizeof(MapSettings) == 56,
              "MapSettings must match the game's ZONEDATA layout");

} // namespace overworld
