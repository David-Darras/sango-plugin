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
 * @file bch.h
 * @brief The BCH format: a compiled 3D model file.
 */

#pragma once

#include "common.h"

namespace renderer {
/**
 * @brief The header of a BCH file (Binary CTR Hardware 3D).
 *
 * A BCH file contains the models, the materials, the meshes, the textures,
 * the skeletons and the animations. A fixed table of sections gives their
 * positions. (A GARC file uses named blocks instead.)
 */
struct Bch {
  /// A section of the file.
  enum SectionType : u32 {
    /// The models, materials, meshes, textures, skeletons and animations.
    kModelData = 0,
    /// The names that kModelData uses (meshes, materials, bones...).
    kNameTable = 1,
    /// The GPU command lists of the materials.
    kGpuCommandList = 2,
    /// The vertex buffers and the index buffers of the meshes.
    kVertexIndexData = 3,
    /// More buffers, when kVertexIndexData is not sufficient.
    kExtraBufferData = 4,
    /// The pointers that the game changes from file offsets to addresses.
    kPointerFixupTable = 5,
    /// The number of sections in the file (each has a section_offset).
    kSectionCount = 6,

    /// A section without data in the file: the game makes this buffer when
    /// it loads the file. Use GetSectionSize() only, never GetSection().
    kRuntimeVertexBuffer = 6,
    /// A section without data in the file. See kRuntimeVertexBuffer.
    kRuntimeCommandBuffer = 7,
    /// The number of section sizes.
    kSectionSizeCount = 8,
  };

  u32 signature; ///< The signature of the file: "BCH\0".
  u8 format_version_min; ///< 0x21 for the files of this game.
  u8 format_version_max; ///< 0x21 for the files of this game.
  u16 revision;
  /// The offset of each section in bytes, from the start of this header.
  u32 section_offset[kSectionCount];
  u32 section_size[kSectionSizeCount]; ///< The size of each section in bytes.
  u8 flags;
  u8 _0;
  /// The number of entries in kPointerFixupTable.
  u16 pointer_fixup_count;

  /// Returns the address of a section.
  INLINE uptr GetSection(SectionType type) const {
    return (uptr)this + section_offset[type];
  }

  /// Returns the size of a section in bytes.
  INLINE u32 GetSectionSize(SectionType type) const {
    return section_size[type];
  }
};
} // namespace renderer
