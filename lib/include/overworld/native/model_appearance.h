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
#include "overworld/constant/model.h"
#include "pokemon/constant/gender.h"

namespace overworld {

struct ModelAppearance {
  ModelId model_id;
  u8 draw_kind;
  u8 draw_code;
  u16 skeleton_preset;
  u8 shadow_kind;
  u8 footprint_kind;
  u8 reflection_kind;
  Gender gender;
  u8 width;
  u8 depth;
  s8 offset_x;
  s8 offset_y;
  s8 offset_z;
  u8 uses_outfit;
  u8 keeps_outfit_in_memory;
  u8 outline_kind;
  u16 outfit_pattern;
  u32 model_id_2;
};

static_assert(sizeof(ModelAppearance) == 24,
              "ModelAppearance must match the game's model list layout");

} // namespace overworld
