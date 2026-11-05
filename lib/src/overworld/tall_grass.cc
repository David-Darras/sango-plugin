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
 * @file tall_grass.cc
 * @brief Shows 3D tall grass around the player.
 *
 * The declarations are in overworld/patch/tall_grass.h.
 */

#include "overworld/patch/tall_grass.h"

#include <cmath>

#include "core/data_folder.h"
#include "core/hook.h"
#include "core/utils.h"
#include "core/native/process_manager.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/placed_decorations.h"
#include "overworld/patch/tile_editor.h"
#include "renderer/native/h3d_model.h"
#include "renderer/native/h3d_shader_model.h"
#include "system/native/file.h"
#include "ui/log.h"
#include "overworld/address.h"

namespace overworld {
namespace {
constexpr f32 kPi = 3.14159265f;

// The numbers of the three grass models in the pack of the field: grass00,
// grass04 and grass05. The game keeps them in memory all the time.
constexpr u32 kGrassModelIndex[3] = {42, 56, 69};

#ifdef GAME_ORAS
// Where the models are: the game manager keeps the object of the resident
// resources of the field, and this object keeps the models of the effects.
constexpr u32 kResidentResourcesOffset = 0x1A0;
constexpr u32 kEffectModelsOffset = 0x8C;
// true when the game does not have the models: the grass cannot work.
bool are_models_missing = false;

// Returns the object that keeps the models of the effects, or null when the
// field is not loaded.
void* FindResidentModels() {
  const uptr manager = READ32(sys::address::kGameManager);
  if (manager == 0) return nullptr;
  u32 resident = 0;
  SAFE_READ32(manager + kResidentResourcesOffset, resident);
  if (resident == 0) return nullptr;
  u32 models = 0;
  SAFE_READ32(resident + kEffectModelsOffset, models);
  return reinterpret_cast<void*>(models);
}
#endif
const c16 kMapsFolder[] = SANGO_DATA_PATH(u"maps");
// Shared by all the products, like the folders of the models.
const c16 kPackPath[] = u"sdmc:/sango/grass/grass.pc";
constexpr u32 kPlacedMagic = 0x31535247;  // "GRS1"

void MakePlacedPath(u32 layout, c16* out) {
  core::Utils::Format(out, SANGO_DATA_PATH(u"maps/layout_%d.grass"),
                      static_cast<int>(layout));
}

constexpr const c16* kTracePath = u"sdmc:/sango/tall_grass.log";

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

TallGrass::Kind TallGrass::PickKind(u32 hash) {
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

bool TallGrass::LoadResources() {
  auto& ctx = GetInstance();
  if (ctx.resources_[kGreen] != nullptr) return true;

#ifdef GAME_ORAS
  // The models are the ones that the game keeps for its own grass: they are
  // set up already, and the game frees them.
  void* models = FindResidentModels();
  if (models == nullptr) return false;
  for (u32 i = 0; i < kKindCount; i++) {
    ctx.resources_[i] = core::GameFunction<renderer::H3dResource *(void*,
                                                                   u32)>::Call(
        address::kGetResidentModel, models, kGrassModelIndex[i]);
    if (ctx.resources_[i] == nullptr) {
      ui::Log::Print(u"tall grass: model %u is not in the game", i);
      are_models_missing = true;
      for (u32 j = 0; j < kKindCount; j++) ctx.resources_[j] = nullptr;
      return false;
    }
  }
  return true;
#else
  return false;
#endif
}

bool TallGrass::LoadXyResources() {
  auto& ctx = GetInstance();
  if (ctx.xy_resources_[0] != nullptr) return true;
  if (ctx.is_xy_missing_) return false;
#ifdef GAME_ORAS
  void* pack = pokemon::ModelLoader::ReadSdPack(kPackPath, kPackCapacity);
  if (pack == nullptr) {
    // The heap of the field may be not ready: try some times.
    if (++ctx.xy_failures_ >= kPackAttempts) {
      ui::Log::Print(u"tall grass: no file sango/grass/grass.pc");
      ctx.is_xy_missing_ = true;
    }
    return false;
  }
  for (u32 i = 0; i < kXyKindCount; i++) {
    ctx.xy_resources_[i] = pokemon::ModelLoader::LoadPackResource(pack, i);
    if (ctx.xy_resources_[i] == nullptr) {
      ui::Log::Print(u"tall grass: the grass %u of the pack does not load", i);
      for (u32 j = 0; j < i; j++) {
        ctx.xy_resources_[j]->RemoveData();
        pokemon::ModelLoader::FreeBuffer(ctx.xy_resources_[j]);
        ctx.xy_resources_[j] = nullptr;
      }
      pokemon::ModelLoader::FreeBuffer(pack);
      ctx.is_xy_missing_ = true;
      return false;
    }
  }
  ctx.xy_pack_ = pack;
  return true;
#else
  return false;
#endif
}

void TallGrass::LoadPlaced(u32 layout) {
  auto& ctx = GetInstance();
  ctx.placed_layout_ = layout;
  ctx.placed_count_ = 0;
  c16 path[64];
  MakePlacedPath(layout, path);
  sys::File file;
  file.Open(path, sys::File::kRead);
  if (!file.IsOpen()) return;
  u32 header[3] = {};
  if (file.Read(header, sizeof(header)) != static_cast<s32>(sizeof(header)) ||
      header[0] != kPlacedMagic || header[1] != layout ||
      header[2] > kMaxPlaced) {
    return;
  }
  const u32 size = header[2] * sizeof(Placed);
  if (size != 0 && file.Read(ctx.placed_, size) != static_cast<s32>(size)) {
    return;
  }
  for (u32 i = 0; i < header[2]; i++) {
    if (ctx.placed_[i].kind >= kXyKindCount) ctx.placed_[i].kind = 0;
  }
  ctx.placed_count_ = header[2];
}

void TallGrass::SavePlaced() {
  auto& ctx = GetInstance();
  if (ctx.placed_layout_ == 0xFFFFFFFF) return;
  c16 path[64];
  MakePlacedPath(ctx.placed_layout_, path);
  if (ctx.placed_count_ == 0) {
    sys::File::Delete(path);
    return;
  }
  core::MakeDataFolder();
  sys::File::CreateDirectory(kMapsFolder);
  sys::File file(path, true);
  if (!file.IsOpen()) return;
  const u32 header[3] = {kPlacedMagic, ctx.placed_layout_, ctx.placed_count_};
  file.Write(header, sizeof(header));
  file.Write(ctx.placed_, ctx.placed_count_ * sizeof(Placed));
}

s32 TallGrass::FindPlaced(s32 tile_x, s32 tile_z) {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < ctx.placed_count_; i++) {
    if (ctx.placed_[i].tile_x == tile_x && ctx.placed_[i].tile_z == tile_z) {
      return static_cast<s32>(i);
    }
  }
  return -1;
}

bool TallGrass::Upsert(s32 tile_x, s32 tile_z, u8 kind) {
  auto& ctx = GetInstance();
  const s32 found = FindPlaced(tile_x, tile_z);
  if (found >= 0) {
    ctx.placed_[found].kind = kind;
    return true;
  }
  if (ctx.placed_count_ >= kMaxPlaced) return false;
  Placed& entry = ctx.placed_[ctx.placed_count_++];
  entry.tile_x = static_cast<s16>(tile_x);
  entry.tile_z = static_cast<s16>(tile_z);
  entry.kind = kind;
  return true;
}

// Writes the tiles of the rectangle in front of the player and returns their
// number. The rectangle starts at the tile in front of the player, goes away
// from the player, and is centred on the line of the player.
u32 TallGrass::GetFrontTiles(s32* tiles_x, s32* tiles_z) {
  auto& ctx = GetInstance();
  auto& player = ModelManager::GetInstance().GetPlayer();
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);
  s32 step_x = 0;
  s32 step_z = 0;
  const Vec3& look = player.facing_direction;
  if (std::fabs(look.x) > std::fabs(look.z)) {
    step_x = look.x > 0 ? 1 : -1;
  } else {
    step_z = look.z > 0 ? 1 : -1;
  }
  const s32 width = static_cast<s32>(
      ctx.placed_width < 1 ? 1 : (ctx.placed_width > kMaxSide
                                      ? kMaxSide : ctx.placed_width));
  const s32 depth = static_cast<s32>(
      ctx.placed_depth < 1 ? 1 : (ctx.placed_depth > kMaxSide
                                      ? kMaxSide : ctx.placed_depth));
  // For the test: the numbers of the facing, in thousandths.
  const Vec3& draw = player.GetDrawModel().position;
  ui::Log::Print(u"grass: look %d %d, tile %d %d, draw %d %d",
                 static_cast<int>(look.x * 1000),
                 static_cast<int>(look.z * 1000), static_cast<int>(player_x),
                 static_cast<int>(player_z), static_cast<int>(draw.x),
                 static_cast<int>(draw.z));
  u32 count = 0;
  for (s32 d = 0; d < depth; d++) {
    for (s32 w = 0; w < width; w++) {
      const s32 side = w - (width - 1) / 2;
      tiles_x[count] = player_x + step_x * (1 + d) + (-step_z) * side;
      tiles_z[count] = player_z + step_z * (1 + d) + step_x * side;
      count++;
    }
  }
  return count;
}

bool TallGrass::PlaceInFrontOfPlayer() {
  auto& ctx = GetInstance();
  ctx.is_xy_missing_ = false;
  ctx.xy_failures_ = 0;
  if (ctx.placed_layout_ == 0xFFFFFFFF) {
    LoadPlaced(WorldLayout::GetInstance().id);
  }
  const u8 kind = static_cast<u8>(ctx.placed_kind < kXyKindCount
                                      ? ctx.placed_kind : 0);
  s32 tiles_x[kMaxSide * kMaxSide];
  s32 tiles_z[kMaxSide * kMaxSide];
  const u32 count = GetFrontTiles(tiles_x, tiles_z);
  bool is_ok = true;
  for (u32 i = 0; i < count; i++) {
    if (!Upsert(tiles_x[i], tiles_z[i], kind)) is_ok = false;
  }
  SavePlaced();
  if (!is_ok) ui::Log::Print(u"tall grass: list full (max %u)", kMaxPlaced);
  return is_ok;
}

bool TallGrass::PlaceSquareInFrontOfPlayer(u32 side) {
  auto& ctx = GetInstance();
  const u32 width = ctx.placed_width;
  const u32 depth = ctx.placed_depth;
  ctx.placed_width = side;
  ctx.placed_depth = side;
  const bool is_ok = PlaceInFrontOfPlayer();
  ctx.placed_width = width;
  ctx.placed_depth = depth;
  return is_ok;
}

void TallGrass::RemoveInFrontOfPlayer() {
  auto& ctx = GetInstance();
  s32 tiles_x[kMaxSide * kMaxSide];
  s32 tiles_z[kMaxSide * kMaxSide];
  const u32 count = GetFrontTiles(tiles_x, tiles_z);
  for (u32 i = 0; i < count; i++) {
    const s32 found = FindPlaced(tiles_x[i], tiles_z[i]);
    if (found < 0) continue;
    ctx.placed_[found] = ctx.placed_[--ctx.placed_count_];
  }
  SavePlaced();
}

void TallGrass::RemoveAllPlaced() {
  auto& ctx = GetInstance();
  ctx.placed_count_ = 0;
  SavePlaced();
}

u32 TallGrass::GetPlacedCount() { return GetInstance().placed_count_; }

TallGrass::XyPatch* TallGrass::FindXyPatch(s32 tile_x, s32 tile_z) {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxShown; i++) {
    XyPatch& patch = ctx.xy_patches_[i];
    if (patch.is_used && patch.tile_x == tile_x && patch.tile_z == tile_z) {
      return &patch;
    }
  }
  return nullptr;
}

TallGrass::XyPatch* TallGrass::TakeFreeXyPatch(u8 kind, u32* builds_left) {
  auto& ctx = GetInstance();
  XyPatch* empty = nullptr;
  XyPatch* other = nullptr;
  for (u32 i = 0; i < kMaxShown; i++) {
    XyPatch& patch = ctx.xy_patches_[i];
    if (patch.is_used) continue;
    if (patch.model.IsLoaded()) {
      if (patch.kind == kind) return &patch;
      other = &patch;
    } else if (empty == nullptr) {
      empty = &patch;
    }
  }
  if (*builds_left == 0) return nullptr;
  if (empty == nullptr && other != nullptr) {
    // A model of another kind: build the model again.
    pokemon::ModelLoader::Drop(&other->model);
    empty = other;
  }
  if (empty == nullptr) return nullptr;
  (*builds_left)--;
  empty->kind = kind;
  return empty;
}

void TallGrass::ReleaseXy(XyPatch& patch) {
  patch.is_used = false;
  if (!patch.model.IsLoaded()) return;
  patch.model.model->SetTranslate(Vec3(0.0f, kHiddenHeight, 0.0f));
}

void TallGrass::RefreshPlaced() {
  auto& ctx = GetInstance();
  auto& player = ModelManager::GetInstance().GetPlayer();
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);
  const f32 ground = player.GetDrawModel().position.y;
  const s32 radius = static_cast<s32>(kPlacedRadius);

  // Hide the grass that was removed or that is far.
  for (u32 i = 0; i < kMaxShown; i++) {
    XyPatch& patch = ctx.xy_patches_[i];
    if (!patch.is_used) continue;
    const s32 found = FindPlaced(patch.tile_x, patch.tile_z);
    if (found < 0 || ctx.placed_[found].kind != patch.kind ||
        Abs(patch.tile_x - player_x) > radius + 2 ||
        Abs(patch.tile_z - player_z) > radius + 2) {
      ReleaseXy(patch);
    }
  }
  if (ctx.placed_count_ == 0 || !LoadXyResources()) return;

  const f32 tile_size = static_cast<f32>(WorldLayout::kUnitsPerTile);
  u32 builds_left = kBuildsPerRefresh;
  for (u32 i = 0; i < ctx.placed_count_; i++) {
    const Placed entry = ctx.placed_[i];
    if (Abs(entry.tile_x - player_x) > radius ||
        Abs(entry.tile_z - player_z) > radius) {
      continue;
    }
    if (FindXyPatch(entry.tile_x, entry.tile_z) != nullptr) continue;
    XyPatch* patch = TakeFreeXyPatch(entry.kind, &builds_left);
    if (patch == nullptr) continue;
    const Vec3 position((entry.tile_x + 0.5f) * tile_size, ground,
                        (entry.tile_z + 0.5f) * tile_size);
    if (!patch->model.IsLoaded() &&
        !pokemon::ModelLoader::LoadShared(&patch->model,
                                          ctx.xy_resources_[entry.kind],
                                          position)) {
      continue;
    }
    // No turn: the light comes from one side, so a turned copy of a model
    // would be lighter or darker than the others.
    patch->yaw = 0.0f;
    patch->model.model->SetTranslate(position);
    patch->model.model->SetRotate(Vec3(0.0f, patch->yaw, 0.0f));
    patch->sway_frames = 0;
    patch->was_player_on = false;
    patch->tile_x = entry.tile_x;
    patch->tile_z = entry.tile_z;
    patch->kind = entry.kind;
    patch->is_used = true;
  }
}

void TallGrass::UpdateSway() {
  auto& ctx = GetInstance();
#ifdef GAME_ORAS
  // The light set can change in the menu: the models follow.
  const s32 light_set = static_cast<s32>(ctx.placed_light_set);
  for (u32 i = 0; i < kMaxShown; i++) {
    XyPatch& patch = ctx.xy_patches_[i];
    if (!patch.model.IsLoaded() || patch.light_set == light_set) continue;
    auto* own = reinterpret_cast<renderer::H3dModel*>(patch.model.model);
    const s32 count = own->GetMaterialCount();
    for (s32 mat = 0; mat < count; mat++) own->SetLightSet(mat, light_set);
    patch.light_set = light_set;
  }
#endif
  auto& player = ModelManager::GetInstance().GetPlayer();
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);
  for (u32 i = 0; i < kMaxShown; i++) {
    XyPatch& patch = ctx.xy_patches_[i];
    if (!patch.is_used || !patch.model.IsLoaded()) continue;
    const bool is_on = patch.tile_x == player_x && patch.tile_z == player_z;
    if (is_on && !patch.was_player_on) patch.sway_frames = kSwayFrames;
    patch.was_player_on = is_on;
    if (patch.sway_frames == 0) continue;
    patch.sway_frames--;
    // A wave that gets smaller until the grass stands again.
    const f32 amount = static_cast<f32>(patch.sway_frames) / kSwayFrames;
    const f32 angle = std::sin(patch.sway_frames * 0.35f) * 0.12f * amount * amount;
    patch.model.model->SetRotate(Vec3(angle, patch.yaw, -angle));
  }
}

void TallGrass::DropXy() {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxShown; i++) {
    pokemon::ModelLoader::Drop(&ctx.xy_patches_[i].model);
    ctx.xy_patches_[i] = XyPatch{};
  }
  for (u32 i = 0; i < kXyKindCount; i++) {
    if (ctx.xy_resources_[i] == nullptr) continue;
    ctx.xy_resources_[i]->RemoveData();
    pokemon::ModelLoader::FreeBuffer(ctx.xy_resources_[i]);
    ctx.xy_resources_[i] = nullptr;
  }
  pokemon::ModelLoader::FreeBuffer(ctx.xy_pack_);
  ctx.xy_pack_ = nullptr;
}

// The game destroyed the scene: forget the models without touching them.
void TallGrass::DiscardXy() {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxShown; i++) {
    if (ctx.xy_patches_[i].model.IsLoaded()) {
      pokemon::ModelLoader::Untrack(&ctx.xy_patches_[i].model);
    }
    ctx.xy_patches_[i] = XyPatch{};
  }
  for (u32 i = 0; i < kXyKindCount; i++) ctx.xy_resources_[i] = nullptr;
  ctx.xy_pack_ = nullptr;
}

TallGrass::Patch* TallGrass::FindPatch(s32 tile_x, s32 tile_z) {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxPatches; i++) {
    Patch& patch = ctx.patches_[i];
    if (patch.is_used && patch.tile_x == tile_x && patch.tile_z == tile_z) {
      return &patch;
    }
  }
  return nullptr;
}

TallGrass::Patch* TallGrass::TakeFreePatch(Kind kind, u32* builds_left) {
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

TallGrass::Patch* TallGrass::StealFarPatch(Kind kind, s32 player_x,
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

void TallGrass::Release(Patch& patch) {
  patch.is_used = false;
  if (!patch.model.IsLoaded()) return;
  patch.model.model->SetTranslate(Vec3(0.0f, kHiddenHeight, 0.0f));
}

void TallGrass::Refresh() {
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

void TallGrass::DropAll() {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxPatches; i++) {
    pokemon::ModelLoader::Drop(&ctx.patches_[i].model);
    ctx.patches_[i] = Patch{};
  }
  for (u32 i = 0; i < kKindCount; i++) ctx.resources_[i] = nullptr;
  DropXy();
}

void TallGrass::DiscardAll() {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < kMaxPatches; i++) {
    if (ctx.patches_[i].model.IsLoaded()) {
      pokemon::ModelLoader::Untrack(&ctx.patches_[i].model);
    }
    ctx.patches_[i] = Patch{};
  }
  for (u32 i = 0; i < kKindCount; i++) ctx.resources_[i] = nullptr;
  DiscardXy();
}

void TallGrass::RemoveModelsBeforeBattle() {
  auto& ctx = GetInstance();
  if (ctx.resources_[kGreen] == nullptr && ctx.xy_pack_ == nullptr) return;
  DropAll();
  ctx.is_battle_starting_ = true;
  ctx.is_battle_seen_ = false;
  ctx.battle_start_frame_ = ctx.frame_;
}

void TallGrass::Update() {
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

  if (core::ProcessManager::IsOverworldActive()) {
    const u32 layout = WorldLayout::GetInstance().id;
    if (layout != ctx.placed_layout_) LoadPlaced(layout);
  }
  const bool has_placed = ctx.placed_count_ > 0;
  const bool has_resources =
      ctx.resources_[kGreen] != nullptr || ctx.xy_pack_ != nullptr;
  if (!has_resources && !ctx.is_enabled && !has_placed) return;

  if (!core::ProcessManager::IsOverworldActive()) {
    DiscardAll();
    return;
  }
  if (MapManager::GetInstance().GetNextMapId() != MapManager::kNoMap) {
    DropAll();
    return;
  }
  if (!ctx.is_enabled && !has_placed) {
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
  if (ctx.xy_pack_ != nullptr) UpdateSway();
  if (ctx.frame_ % kRefreshFrames != 0) return;
  if (has_placed || ctx.xy_pack_ != nullptr) RefreshPlaced();
  if (!ctx.is_enabled) {
    // The automatic grass is off: remove it, and keep the placed grass.
    if (ctx.resources_[kGreen] != nullptr) {
      for (u32 i = 0; i < kMaxPatches; i++) {
        pokemon::ModelLoader::Drop(&ctx.patches_[i].model);
        ctx.patches_[i] = Patch{};
      }
      for (u32 i = 0; i < kKindCount; i++) ctx.resources_[i] = nullptr;
    }
    return;
  }
  if (!LoadResources()) {
    // Without the object of the game, wait: the field is loading. Without
    // the models, nothing will change: stop.
#ifdef GAME_ORAS
    if (are_models_missing) ctx.is_enabled = false;
#else
    ctx.is_enabled = false;
#endif
    return;
  }
  if (ctx.frame_ < 3000) Trace("refresh");
  Refresh();
  if (ctx.frame_ < 3000) Trace("refresh done");
}
} // namespace overworld
