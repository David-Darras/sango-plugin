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

#ifndef SANGO_PLUGIN_FEATURE_TILE_EDITOR_H
#define SANGO_PLUGIN_FEATURE_TILE_EDITOR_H
#include <cstring>

#include "common.h"
#include "feature/core/hook_manager.h"
#include "game/core/manager.h"
#include "game/core/process_manager.h"
#include "game/overworld/map_manager.h"
#include "game/overworld/model_manager.h"
#include "game/overworld/tile.h"
#include "game/overworld/world_layout.h"
#include "system/file.h"
#include "utils.h"

namespace feature {

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

  struct Block {
    u32 id;
    s32 x;
    s32 z;
    const u16* header;
    u32* attrs;
    u32 width;
    u32 height;
  };

  bool is_enabled = true;

  STATIC_INLINE void Initialize() { Load(); }

  static void Update() {
    auto& ctx = GetInstance();
    ctx.frame_++;
    if (!ctx.is_enabled || ctx.edit_count_ == 0) return;
    if (!game::ProcessManager::IsOverworldActive()) return;

    Block blocks[kMaxBlocks];
    const u32 count = CollectBlocks(blocks, kMaxBlocks);
    if (count == 0) return;
    const u16 layout_id = overworld::WorldLayout::GetInstance().id;
    for (u32 i = 0; i < ctx.edit_count_; i++) {
      const TileEdit& edit = ctx.edits_[i];
      if (edit.layout_id != layout_id) continue;
      u32* slot = Slot(blocks, count, edit.tile_x, edit.tile_z);
      if (slot != nullptr && *slot != edit.attr) *slot = edit.attr;
    }
  }

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

  static constexpr u32 kMaxCaptured = 16;
  static constexpr u32 kCapturedLifetime = 300;

  static const Report& GetReport() { return GetInstance().report_; }

  static void OnAttrQuery(void* data, const Vec3* local) {
    if (data == nullptr || local == nullptr) return;
    auto& ctx = GetInstance();
    const s32 tiles = overworld::WorldLayout::kTilesPerBlockSide;
    const f32 unit = overworld::WorldLayout::kUnitsPerTile;
    auto& player = overworld::ModelManager::GetInstance().GetPlayer();
    if (player.id != overworld::ModelManager::kPlayerId) return;
    const s32 tile_x = static_cast<s32>(player.map_pos.coords.x);
    const s32 tile_z = static_cast<s32>(player.map_pos.coords.z);
    if (tile_x < 0 || tile_z < 0) return;
    if (static_cast<s32>(local->x / unit) != tile_x % tiles) return;
    if (static_cast<s32>(local->z / unit) != tile_z % tiles) return;
    const u16* header = *(const u16**)data;
    if (!IsHeapPointer((uptr)header)) return;
    if (header[0] != tiles || header[1] != tiles) return;

    const s32 x = tile_x / tiles;
    const s32 z = tile_z / tiles;
    Captured* slot = nullptr;
    for (u32 i = 0; i < kMaxCaptured; i++) {
      Captured& entry = ctx.captured_[i];
      if (entry.header == nullptr) {
        if (slot == nullptr) slot = &entry;
        continue;
      }
      if (entry.x == x && entry.z == z) {
        slot = &entry;
        break;
      }
      if (entry.header == header) entry.header = nullptr;
      if (slot == nullptr && ctx.frame_ - entry.frame > kCapturedLifetime) {
        slot = &entry;
      }
    }
    if (slot == nullptr) slot = &ctx.captured_[0];
    slot->x = x;
    slot->z = z;
    slot->header = header;
    slot->frame = ctx.frame_;
  }

  static u32 CollectBlocks(Block* out, u32 max) {
    auto& ctx = GetInstance();
    ctx.report_.slots = 0;
    ctx.report_.blocks = 0;
    ctx.report_.captured = 0;
    if (!game::ProcessManager::IsOverworldActive()) return 0;
    auto& layout = overworld::WorldLayout::GetInstance();
    u32 found = ctx.CollectMapperBlocks(out, max, layout);
    for (u32 i = 0; i < kMaxCaptured && found < max; i++) {
      const Captured& entry = ctx.captured_[i];
      if (entry.header == nullptr) continue;
      if (ctx.frame_ - entry.frame > kCapturedLifetime) continue;
      if (!layout.ContainsBlock(entry.x, entry.z)) continue;
      bool present = false;
      for (u32 j = 0; j < found && !present; j++) {
        present = out[j].x == entry.x && out[j].z == entry.z;
      }
      if (present) continue;
      Block& block = out[found++];
      block.id = layout.BlockAt(entry.x, entry.z);
      block.x = entry.x;
      block.z = entry.z;
      block.header = entry.header;
      block.width = entry.header[0];
      block.height = entry.header[1];
      block.attrs = (u32*)(entry.header + 2);
      ctx.report_.captured++;
    }
    ctx.report_.blocks = found;
    return found;
  }

  u32 CollectMapperBlocks(Block* out, u32 max, overworld::WorldLayout& layout) {
    auto& ctx = *this;
    auto* field = &game::Manager::GetInstance().GetOverworldMapManager();
    if (field == nullptr) return 0;
    const uptr mapper = (uptr)field->GetMapper();
    if (!IsHeapPointer(mapper)) return 0;
    if (!ctx.FindSlots(mapper)) return 0;

    const uptr slots = READ32(mapper + ctx.report_.slots_offset);
    u32 found = 0;
    for (u32 i = 0; i < kMaxBlocks && found < max; i++) {
      const uptr block = READ32(slots + i * 4);
      if (!IsMapBlock(block)) break;
      ctx.report_.slots++;
      if (!ctx.FindFields(block, layout)) continue;
      const Vec3& translate =
          *(const Vec3*)(block + ctx.report_.translate_offset);
      const u32 id = READ32(block + ctx.report_.id_offset);
      auto* pack = *(Bundle**)(block + ctx.report_.pack_offset);
      const f32 block_units = overworld::WorldLayout::kTilesPerBlockSide *
                              overworld::WorldLayout::kUnitsPerTile;
      const s32 x = static_cast<s32>(translate.x / block_units);
      const s32 z = static_cast<s32>(translate.z / block_units);
      if (x < 0 || z < 0 || !layout.ContainsBlock(x, z)) continue;
      if (layout.BlockAt(x, z) != id || id == overworld::WorldLayout::kNoBlock) {
        continue;
      }
      if (!IsPack(pack)) continue;
      const u16* header = (const u16*)pack->GetResource(0);
      if (header[0] == 0 || header[0] > 64 || header[1] == 0 ||
          header[1] > 64) {
        continue;
      }
      const Captured* captured = ctx.FindCaptured(x, z);
      if (captured != nullptr && captured->header != header) {
        ctx.report_.translate_offset = 0;
        if (!ctx.FindFields(block, layout)) continue;
        pack = *(Bundle**)(block + ctx.report_.pack_offset);
        header = (const u16*)pack->GetResource(0);
      }
      Block& entry = out[found++];
      entry.id = id;
      entry.x = x;
      entry.z = z;
      entry.header = header;
      entry.width = header[0];
      entry.height = header[1];
      entry.attrs = (u32*)(header + 2);
    }
    return found;
  }

  const Captured* FindCaptured(s32 x, s32 z) const {
    for (u32 i = 0; i < kMaxCaptured; i++) {
      const Captured& entry = captured_[i];
      if (entry.header == nullptr || entry.x != x || entry.z != z) continue;
      if (frame_ - entry.frame > kCapturedLifetime) continue;
      return &entry;
    }
    return nullptr;
  }

  static const Block* Find(const Block* blocks, u32 count, s32 tile_x,
                           s32 tile_z) {
    const s32 tiles = overworld::WorldLayout::kTilesPerBlockSide;
    if (tile_x < 0 || tile_z < 0) return nullptr;
    for (u32 i = 0; i < count; i++) {
      const Block& block = blocks[i];
      if (tile_x / tiles != block.x || tile_z / tiles != block.z) continue;
      const u32 local_x = tile_x - block.x * tiles;
      const u32 local_z = tile_z - block.z * tiles;
      if (local_x >= block.width || local_z >= block.height) return nullptr;
      return &block;
    }
    return nullptr;
  }

  static u32* Slot(const Block* blocks, u32 count, s32 tile_x, s32 tile_z) {
    const Block* block = Find(blocks, count, tile_x, tile_z);
    if (block == nullptr) return nullptr;
    const s32 tiles = overworld::WorldLayout::kTilesPerBlockSide;
    const u32 local_x = tile_x - block->x * tiles;
    const u32 local_z = tile_z - block->z * tiles;
    return &block->attrs[local_x + local_z * block->width];
  }

  static bool ReadRaw(const Block* blocks, u32 count, s32 tile_x, s32 tile_z,
                      u32* attr) {
    const u32* slot = Slot(blocks, count, tile_x, tile_z);
    if (slot == nullptr) return false;
    *attr = *slot;
    return true;
  }

  static bool Read(const Block* blocks, u32 count, s32 tile_x, s32 tile_z,
                   overworld::Tile* tile) {
    const Block* block = Find(blocks, count, tile_x, tile_z);
    if (block == nullptr) return false;
    const s32 tiles = overworld::WorldLayout::kTilesPerBlockSide;
    const f32 unit = overworld::WorldLayout::kUnitsPerTile;
    const Vec3 local((tile_x - block->x * tiles) * unit + unit / 2.0f, 0.0f,
                     (tile_z - block->z * tiles) * unit + unit / 2.0f);
    const void* attr_data = block->header;
    Hook* hook = HookManager::GetInstance().Get(HookID::kGetMapTile);
    if (hook != nullptr && hook->IsInitialized()) {
      *tile = hook->CallOriginal<overworld::Tile>((void*)&attr_data,
                                                  (Vec3*)&local);
    } else {
      *tile = ((overworld::Tile (*)(const void*, const Vec3*))
          ADDRESS_OVERWORLD_GET_MAP_TILE)(&attr_data, &local);
    }
    return true;
  }

  static bool Write(s32 tile_x, s32 tile_z, u32 attr) {
    auto& ctx = GetInstance();
    Block blocks[kMaxBlocks];
    const u32 count = CollectBlocks(blocks, kMaxBlocks);
    u32* slot = Slot(blocks, count, tile_x, tile_z);
    if (slot == nullptr) return false;
    *slot = attr;
    ctx.Record(overworld::WorldLayout::GetInstance().id, tile_x, tile_z, attr);
    return true;
  }

  static u32 GetEditCount() { return GetInstance().edit_count_; }
  static void ClearEdits() { GetInstance().edit_count_ = 0; }

  static bool Save() {
    auto& ctx = GetInstance();
    File file(kFilename, true);
    if (!file.IsOpen()) return false;
    const u32 header[2] = {kMagic, ctx.edit_count_};
    file.Write(header, sizeof(header));
    if (ctx.edit_count_ > 0) {
      file.Write(ctx.edits_, sizeof(TileEdit) * ctx.edit_count_);
    }
    return true;
  }

  static bool Load() {
    auto& ctx = GetInstance();
    static u8 buffer[8 + sizeof(TileEdit) * kMaxEdits];
    const u32 size = File::ReadAll(kFilename, buffer, sizeof(buffer));
    if (size < 8) return false;
    u32 header[2];
    std::memcpy(header, buffer, sizeof(header));
    if (header[0] != kMagic) return false;
    u32 count = header[1];
    if (count > kMaxEdits) count = kMaxEdits;
    if (8 + sizeof(TileEdit) * count > size) return false;
    std::memcpy(ctx.edits_, buffer + 8, sizeof(TileEdit) * count);
    ctx.edit_count_ = count;
    return true;
  }

private:
  static constexpr u32 kMagic = 0x454C4954;
  static constexpr const c16* kFilename =
      u"sdmc:/luma/plugins/000400000011C500/sango_tiles.bin";

  static bool IsHeapPointer(uptr address) {
    return address >= 0x08000000 && address < 0x40000000 &&
           (address & 3) == 0;
  }

  static bool IsMapBlock(uptr object) {
    if (!IsHeapPointer(object)) return false;
    const uptr vtable = READ32(object);
    if (vtable == ADDRESS_MAP_BLOCK_VTABLE) return true;
    if (vtable < ADDRESS_PROCESS_MEMORY_START ||
        vtable > ADDRESS_PROCESS_MEMORY_END) {
      return false;
    }
    return std::strcmp(Utils::GetClassNameFromVTable((void*)vtable),
                       "N5field8MapBlockE") == 0;
  }

  static bool IsPack(const Bundle* pack) {
    if (!IsHeapPointer((uptr)pack)) return false;
    if (pack->resource_count == 0 || pack->resource_count > 64) return false;
    return pack->resource_offset[0] >= 4 + 4 * (pack->resource_count + 1u);
  }

  static bool IsBlockCenter(f32 value) {
    const f32 block_units = overworld::WorldLayout::kTilesPerBlockSide *
                            overworld::WorldLayout::kUnitsPerTile;
    if (value < 0.0f || value >= block_units * overworld::WorldLayout::kMaxWidth) {
      return false;
    }
    const s32 index = static_cast<s32>(value / block_units);
    return value == index * block_units + block_units / 2.0f;
  }

  bool FindSlots(uptr mapper) {
    if (report_.slots_offset != 0) {
      const uptr slots = READ32(mapper + report_.slots_offset);
      if (IsHeapPointer(slots) && IsMapBlock(READ32(slots))) return true;
      report_.slots_offset = 0;
    }
    for (u32 offset = 4; offset < 0x100; offset += 4) {
      const uptr slots = READ32(mapper + offset);
      if (!IsHeapPointer(slots)) continue;
      if (!IsMapBlock(READ32(slots))) continue;
      report_.slots_offset = offset;
      report_.slot_class =
          Utils::GetClassNameFromVTable((void*)READ32(READ32(slots)));
      return true;
    }
    return false;
  }

  bool FindFields(uptr block, overworld::WorldLayout& layout) {
    const f32 block_units = overworld::WorldLayout::kTilesPerBlockSide *
                            overworld::WorldLayout::kUnitsPerTile;
    if (report_.translate_offset != 0) {
      const Vec3& translate = *(const Vec3*)(block + report_.translate_offset);
      if (IsBlockCenter(translate.x) && IsBlockCenter(translate.z)) return true;
    }
    for (u32 offset = 4; offset < 0x80; offset += 4) {
      const Vec3& translate = *(const Vec3*)(block + offset);
      if (!IsBlockCenter(translate.x) || !IsBlockCenter(translate.z)) continue;
      if (translate.y != 0.0f) continue;
      const u32 x = static_cast<u32>(translate.x / block_units);
      const u32 z = static_cast<u32>(translate.z / block_units);
      if (!layout.ContainsBlock(x, z)) continue;
      const u32 expected = layout.BlockAt(x, z);
      if (expected == overworld::WorldLayout::kNoBlock) continue;
      for (u32 id_offset = offset + 12; id_offset < offset + 64;
           id_offset += 4) {
        if (READ32(block + id_offset) != expected) continue;
        const Captured* captured = FindCaptured(x, z);
        for (u32 pack_offset = id_offset + 4; pack_offset < id_offset + 32;
             pack_offset += 4) {
          auto* pack = *(Bundle**)(block + pack_offset);
          if (!IsPack(pack)) continue;
          const u16* header = (const u16*)pack->GetResource(0);
          if (header[0] != overworld::WorldLayout::kTilesPerBlockSide) continue;
          if (captured != nullptr && captured->header != header) continue;
          report_.translate_offset = offset;
          report_.id_offset = id_offset;
          report_.pack_offset = pack_offset;
          return true;
        }
      }
    }
    return false;
  }

  void Record(u16 layout_id, s32 tile_x, s32 tile_z, u32 attr) {
    for (u32 i = 0; i < edit_count_; i++) {
      TileEdit& edit = edits_[i];
      if (edit.layout_id != layout_id || edit.tile_x != tile_x ||
          edit.tile_z != tile_z) {
        continue;
      }
      edit.attr = attr;
      return;
    }
    if (edit_count_ >= kMaxEdits) return;
    TileEdit& edit = edits_[edit_count_++];
    edit.layout_id = layout_id;
    edit.tile_x = tile_x;
    edit.tile_z = tile_z;
    edit._0 = 0;
    edit.attr = attr;
  }

  TileEdit edits_[kMaxEdits];
  u32 edit_count_ = 0;
  Report report_ = {};
  Captured captured_[kMaxCaptured] = {};
  u32 frame_ = 0;
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_TILE_EDITOR_H
