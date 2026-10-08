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

/**
 * @file model_resource.h
 * @brief The description of an overworld model.
 */

#pragma once

#include "core/types.h"
#include "overworld/constant/model.h"

namespace overworld {

/// The description of an overworld model. It has the same layout as ModelAppearance.
struct ModelResource {
  u16 code;
  u8 draw_type;
  u8 draw_code;
  u16 skeleton_preset;
  u8 shadow_kind;
  u8 footprint_kind;
  u8 reflection_kind;
  u8 gender;
  u8 width;
  u8 depth;
  s8 offset[3];
  u8 uses_outfit;
  u8 keeps_outfit_in_memory;
  u8 outline_kind;
  u16 outfit_pattern;
  ModelId model_id;
  u16 padding;
};
static_assert(sizeof(ModelResource) == 24, "ModelResource must have the size of the game structure");

} // namespace overworld
