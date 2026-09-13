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

namespace overworld {
struct WorldLayout;
}

namespace overworld {

struct TileEdit {
  u16 layout_id;
  u16 tile_x;
  u16 tile_z;
  u16 _0;
  u32 attr;
};

class TileEditor {
  MAKE_SINGLETON(TileEditor)

public:
  static constexpr u32 kMaxEdits = 1024;
  static constexpr u32 kMaxBlocks = 16;
  static constexpr u32 kMaxCaptured = 16;
  static constexpr u32 kCapturedLifetime = 300;

  struct Block {
    u32 id;
    s32 x;
    s32 z;
    const u16* header;
    u32* attrs;
    u32 width;
    u32 height;
  };

  struct Report {
    u32 slots;
    u32 blocks;
    u32 captured;
    u32 slots_offset;
    u32 translate_offset;
    u32 id_offset;
    u32 pack_offset;
    const c8* slot_class;
  };

  struct Captured {
    s32 x;
    s32 z;
    const u16* header;
    u32 frame;
  };

  bool is_enabled = true;

  static void Initialize();
  static void Update();
  static const Report& GetReport();
  static void OnAttrQuery(void* data, const Vec3* local);
  static u32 CollectBlocks(Block* out, u32 max);
  static const Block* Find(const Block* blocks, u32 count, s32 tile_x, s32 tile_z);
  static u32* Slot(const Block* blocks, u32 count, s32 tile_x, s32 tile_z);
  static bool ReadRaw(const Block* blocks, u32 count, s32 tile_x, s32 tile_z, u32* attr);
  static bool Read(const Block* blocks, u32 count, s32 tile_x, s32 tile_z, Tile* tile);
  static bool Write(s32 tile_x, s32 tile_z, u32 attr);
  static u32 GetEditCount();
  static void ClearEdits();
  static bool Save();
  static bool Load();

private:
  static constexpr u32 kMagic = 0x454C4954;
  static constexpr const c16* kFilename =
      u"sdmc:/luma/plugins/000400000011C500/sango_tiles.bin";

  static bool IsHeapPointer(uptr address);
  static bool IsMapBlock(uptr object);
  static bool IsPack(const Bundle* pack);
  static bool IsBlockCenter(f32 value);

  u32 CollectMapperBlocks(Block* out, u32 max, WorldLayout& layout);
  const Captured* FindCaptured(s32 x, s32 z) const;
  bool FindSlots(uptr mapper);
  bool FindFields(uptr block, WorldLayout& layout);
  void Record(u16 layout_id, s32 tile_x, s32 tile_z, u32 attr);

  TileEdit edits_[kMaxEdits];
  u32 edit_count_ = 0;
  Report report_ = {};
  Captured captured_[kMaxCaptured] = {};
  u32 frame_ = 0;
};

} // namespace overworld
