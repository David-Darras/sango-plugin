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

#ifndef SANGO_PLUGIN_POKEMON_MODEL_H
#define SANGO_PLUGIN_POKEMON_MODEL_H

#include "core/core.h"

enum TextureFormat {
  rgba8 = 0,
  rgb8 = 1,
  rgba5551 = 2,
  rgb565 = 3,
  rgba4 = 4,
  la8 = 5,
  hilo8 = 6,
  l8 = 7,
  a8 = 8,
  la4 = 9,
  l4 = 10,
  a4 = 11,
  etc1 = 12,
  etc1a4 = 13
};

struct RadixNode {
  u32 bit_index;
  u16 left_child_index;
  u16 right_child_index;
  const char* key;
};

struct RadixMap {
  void* elements;
  u16 count;
  u16 _0;
  RadixNode* root;
};

struct Content {
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

struct Section {
  u32 content;
  u32 string;
  u32 command;
  u32 data;
  u32 data_ex;
  u32 relocate;
};

struct Resource {
  u32 signature; // "BCH\0" (0x484342)

  u8 backward_compatibility;
  u8 forward_compatibility;
  u16 version;

  Section offset;
  Section size;

  u16 _0;
  u16 count;

  INLINE bool IsValid() const {
    return signature == 0x484342;
  }

  INLINE Content& GetContent() {
    return *(Content*)((uptr)this + offset.content);
  }

  INLINE u32* GetGpuCommands(u32 tex_idx, u32& cmd_count) {
    u32* elements = (u32*)GetContent().textures.elements;
    u32* cmd = (u32*)elements[tex_idx];
    cmd_count = cmd[1];
    return (u32*)cmd[0];
  }
};

struct ResourcePack {
  u16 signature; // "PC"
  u16 file_count;
  u32 file_offset[];

  u32 GetSize(u32 idx) const {
    u32 safe_idx = (idx >= file_count) ? 0 : idx;
    return file_offset[safe_idx + 1] - file_offset[safe_idx];
  }

  Resource& GetResource(u32 idx) {
    u32 safe_idx = (idx >= file_count) ? 0 : idx;
    return *(Resource*)((uptr)this + file_offset[safe_idx]);
  }
};

struct PokemonModel {
  INLINE Resource& GetModel() const {
    return model->GetResource(0);
  }

  INLINE Resource& GetTexture() const {
    return texture->GetResource(0);
  }

  void* vtable;
  u32 _0[123];
  ResourcePack* model;
  ResourcePack* face_animation;
  ResourcePack* texture;
};

#endif // SANGO_PLUGIN_POKEMON_MODEL_H