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

#include "archive.h"
#include "common.h"
#include "game/constant/gender.h"

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

struct PokemonModel {
  INLINE Resource& GetModel() const {
    return *(Resource*)model->GetResource(0);
  }

  INLINE Resource& GetTexture() const {
    return *(Resource*)texture->GetResource(0);
  }

  void* vtable;
  u32 _0[123];
  ResourcePack* model;
  ResourcePack* face_animation;
  ResourcePack* texture;
};

struct SpeciesMetadata {
  enum Flag : u8 {
    kEnabled = 1,
    kHasFemale = 2,
    kHasFormChange = 4
  };

  u16 data_offset;
  u8 data_count;
  u8 flags;
};

struct SpeciesFlagMetadata {
  enum Flag : u8 {
    kShareAnimation = 1,
    kShareTexture = 2,
    kShareModel = 4
  };

  u8 flags;
  u8 share_from;
};

struct PokeModelTable {
  static constexpr u16 kSpeciesCount = 721;
  static constexpr u32 kInvalidPack = 0xFFFFFFFF;

  explicit PokeModelTable(void* buffer)
    : species_metadata_((SpeciesMetadata*)buffer),
      flags_((SpeciesFlagMetadata*)((uptr)buffer + (kSpeciesCount + 1) *
                                    sizeof(SpeciesMetadata))) {
  }

  INLINE bool IsValid() const { return species_metadata_ != nullptr; }

  INLINE const SpeciesMetadata* GetSpeciesMetadata(u16 species) const {
    if (species == 0 || species > kSpeciesCount) return nullptr;
    return &species_metadata_[species - 1];
  }

  INLINE const SpeciesFlagMetadata* GetSpeciesFlagMetadata(u16 species, u8 form,
    u8 gender) const {
    const s32 index = GetDataIndex(species, form, gender);
    return index < 0 ? nullptr : &flags_[index];
  }

  INLINE s32 GetDataIndex(u16 species, u8 form, u8 gender) const {
    const SpeciesMetadata* species_metadata = GetSpeciesMetadata(species);
    if (species_metadata == nullptr) return -1;

    u32 index = 0;
    if ((species_metadata->flags & SpeciesMetadata::kHasFormChange) != 0 && form
        > 0) {
      index = (species_metadata->flags & SpeciesMetadata::kHasFemale) != 0
                ? form + 1u
                : form;
    } else if (gender == static_cast<u8>(Gender::kFemale) &&
               (species_metadata->flags & SpeciesMetadata::kHasFemale) != 0) {
      index = 1;
    }
    if (index >= species_metadata->data_count) index = 0;
    return (s32)(species_metadata->data_offset + index);
  }

  INLINE u32 GetPackTop(u16 species, u8 form, u8 gender) const {
    const s32 index = GetDataIndex(species, form, gender);
    if (index < 0) return kInvalidPack;
    return (u32)index * POKEMON_FILE_SECTION_COUNT + POKEMON_PACK_FIRST_FILE;
  }

private:
  SpeciesMetadata* species_metadata_;
  SpeciesFlagMetadata* flags_;
};