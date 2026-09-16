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
#include "core/native/data_manager.h"
#include "overworld/constant/map.h"
#include "overworld/native/camera_scroll_area.h"

namespace overworld {

struct WorldLayout {
  static constexpr u32 kMaxWidth = 32;
  static constexpr u32 kMaxHeight = 32;
  static constexpr u32 kMaxBlocks = kMaxWidth * kMaxHeight;
  static constexpr u32 kCellsPerBlockSide = 4;
  static constexpr u32 kMaxCells = kMaxBlocks * kCellsPerBlockSide *
                                   kCellsPerBlockSide;
  static constexpr u32 kTilesPerBlockSide = 40;
  static constexpr u32 kTilesPerCellSide = kTilesPerBlockSide /
                                           kCellsPerBlockSide;
  static constexpr u32 kUnitsPerTile = 18;
  static constexpr u16 kNoBlock = 0xFFFF;
  static constexpr u16 kNoLod = 0xFFFF;

  STATIC_INLINE WorldLayout& GetInstance() {
    return core::DataManager::GetInstance().GetWorldLayout();
  }

  INLINE u32 GetCellWidth() const { return width * kCellsPerBlockSide; }
  INLINE u32 GetCellHeight() const { return height * kCellsPerBlockSide; }
  INLINE bool ContainsBlock(u32 x, u32 z) const {
    return x < width && z < height;
  }

  INLINE bool ContainsCell(u32 x, u32 z) const {
    return x < GetCellWidth() && z < GetCellHeight();
  }

  INLINE u16& BlockAt(u32 x, u32 z) { return block_id[x + z * width]; }
  INLINE u16& LodAt(u32 x, u32 z) { return lod[x + z * width]; }
  INLINE MapId& MapAt(u32 cell_x, u32 cell_z) {
    return map_id[cell_x + cell_z * GetCellWidth()];
  }

  MapId loaded_map_id;
  u16 _0; // 0x02
  u32 id; // 0x04
  void* archive_; // 0x08
  u8 facing; // 0x0C
  u8 _1; // 0x0D
  u16 width; // 0x0E
  u16 height; // 0x10
  u16 has_map_table; // 0x12
  MapId map_id[kMaxCells]; // 0x14
  u16 block_id[kMaxBlocks]; // 0x8014
  u16 lod[kMaxBlocks]; // 0x8814
  u16 load_step; // 0x9014
  u16 _2; // 0x9016
  void* load_buffer; // 0x9018
  CameraScrollArea camera_scroll; // 0x901C
};

static_assert(sizeof(WorldLayout) == 0x9160,
              "WorldLayout must match the game's MapMatrix layout");
} // namespace overworld
