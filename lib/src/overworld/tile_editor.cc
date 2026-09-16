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

#include "overworld/patch/tile_editor.h"
#include "overworld/native/map_manager.h"
#include <cstring>
#include "core/hook_manager.h"
#include "core/native/game_manager.h"
#include "core/native/process_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
#include "system/native/file.h"
#include "core/utils.h"

namespace overworld {

constexpr const c16* TileEditor::kFilename;

void TileEditor::Initialize() {
  Load();
}

void TileEditor::Update() {
  auto& ctx = GetInstance();
  ctx.frame_++;
  if (!ctx.is_enabled || ctx.edit_count_ == 0) return;
  if (!core::ProcessManager::IsOverworldActive()) return;

  Block blocks[kMaxBlocks];
  const u32 count = CollectBlocks(blocks, kMaxBlocks);
  if (count == 0) return;
  const u16 layout_id = WorldLayout::GetInstance().id;
  for (u32 i = 0; i < ctx.edit_count_; i++) {
    const TileEdit& edit = ctx.edits_[i];
    if (edit.layout_id != layout_id) continue;
    u32* slot = Slot(blocks, count, edit.tile_x, edit.tile_z);
    if (slot != nullptr && *slot != edit.attr) *slot = edit.attr;
  }
}

const TileEditor::Report& TileEditor::GetReport() {
  return GetInstance().report_;
}

void TileEditor::OnAttrQuery(void* data, const Vec3* local) {
  if (data == nullptr || local == nullptr) return;
  auto& ctx = GetInstance();
  const s32 tiles = WorldLayout::kTilesPerBlockSide;
  const f32 unit = WorldLayout::kUnitsPerTile;
  auto& player = ModelManager::GetInstance().GetPlayer();
  if (player.id != ModelManager::kPlayerId) return;
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

u32 TileEditor::CollectBlocks(Block* out, u32 max) {
  auto& ctx = GetInstance();
  ctx.report_.slots = 0;
  ctx.report_.blocks = 0;
  ctx.report_.captured = 0;
  if (!core::ProcessManager::IsOverworldActive()) return 0;
  auto& layout = WorldLayout::GetInstance();
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

u32 TileEditor::CollectMapperBlocks(Block* out, u32 max, WorldLayout& layout) {
  auto& ctx = *this;
  auto* field = &core::GameManager::GetInstance().GetOverworldMapManager();
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
    const f32 block_units = WorldLayout::kTilesPerBlockSide *
                            WorldLayout::kUnitsPerTile;
    const s32 x = static_cast<s32>(translate.x / block_units);
    const s32 z = static_cast<s32>(translate.z / block_units);
    if (x < 0 || z < 0 || !layout.ContainsBlock(x, z)) continue;
    if (layout.BlockAt(x, z) != id || id == WorldLayout::kNoBlock) {
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

const TileEditor::Captured* TileEditor::FindCaptured(s32 x, s32 z) const {
  for (u32 i = 0; i < kMaxCaptured; i++) {
    const Captured& entry = captured_[i];
    if (entry.header == nullptr || entry.x != x || entry.z != z) continue;
    if (frame_ - entry.frame > kCapturedLifetime) continue;
    return &entry;
  }
  return nullptr;
}

const TileEditor::Block* TileEditor::Find(const Block* blocks, u32 count, s32 tile_x,
                                       s32 tile_z) {
  const s32 tiles = WorldLayout::kTilesPerBlockSide;
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

u32* TileEditor::Slot(const Block* blocks, u32 count, s32 tile_x, s32 tile_z) {
  const Block* block = Find(blocks, count, tile_x, tile_z);
  if (block == nullptr) return nullptr;
  const s32 tiles = WorldLayout::kTilesPerBlockSide;
  const u32 local_x = tile_x - block->x * tiles;
  const u32 local_z = tile_z - block->z * tiles;
  return &block->attrs[local_x + local_z * block->width];
}

bool TileEditor::ReadRaw(const Block* blocks, u32 count, s32 tile_x, s32 tile_z,
                        u32* attr) {
  const u32* slot = Slot(blocks, count, tile_x, tile_z);
  if (slot == nullptr) return false;
  *attr = *slot;
  return true;
}

bool TileEditor::Read(const Block* blocks, u32 count, s32 tile_x, s32 tile_z,
                     Tile* tile) {
  const Block* block = Find(blocks, count, tile_x, tile_z);
  if (block == nullptr) return false;
  const s32 tiles = WorldLayout::kTilesPerBlockSide;
  const f32 unit = WorldLayout::kUnitsPerTile;
  const Vec3 local((tile_x - block->x * tiles) * unit + unit / 2.0f, 0.0f,
                   (tile_z - block->z * tiles) * unit + unit / 2.0f);
  const void* attr_data = block->header;
  core::Hook* hook = core::HookManager::GetInstance().Get(HookId::kGetMapTile);
  if (hook != nullptr && hook->IsInitialized()) {
    *tile = hook->CallOriginal<Tile>((void*)&attr_data,
                                                (Vec3*)&local);
  } else {
    *tile = ((Tile (*)(const void*, const Vec3*))
        address::kGetMapTile)(&attr_data, &local);
  }
  return true;
}

bool TileEditor::Write(s32 tile_x, s32 tile_z, u32 attr) {
  auto& ctx = GetInstance();
  Block blocks[kMaxBlocks];
  const u32 count = CollectBlocks(blocks, kMaxBlocks);
  u32* slot = Slot(blocks, count, tile_x, tile_z);
  if (slot == nullptr) return false;
  *slot = attr;
  ctx.Record(WorldLayout::GetInstance().id, tile_x, tile_z, attr);
  return true;
}

u32 TileEditor::GetEditCount() {
  return GetInstance().edit_count_;
}

void TileEditor::ClearEdits() {
  GetInstance().edit_count_ = 0;
}

bool TileEditor::Save() {
  auto& ctx = GetInstance();
  sys::File file(kFilename, true);
  if (!file.IsOpen()) return false;
  const u32 header[2] = {kMagic, ctx.edit_count_};
  file.Write(header, sizeof(header));
  if (ctx.edit_count_ > 0) {
    file.Write(ctx.edits_, sizeof(TileEdit) * ctx.edit_count_);
  }
  return true;
}

bool TileEditor::Load() {
  auto& ctx = GetInstance();
  static u8 buffer[8 + sizeof(TileEdit) * kMaxEdits];
  const u32 size = sys::File::ReadAll(kFilename, buffer, sizeof(buffer));
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

bool TileEditor::IsHeapPointer(uptr address) {
  return address >= 0x08000000 && address < 0x40000000 &&
         (address & 3) == 0;
}

bool TileEditor::IsMapBlock(uptr object) {
  if (!IsHeapPointer(object)) return false;
  const uptr vtable = READ32(object);
  if (vtable == address::kMapBlockVtable) return true;
  if (vtable < sys::address::kProcessMemoryStart ||
      vtable > sys::address::kProcessMemoryEnd) {
    return false;
  }
  return std::strcmp(core::Utils::GetClassNameFromVTable((void*)vtable),
                     "N5field8MapBlockE") == 0;
}

bool TileEditor::IsPack(const Bundle* pack) {
  if (!IsHeapPointer((uptr)pack)) return false;
  if (pack->resource_count == 0 || pack->resource_count > 64) return false;
  return pack->resource_offset[0] >= 4 + 4 * (pack->resource_count + 1u);
}

bool TileEditor::IsBlockCenter(f32 value) {
  const f32 block_units = WorldLayout::kTilesPerBlockSide *
                          WorldLayout::kUnitsPerTile;
  if (value < 0.0f || value >= block_units * WorldLayout::kMaxWidth) {
    return false;
  }
  const s32 index = static_cast<s32>(value / block_units);
  return value == index * block_units + block_units / 2.0f;
}

bool TileEditor::FindSlots(uptr mapper) {
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
        core::Utils::GetClassNameFromVTable((void*)READ32(READ32(slots)));
    return true;
  }
  return false;
}

bool TileEditor::FindFields(uptr block, WorldLayout& layout) {
  const f32 block_units = WorldLayout::kTilesPerBlockSide *
                          WorldLayout::kUnitsPerTile;
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
    if (expected == WorldLayout::kNoBlock) continue;
    for (u32 id_offset = offset + 12; id_offset < offset + 64;
         id_offset += 4) {
      if (READ32(block + id_offset) != expected) continue;
      const Captured* captured = FindCaptured(x, z);
      for (u32 pack_offset = id_offset + 4; pack_offset < id_offset + 32;
           pack_offset += 4) {
        auto* pack = *(Bundle**)(block + pack_offset);
        if (!IsPack(pack)) continue;
        const u16* header = (const u16*)pack->GetResource(0);
        if (header[0] != WorldLayout::kTilesPerBlockSide) continue;
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

void TileEditor::Record(u16 layout_id, s32 tile_x, s32 tile_z, u32 attr) {
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

} // namespace overworld
