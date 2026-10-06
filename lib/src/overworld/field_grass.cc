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

#include "overworld/patch/field_grass.h"

#include "core/native/process_manager.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/placed_decorations.h"
#include "overworld/patch/tile_editor.h"
#include "renderer/native/h3d_shader_model.h"
#include "system/native/file.h"
#include "ui/log_application.h"

namespace overworld {
namespace {
constexpr f32 kPi = 3.14159265f;

constexpr const c16* kGrassPackPath = u"sdmc:/sango/field_grass.bin";
constexpr u32 kGrassPackCapacity = 0x10000;
constexpr const c16* kTracePath = u"sdmc:/sango/field_grass.log";

void Trace(const c8* step) {
  static u32 offset = 0;
  static u32 lines = 0;
  if (lines++ >= 80) return;
  sys::File file;
  file.Open(kTracePath);
  if (!file.IsOpen()) return;
  c8 line[64];
  u32 length = 0;
  while (step[length] != 0 && length < 62) {
    line[length] = step[length];
    length++;
  }
  line[length++] = 10;
  file.Write(line, length, offset);
  offset += length;
}

u32 Hash(s32 tile_x, s32 tile_z) {
  u32 h = static_cast<u32>(tile_x) * 0x9E3779B1u;
  h ^= static_cast<u32>(tile_z) * 0x85EBCA77u + 0x165667B1u;
  h ^= h >> 15;
  h *= 0x2C1B3C6Du;
  h ^= h >> 12;
  return h;
}

s32 Abs(s32 value) { return value < 0 ? -value : value; }
} // namespace

u32 FieldGrass::GetPatchCount() {
  auto& ctx = GetInstance();
  u32 count = 0;
  for (u32 i = 0; i < kMaxPatches; i++) {
    if (ctx.patches_[i].is_used) count++;
  }
  return count;
}

FieldGrass::Kind FieldGrass::PickKind(u32 hash) {
  auto& ctx = GetInstance();
  switch (ctx.mix) {
    case kMixGreen:
      return kGreen;
    case kMixFern:
      return kFern;
    case kMixAsh:
      return kAsh;
    default:
      return static_cast<Kind>((hash >> 8) % kKindCount);
  }
}

bool FieldGrass::LoadResources() {
  auto& ctx = GetInstance();
  if (ctx.resources_[kGreen] != nullptr) return true;

  if (ctx.pack_ == nullptr) {
    Trace("read pack");
    ctx.pack_ = pokemon::ModelLoader::ReadSdPack(kGrassPackPath,
                                                 kGrassPackCapacity);
  }
  if (ctx.pack_ == nullptr) {
    ui::LogApplication::Print(u"field grass: pack unreadable");
    return false;
  }
  for (u32 i = 0; i < kKindCount; i++) {
    Trace("load resource");
    ctx.resources_[i] = pokemon::ModelLoader::LoadPackResource(ctx.pack_, i);
    if (ctx.resources_[i] == nullptr) {
      ui::LogApplication::Print(u"field grass: model %u unusable", i);
      for (u32 j = 0; j < i; j++) ctx.resources_[j]->RemoveData();
      for (u32 j = 0; j < kKindCount; j++) ctx.resources_[j] = nullptr;
      return false;
    }
  }
  return true;
}

FieldGrass::Patch* FieldGrass::FindPatch(s32 tile_x, s32 tile_z) {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxPatches; i++) {
    Patch& patch = ctx.patches_[i];
    if (patch.is_used && patch.tile_x == tile_x && patch.tile_z == tile_z) {
      return &patch;
    }
  }
  return nullptr;
}

FieldGrass::Patch* FieldGrass::TakeFreePatch(Kind kind, u32* builds_left) {
  auto& ctx = GetInstance();
  Patch* empty = nullptr;
  for (u32 i = 0; i < kMaxPatches; i++) {
    Patch& patch = ctx.patches_[i];
    if (patch.is_used) continue;
    if (patch.model.IsLoaded() && patch.kind == kind) return &patch;
    if (!patch.model.IsLoaded() && empty == nullptr) empty = &patch;
  }
  if (empty != nullptr && *builds_left > 0) {
    (*builds_left)--;
    empty->kind = kind;
    return empty;
  }
  return nullptr;
}

FieldGrass::Patch* FieldGrass::StealFarPatch(Kind kind, s32 player_x,
                                             s32 player_z, s32 max_distance) {
  auto& ctx = GetInstance();
  Patch* farthest = nullptr;
  s32 farthest_distance = max_distance;
  for (u32 i = 0; i < kMaxPatches; i++) {
    Patch& patch = ctx.patches_[i];
    if (!patch.is_used || patch.kind != kind) continue;
    const s32 dx = Abs(patch.tile_x - player_x);
    const s32 dz = Abs(patch.tile_z - player_z);
    const s32 distance = dx > dz ? dx : dz;
    if (distance > farthest_distance) {
      farthest_distance = distance;
      farthest = &patch;
    }
  }
  return farthest;
}

void FieldGrass::Release(Patch& patch) {
  patch.is_used = false;
  if (!patch.model.IsLoaded()) return;
  patch.model.model->SetTranslate(Vec3(0.0f, kHiddenHeight, 0.0f));
}

void FieldGrass::Refresh() {
  auto& ctx = GetInstance();
  auto& player = ModelManager::GetInstance().GetPlayer();
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);
  const f32 ground = player.GetDrawModel().position.y;
  const s32 radius =
      static_cast<s32>(ctx.radius > kMaxRadius ? kMaxRadius : ctx.radius);

  for (u32 i = 0; i < kMaxPatches; i++) {
    Patch& patch = ctx.patches_[i];
    if (!patch.is_used) continue;
    if (Abs(patch.tile_x - player_x) > radius ||
        Abs(patch.tile_z - player_z) > radius) {
      Release(patch);
    }
  }

  TileEditor::Block blocks[TileEditor::kMaxBlocks];
  const u32 block_count =
      TileEditor::CollectBlocks(blocks, TileEditor::kMaxBlocks);
  if (block_count == 0) return;

  const f32 tile_size = static_cast<f32>(WorldLayout::kUnitsPerTile);
  u32 builds_left = kBuildsPerRefresh;
  for (s32 ring = 1; ring <= radius; ring++) {
    for (s32 dz = -ring; dz <= ring; dz++) {
      const s32 step = Abs(dz) == ring ? 1 : 2 * ring;
      for (s32 dx = -ring; dx <= ring; dx += step) {
      const s32 tile_x = player_x + dx;
      const s32 tile_z = player_z + dz;
      const u32 hash = Hash(tile_x, tile_z);
      if (hash % 100 >= ctx.density) continue;
      if (FindPatch(tile_x, tile_z) != nullptr) continue;

      Tile tile;
      if (!TileEditor::Read(blocks, block_count, tile_x, tile_z, &tile)) {
        continue;
      }
      if (tile.is_impassable || tile.is_water) continue;

      const Kind kind = PickKind(hash);
      Patch* patch = TakeFreePatch(kind, &builds_left);
      if (patch == nullptr && builds_left > 0) {
        patch = StealFarPatch(kind, player_x, player_z, ring);
        if (patch != nullptr) Release(*patch);
      }
      if (patch == nullptr) continue;

      const Vec3 position((tile_x + 0.5f) * tile_size, ground,
                          (tile_z + 0.5f) * tile_size);
      if (!patch->model.IsLoaded()) Trace("build patch");
      if (!patch->model.IsLoaded() &&
          !pokemon::ModelLoader::LoadShared(&patch->model,
                                            ctx.resources_[kind], position)) {
        continue;
      }
      Trace("patch ready");
      patch->model.model->SetTranslate(position);
      patch->model.model->SetRotate(
          Vec3(0.0f, static_cast<f32>((hash >> 20) & 3) * kPi / 2.0f, 0.0f));
      patch->tile_x = tile_x;
      patch->tile_z = tile_z;
      patch->is_used = true;
      }
    }
  }
}

void FieldGrass::DropAll() {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxPatches; i++) {
    pokemon::ModelLoader::Drop(&ctx.patches_[i].model);
    ctx.patches_[i] = Patch{};
  }
  for (u32 i = 0; i < kKindCount; i++) {
    if (ctx.resources_[i] != nullptr) ctx.resources_[i]->RemoveData();
    ctx.resources_[i] = nullptr;
  }
  pokemon::ModelLoader::FreeBuffer(ctx.pack_);
  ctx.pack_ = nullptr;
}

void FieldGrass::DiscardAll() {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxPatches; i++) {
    if (ctx.patches_[i].model.IsLoaded()) {
      pokemon::ModelLoader::Untrack(&ctx.patches_[i].model);
    }
    ctx.patches_[i] = Patch{};
  }
  for (u32 i = 0; i < kKindCount; i++) ctx.resources_[i] = nullptr;
  ctx.pack_ = nullptr;
}

void FieldGrass::RemoveModelsBeforeBattle() {
  auto& ctx = GetInstance();
  if (ctx.resources_[kGreen] == nullptr) return;
  DropAll();
  ctx.is_battle_starting_ = true;
  ctx.is_battle_seen_ = false;
  ctx.battle_start_frame_ = ctx.frame_;
}

void FieldGrass::Update() {
  auto& ctx = GetInstance();
  ctx.frame_++;

  if (ctx.is_battle_starting_) {
    if (core::ProcessManager::IsBattleActive()) {
      ctx.is_battle_seen_ = true;
    } else if (core::ProcessManager::IsOverworldActive() &&
               (ctx.is_battle_seen_ ||
                ctx.frame_ - ctx.battle_start_frame_ > kBattleTimeoutFrames)) {
      ctx.is_battle_starting_ = false;
      ctx.is_battle_seen_ = false;
    }
  }

  const bool has_resources = ctx.resources_[kGreen] != nullptr;
  if (!has_resources && !ctx.is_enabled) return;

  if (!core::ProcessManager::IsOverworldActive()) {
    DiscardAll();
    return;
  }
  if (MapManager::GetInstance().GetNextMapId() != MapManager::kNoMap) {
    DropAll();
    return;
  }
  if (!ctx.is_enabled) {
    DropAll();
    return;
  }
  if (ctx.is_battle_starting_) return;
  if (PlacedDecorations::IsNearExit(MapManager::GetInstance().GetMap())) {
    DropAll();
    return;
  }

  const u32 map = static_cast<u32>(MapManager::GetInstance().GetMap());
  if (map != ctx.last_map_) {
    DiscardAll();
    ctx.last_map_ = map;
    ctx.map_load_frame_ = ctx.frame_;
    return;
  }
  if (ctx.frame_ - ctx.map_load_frame_ < kShowDelayFrames) return;
  if (ctx.frame_ % kRefreshFrames != 0) return;
  if (!LoadResources()) {
    ctx.is_enabled = false;
    return;
  }
  if (ctx.frame_ < 3000) Trace("refresh");
  Refresh();
  if (ctx.frame_ < 3000) Trace("refresh done");
}
} // namespace overworld
