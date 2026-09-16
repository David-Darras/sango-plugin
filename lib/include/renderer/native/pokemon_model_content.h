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
#include "renderer/native/archive/bch.h"
#include "renderer/native/radix_map.h"

namespace renderer {

/// The Bch::kModelData section of a Pokémon model: the layout of that
/// section is specific to Pokémon models, so it lives here rather than in
/// the generic archive/bch.h.
struct PokemonModelContent {
  RadixMap models;
  RadixMap materials;
  RadixMap shaders;
  RadixMap textures;
  RadixMap look_up_tables;
  RadixMap lights;
  RadixMap cameras;
  RadixMap fogs;
  RadixMap environments;
  RadixMap skeletal_animations;
  RadixMap material_animations;
  RadixMap visibility_animations;
  RadixMap light_animations;
  RadixMap camera_animations;
  RadixMap fog_animations;
};

INLINE PokemonModelContent& GetModelData(Bch& bch) {
  return *(PokemonModelContent*)bch.GetSection(Bch::kModelData);
}

INLINE u32* GetGpuCommands(Bch& bch, u32 tex_idx, u32& cmd_count) {
  u32* elements = (u32*)GetModelData(bch).textures.elements;
  u32* cmd = (u32*)elements[tex_idx];
  cmd_count = cmd[1];
  return (u32*)cmd[0];
}

} // namespace renderer
