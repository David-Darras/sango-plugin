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
 * @file world_overlay.cc
 * @brief Marks on the 3D world of the overworld.
 *
 * The declarations are in overworld/patch/world_overlay.h.
 */

#include "overworld/patch/world_overlay.h"

#include "core/hook.h"
#include "core/native/process_manager.h"
#include "overworld/native/hidden_item.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/tile_editor.h"
#include "renderer/address.h"
#include "system/native/graphics.h"
#include "ui/main_application.h"

namespace overworld {

namespace {
core::Hook<void(void*, void*, void*, void*, s32, bool)> draw_scene_hook;
core::Hook<void(void*, void*, void*, bool)> stereo_draw_hook;

constexpr s32 kMarkSize = 6;

// The top screen is 400 x 240 pixels (a ratio of 1.67), the bottom screen is
// 320 x 240 (1.33). A perspective projection has two scales, one for each
// side of the screen: their ratio is the ratio of the screen, whatever the
// turn of the screen.
bool IsTopScreenProjection(const Mtx44& projection) {
  auto abs = [](f32 value) { return value < 0 ? -value : value; };
  f32 first = abs(projection.m[0][0]) > abs(projection.m[0][1])
                  ? abs(projection.m[0][0])
                  : abs(projection.m[0][1]);
  f32 second = abs(projection.m[1][0]) > abs(projection.m[1][1])
                   ? abs(projection.m[1][0])
                   : abs(projection.m[1][1]);
  if (first < 0.0001f || second < 0.0001f) return false;
  const f32 ratio = first > second ? first / second : second / first;
  return ratio > 1.55f && ratio < 1.8f;
}

// The center of a tile in the world: a tile is 18 x 18 units, and the tile
// (0, 0) starts at the origin of the world.
f32 TileCenter(s32 tile) {
  return (tile + 0.5f) * WorldLayout::kUnitsPerTile;
}

void DrawMark(s32 x, s32 y, Color color) {
  sys::Graphics::DrawRect(x - kMarkSize / 2, y - kMarkSize / 2, kMarkSize,
                          kMarkSize, color);
}
} // namespace

void WorldOverlay::Initialize() {
  draw_scene_hook.Install(renderer::address::kSceneDraw, DrawSceneHook);
  stereo_draw_hook.Install(renderer::address::kSceneStereoDraw,
                           StereoDrawHook);
}

void WorldOverlay::StereoDrawHook(void* scene, void* graphics, void* camera,
                                  bool flag) {
  auto& self = GetInstance();
  self.draws_seen++;
  // The stereo camera is a camera too: its matrices are those of the center
  // between the eyes, which is right for a flat overlay.
  if (!self.has_camera_) self.TryUseCamera(camera);
  stereo_draw_hook(scene, graphics, camera, flag);
}

void WorldOverlay::DrawSceneHook(void* scene, void* graphics, void* camera,
                                 void* target, s32 index, bool flag) {
  auto& self = GetInstance();
  self.draws_seen++;
  // Keep the first camera of the top screen in each frame: the camera of
  // the overworld. The other scenes of the frame (effects) come after it.
  if (!self.has_camera_) self.TryUseCamera(camera);
  draw_scene_hook(scene, graphics, camera, target, index, flag);
}

bool WorldOverlay::TryUseCamera(void* camera) {
  const uptr address = reinterpret_cast<uptr>(camera);
  if (renderer::address::kCameraGetMatrices == 0 || (address & 3) != 0 ||
      !sys::Graphics::IsDataAddress(address)) {
    return false;
  }
  Mtx34 view = {};
  Mtx44 projection = {};
  ((void (*)(void*, Mtx34*, Mtx44*))
       renderer::address::kCameraGetMatrices)(camera, &view, &projection);
  if (!IsTopScreenProjection(projection)) {
    cameras_refused++;
    return false;
  }
  view_ = view;
  projection_ = projection;
  has_camera_ = true;
  cameras_found++;
  return true;
}

bool WorldOverlay::Project(const Vec3& world, s32& x, s32& y) const {
  // The point in the space of the camera, then in the space of the screen.
  f32 view[4];
  for (u32 i = 0; i < 3; i++) {
    view[i] = view_.m[i][0] * world.x + view_.m[i][1] * world.y +
              view_.m[i][2] * world.z + view_.m[i][3];
  }
  view[3] = 1.0f;
  f32 clip[4];
  for (u32 i = 0; i < 4; i++) {
    clip[i] = projection_.m[i][0] * view[0] + projection_.m[i][1] * view[1] +
              projection_.m[i][2] * view[2] + projection_.m[i][3] * view[3];
  }
  if (clip[3] <= 0.001f) return false; // Behind the camera.

  // -1 to 1 on the screen.
  f32 screen_x;
  f32 screen_y;
  if (pivot == 0) {
    screen_x = -clip[1] / clip[3];
    screen_y = -clip[0] / clip[3];
  } else {
    screen_x = clip[0] / clip[3];
    screen_y = -clip[1] / clip[3];
  }
  x = (s32)((screen_x * 0.5f + 0.5f) * 400.0f);
  y = (s32)((screen_y * 0.5f + 0.5f) * 240.0f);
  return x >= 0 && x < 400 && y >= 0 && y < 240;
}

void WorldOverlay::DrawTop() {
  auto& self = GetInstance();
  const bool has_camera = self.has_camera_;
  // The next frame needs a new camera: the overworld can end at any time.
  self.has_camera_ = false;
  if (!self.is_enabled) return;
  if (!has_camera) {
    self.marks_drawn = 0;
    return;
  }
  self.camera_frames++;
  self.marks_drawn = 0;
  if (!core::ProcessManager::IsOverworldActive()) return;
  auto& menu = ui::MainApplication::GetInstance();
  if (menu.IsOpened() && !menu.IsWalking()) return;

  auto& player = ModelManager::GetInstance().GetPlayer();
  const f32 ground = player.world_pos.coords.y;
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);
  s32 x;
  s32 y;

  if (self.show_tiles) {
    TileEditor::Block blocks[TileEditor::kMaxBlocks];
    const u32 count = TileEditor::CollectBlocks(blocks, TileEditor::kMaxBlocks);
    s32 radius = self.radius;
    if (radius < 1) radius = 1;
    if (radius > (s32)kMaxRadius) radius = kMaxRadius;
    for (s32 dz = -radius; dz <= radius; dz++) {
      for (s32 dx = -radius; dx <= radius; dx++) {
        Tile tile;
        if (!TileEditor::Read(blocks, count, player_x + dx, player_z + dz,
                              &tile)) {
          continue;
        }
        // Only the tiles with a special attribute: fewer drawings.
        Color color;
        if (tile.is_impassable) {
          color = Color(1, 0.2f, 0.2f, 0.75f);
        } else if (tile.is_water) {
          color = Color(0.2f, 0.5f, 1, 0.75f);
        } else if (tile.permits_encounters) {
          color = Color(0.3f, 1, 0.3f, 0.75f);
        } else {
          continue;
        }
        const Vec3 center(TileCenter(player_x + dx), ground,
                          TileCenter(player_z + dz));
        if (self.Project(center, x, y)) {
          DrawMark(x, y, color);
          self.marks_drawn++;
        }
      }
    }
  }

  if (self.show_hidden_items) {
    const u32 map_id = MapManager::GetInstance().GetMapId();
    for (u32 i = 0; i < HiddenItem::kCount; i++) {
      const HiddenItem& item = HiddenItem::GetInstance(i);
      if (static_cast<u32>(item.map_id) != map_id) continue;
      const Vec3 center(TileCenter(item.tile_x), ground,
                        TileCenter(item.tile_z));
      if (self.Project(center, x, y)) {
        DrawMark(x, y, Color(1, 0.9f, 0.1f, 0.9f));
        self.marks_drawn++;
      }
    }
  }

  if (self.show_player && self.Project(player.world_pos.coords, x, y)) {
    DrawMark(x, y, Color(1, 1, 1, 0.9f));
    self.marks_drawn++;
  }
}

} // namespace overworld
