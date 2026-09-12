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

/// BCH = Binary CTR Hardware 3D. Top-level header of a compiled 3D model file
/// (models, materials, meshes, textures, skeletons and animations all
/// live inside one of its sections). Unlike Garc, sections are reached
/// through a fixed-index table instead of named/counted blocks.
struct Bch {
  enum SectionType {
    // Models, materials, meshes, textures, skeletons, animations
    kModelData = 0,
    // Strings referenced by kModelData (mesh/material/bone names, ...)
    kNameTable = 1,
    // Precompiled GPU command lists for material/rendering setup
    kGpuCommandList = 2,
    // Raw vertex and index buffers used by the meshes
    kVertexIndexData = 3,
    // Additional raw buffers, used when kVertexIndexData isn't enough
    kExtraBufferData = 4,
    // Pointers to patch from file offsets to real addresses at load time
    kPointerFixupTable = 5,
    // Sections actually stored in the file (have an entry in section_offset)
    kSectionCount = 6,

    // These two hold no data in the file: they only reserve room for
    // buffers the game allocates itself at load time. Don't call
    // GetSection() with them, only GetSectionSize().
    kRuntimeVertexBuffer = 6,
    kRuntimeCommandBuffer = 7,
    kSectionSizeCount = 8,
  };

  u32 signature; // Identifies the file: "BCH\0"
  u8 format_version_min; // Both must equal 0x21 for this game's files
  u8 format_version_max;
  u16 revision;
  u32 section_offset[kSectionCount];
  // Byte offset of each section, relative to this header
  u32 section_size[kSectionSizeCount]; // Byte size of each section
  u8 flags;
  u8 _0; // Padding
  u16 pointer_fixup_count; // Number of entries in kPointerFixupTable

  INLINE uptr GetSection(SectionType type) const {
    return (uptr)this + section_offset[type];
  }

  INLINE u32 GetSectionSize(SectionType type) const {
    return section_size[type];
  }
};