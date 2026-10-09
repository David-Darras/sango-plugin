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
core::Hook<void(void*, void*, u32, void*, u32)> draw_scene_hook;

// The value of the top screen (the left eye in 3D).
constexpr u32 kDisplayUpper = 0;
constexpr s32 kMarkSize = 6;

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
}

void WorldOverlay::DrawSceneHook(void* scene, void* graphics, u32 display,
                                 void* camera, u32 command_cache_dump) {
  auto& self = GetInstance();
  // Keep the first camera of the top screen in each frame: the camera of
  // the overworld. The other scenes of the frame (effects) come after it.
  if (display == kDisplayUpper && self.camera_ == nullptr) {
    self.camera_ = camera;
  }
  draw_scene_hook(scene, graphics, display, camera, command_cache_dump);
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
  void* const camera = self.camera_;
  // The next frame needs a new camera: the overworld can end at any time.
  self.camera_ = nullptr;
  if (!self.is_enabled || camera == nullptr) return;
  if (renderer::address::kCameraGetMatrices == 0) return;
  if (!core::ProcessManager::IsOverworldActive()) return;
  if (ui::MainApplication::GetInstance().IsOpened()) return;

  ((void (*)(void*, Mtx34*, Mtx44*))
       renderer::address::kCameraGetMatrices)(camera, &self.view_,
                                                       &self.projection_);

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
        if (self.Project(center, x, y)) DrawMark(x, y, color);
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
      }
    }
  }

  if (self.show_player && self.Project(player.world_pos.coords, x, y)) {
    DrawMark(x, y, Color(1, 1, 1, 0.9f));
  }
}

} // namespace overworld
