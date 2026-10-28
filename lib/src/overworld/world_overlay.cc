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
#include "overworld/patch/map_character.h"
#include "overworld/patch/tile_editor.h"
#include "overworld/patch/trigger_editor.h"
#include "renderer/address.h"
#include "system/native/graphics.h"
#include "ui/main_application.h"

namespace overworld {

namespace {
core::Hook<void(void*, void*, void*, void*, s32, bool)> draw_scene_hook;
core::Hook<void(void*, void*, void*, bool)> stereo_draw_hook;

constexpr s32 kMarkSize = 6;
constexpr s32 kScreenWidth = 400;
constexpr s32 kScreenHeight = 240;
// The overlay draws about this number of strips for all the quads.
constexpr f32 kStripBudget = 450.0f;
constexpr u32 kMaxLabels = 20;
// A zone of an event works for a player within two tiles of height.
constexpr f32 kHeightWindow = 2.0f * WorldLayout::kUnitsPerTile;

// The kinds of tiles, from the most important to the least.
enum TileClass : u8 {
  kNone,
  kWall,
  kWater,
  kEncounter,
  kBlocker,
  kDecoration,
};

// The kinds of characters (event types of the game) that look for the player.
// A trainer sees along a line in front of it, as far as its range. The kind 2
// sees one tile in all the directions.
constexpr u16 kSeesAllSides = 2;
bool IsTrainerKind(u16 kind) {
  return kind == 1 || kind == 2 || kind == 3 || kind == 6 ||
         (kind >= 9 && kind <= 19) || kind == 21;
}

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

f32 Abs(f32 value) { return value < 0 ? -value : value; }

// The center of a tile in the world: a tile is 18 x 18 units, and the tile
// (0, 0) starts at the origin of the world.
f32 TileCenter(s32 tile) {
  return (tile + 0.5f) * WorldLayout::kUnitsPerTile;
}

void DrawMark(s32 x, s32 y, Color color) {
  sys::Graphics::DrawRect(x - kMarkSize / 2, y - kMarkSize / 2, kMarkSize,
                          kMarkSize, color);
}

void FormatNumber(u32 value, c16* out) {
  c16 digits[8];
  u32 count = 0;
  do {
    digits[count++] = static_cast<c16>(u'0' + value % 10);
    value /= 10;
  } while (value != 0 && count < 7);
  u32 length = 0;
  while (count != 0) out[length++] = digits[--count];
  out[length] = 0;
}

// On the top screen, a text uses a space of 512 x 256 and a rectangle uses
// 400 x 240 pixels. This draws a text at a pixel of the rectangles, centered
// on it when `is_centered` is true.
void DrawTextAtPixel(s32 x, s32 y, const c16* text, Color color,
                     bool is_centered) {
  s32 text_x = static_cast<s32>(static_cast<f32>(x) * 512.0f / 400.0f);
  const s32 text_y = static_cast<s32>(static_cast<f32>(y) * 256.0f / 240.0f);
  if (is_centered) text_x -= sys::Graphics::GetTextWidth(text) / 2;
  sys::Graphics::DrawText(text_x, text_y, text, color);
}

Color WithAlpha(f32 r, f32 g, f32 b, f32 alpha) { return Color(r, g, b, alpha); }

Color ColorOf(TileClass tile_class, f32 alpha) {
  switch (tile_class) {
    case kWall: return WithAlpha(1.0f, 0.2f, 0.2f, alpha);
    case kWater: return WithAlpha(0.2f, 0.5f, 1.0f, alpha);
    case kEncounter: return WithAlpha(0.3f, 1.0f, 0.3f, alpha);
    case kBlocker: return WithAlpha(1.0f, 0.6f, 0.1f, alpha);
    case kDecoration: return WithAlpha(0.7f, 0.3f, 1.0f, alpha);
    default: return WithAlpha(1.0f, 1.0f, 1.0f, alpha);
  }
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

bool WorldOverlay::ProjectPoint(const Vec3& world, ScreenPoint& out) const {
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
  out.x = (screen_x * 0.5f + 0.5f) * static_cast<f32>(kScreenWidth);
  out.y = (screen_y * 0.5f + 0.5f) * static_cast<f32>(kScreenHeight);
  return true;
}

bool WorldOverlay::Project(const Vec3& world, s32& x, s32& y) const {
  ScreenPoint point;
  if (!ProjectPoint(world, point)) return false;
  x = static_cast<s32>(point.x);
  y = static_cast<s32>(point.y);
  return x >= 0 && x < kScreenWidth && y >= 0 && y < kScreenHeight;
}

bool WorldOverlay::AddTileQuad(s32 x, s32 z, s32 width, s32 depth, f32 ground,
                               Color color) {
  if (quad_count_ >= kMaxQuads || width <= 0 || depth <= 0) return false;
  const f32 unit = static_cast<f32>(WorldLayout::kUnitsPerTile);
  const Vec3 corners[4] = {
      Vec3(x * unit, ground, z * unit),
      Vec3((x + width) * unit, ground, z * unit),
      Vec3((x + width) * unit, ground, (z + depth) * unit),
      Vec3(x * unit, ground, (z + depth) * unit),
  };
  Quad& quad = quads_[quad_count_];
  f32 min_x = 1e9f;
  f32 max_x = -1e9f;
  f32 min_y = 1e9f;
  f32 max_y = -1e9f;
  for (u32 i = 0; i < 4; i++) {
    if (!ProjectPoint(corners[i], quad.corner[i])) return false;
    const ScreenPoint& p = quad.corner[i];
    if (p.x < min_x) min_x = p.x;
    if (p.x > max_x) max_x = p.x;
    if (p.y < min_y) min_y = p.y;
    if (p.y > max_y) max_y = p.y;
  }
  // Out of the screen: nothing to draw.
  if (max_x < 0 || min_x > kScreenWidth || max_y < 0 || min_y > kScreenHeight) {
    return false;
  }
  quad.color = color;
  quad_count_++;
  return true;
}

void WorldOverlay::FlushQuads() {
  // The total height of the quads tells how many strips the overlay draws.
  f32 total = 0.0f;
  for (u32 i = 0; i < quad_count_; i++) {
    f32 min_y = 1e9f;
    f32 max_y = -1e9f;
    for (const ScreenPoint& p : quads_[i].corner) {
      if (p.y < min_y) min_y = p.y;
      if (p.y > max_y) max_y = p.y;
    }
    if (min_y < 0) min_y = 0;
    if (max_y > kScreenHeight) max_y = static_cast<f32>(kScreenHeight);
    if (max_y > min_y) total += max_y - min_y;
  }
  s32 step = 2;
  while (step < 8 && total / static_cast<f32>(step) > kStripBudget) step++;

  for (u32 i = 0; i < quad_count_; i++) {
    const Quad& quad = quads_[i];
    f32 min_y = 1e9f;
    f32 max_y = -1e9f;
    for (const ScreenPoint& p : quad.corner) {
      if (p.y < min_y) min_y = p.y;
      if (p.y > max_y) max_y = p.y;
    }
    s32 y_start = min_y < 0 ? 0 : static_cast<s32>(min_y);
    s32 y_end = max_y > kScreenHeight ? kScreenHeight : static_cast<s32>(max_y) + 1;
    for (s32 y = y_start; y < y_end; y += step) {
      const f32 center = static_cast<f32>(y) + static_cast<f32>(step) * 0.5f;
      f32 left = 1e9f;
      f32 right = -1e9f;
      for (u32 e = 0; e < 4; e++) {
        const ScreenPoint& a = quad.corner[e];
        const ScreenPoint& b = quad.corner[(e + 1) % 4];
        if ((a.y <= center && b.y > center) || (b.y <= center && a.y > center)) {
          const f32 x = a.x + (center - a.y) / (b.y - a.y) * (b.x - a.x);
          if (x < left) left = x;
          if (x > right) right = x;
        }
      }
      if (right < left) continue;
      if (left < 0) left = 0;
      if (right > kScreenWidth) right = static_cast<f32>(kScreenWidth);
      if (right - left < 1.0f) continue;
      const s32 height = y + step > kScreenHeight ? kScreenHeight - y : step;
      sys::Graphics::DrawRect(static_cast<s32>(left), y,
                              static_cast<s32>(right - left), height, quad.color);
      strips_drawn++;
    }
  }
  quad_count_ = 0;
}

void WorldOverlay::DrawTileLayer(s32 player_x, s32 player_z, f32 ground) {
  const bool wants_tiles = show_tiles || show_trainer_blockers ||
                           show_decoration_spots;
  if (!wants_tiles) return;
  TileEditor::Block blocks[TileEditor::kMaxBlocks];
  const u32 count = TileEditor::CollectBlocks(blocks, TileEditor::kMaxBlocks);
  s32 range = radius;
  if (range < 1) range = 1;
  if (range > static_cast<s32>(kMaxRadius)) range = kMaxRadius;
  const f32 alpha = (opacity < 10 ? 10 : opacity > 100 ? 100 : opacity) / 100.0f;

  auto classify = [&](s32 tile_x, s32 tile_z) -> TileClass {
    Tile tile;
    if (!TileEditor::Read(blocks, count, tile_x, tile_z, &tile)) return kNone;
    if (show_tiles) {
      if (tile.is_impassable) return kWall;
      if (tile.is_water) return kWater;
      if (tile.permits_encounters) return kEncounter;
    }
    if (show_trainer_blockers && tile.blocks_trainer_movement) return kBlocker;
    if (show_decoration_spots && tile.can_place_decorations) return kDecoration;
    return kNone;
  };

  for (s32 dz = -range; dz <= range; dz++) {
    TileClass run_class = kNone;
    s32 run_start = 0;
    for (s32 dx = -range; dx <= range + 1; dx++) {
      const TileClass tile_class =
          dx <= range ? classify(player_x + dx, player_z + dz) : kNone;
      if (fill_tiles) {
        if (tile_class != run_class) {
          if (run_class != kNone) {
            AddTileQuad(player_x + run_start, player_z + dz, dx - run_start, 1,
                        ground, ColorOf(run_class, alpha));
          }
          run_class = tile_class;
          run_start = dx;
        }
      } else if (tile_class != kNone && dx <= range) {
        s32 x;
        s32 y;
        const Vec3 center(TileCenter(player_x + dx), ground,
                          TileCenter(player_z + dz));
        if (Project(center, x, y)) {
          DrawMark(x, y, ColorOf(tile_class, 0.75f));
          marks_drawn++;
        }
      }
    }
  }
}

void WorldOverlay::DrawEventLayer(s32 player_x, s32 player_z, f32 ground) {
  const MapEventData* events = TriggerEditor::GetEvents();
  if (events == nullptr) return;
  s32 range = radius;
  if (range > static_cast<s32>(kMaxRadius)) range = kMaxRadius;
  const f32 alpha = (opacity < 10 ? 10 : opacity > 100 ? 100 : opacity) / 100.0f;

  // Adds the part of a zone that is near the player.
  auto add_zone = [&](s32 x, s32 z, s32 width, s32 depth, f32 height,
                      Color color) {
    if (Abs(height - ground) >= kHeightWindow) return;
    const s32 x0 = x > player_x - range ? x : player_x - range;
    const s32 z0 = z > player_z - range ? z : player_z - range;
    const s32 x1 = x + width < player_x + range + 1 ? x + width : player_x + range + 1;
    const s32 z1 = z + depth < player_z + range + 1 ? z + depth : player_z + range + 1;
    if (x1 <= x0 || z1 <= z0) return;
    AddTileQuad(x0, z0, x1 - x0, z1 - z0, ground, color);
  };

  if (show_signs) {
    for (u32 i = 0; i < TriggerEditor::GetSignCount(); i++) {
      const SignEvent* sign = TriggerEditor::GetSign(i);
      add_zone(sign->tiles.tile_x, sign->tiles.tile_z, sign->tiles.width,
               sign->tiles.depth, static_cast<f32>(sign->tiles.height),
               WithAlpha(0.1f, 0.9f, 1.0f, alpha));
    }
  }
  if (show_triggers) {
    for (u32 i = 0; i < TriggerEditor::GetTriggerCount(); i++) {
      const TriggerEvent* trigger = TriggerEditor::GetTrigger(i);
      const Color color = trigger->check_kind == 0
                              ? WithAlpha(1.0f, 0.2f, 1.0f, alpha)
                              : WithAlpha(1.0f, 0.55f, 0.75f, alpha);
      add_zone(trigger->tiles.tile_x, trigger->tiles.tile_z,
               trigger->tiles.width, trigger->tiles.depth,
               static_cast<f32>(trigger->tiles.height), color);
    }
  }
  if (show_warps && events->warps != nullptr) {
    const s32 unit = static_cast<s32>(WorldLayout::kUnitsPerTile);
    for (u32 i = 0; i < events->warp_count; i++) {
      const WarpEvent& warp = events->warps[i];
      if (warp.position_kind != 0) continue;
      const s32 width = (warp.world.width + unit - 1) / unit;
      const s32 depth = (warp.world.depth + unit - 1) / unit;
      add_zone(warp.world.x / unit, warp.world.z / unit, width < 1 ? 1 : width,
               depth < 1 ? 1 : depth, static_cast<f32>(warp.world.y),
               WithAlpha(0.7f, 1.0f, 0.1f, alpha));
    }
  }
}

void WorldOverlay::AddSightQuads(f32 ground) {
  ModelManager* manager = ModelManager::GetInstanceOrNull();
  if (manager == nullptr) return;
  const f32 alpha = (opacity < 10 ? 10 : opacity > 100 ? 100 : opacity) / 100.0f;
  TileEditor::Block blocks[TileEditor::kMaxBlocks];
  const u32 block_count = TileEditor::CollectBlocks(blocks, TileEditor::kMaxBlocks);

  for (u32 i = 0; i < ModelManager::kMaxModels; i++) {
    Model& model = manager->GetModel(i);
    if (!model.IsUsed() || model.id == ModelManager::kPlayerId) continue;
    const CharacterPlacement* placement = MapCharacter::FindPlacement(model.id);
    if (placement == nullptr || !IsTrainerKind(placement->interaction_kind)) {
      continue;
    }
    if (Abs(model.world_pos.coords.y - ground) >= kHeightWindow) continue;
    const bool sees_all = placement->interaction_kind == kSeesAllSides;
    const u32 sight = sees_all ? 1 : placement->param0;
    if (sight == 0 || sight > 20) continue;

    const s32 tile_x = static_cast<s32>(model.map_pos.coords.x);
    const s32 tile_z = static_cast<s32>(model.map_pos.coords.z);
    // The directions: all four, or the one where the trainer looks.
    s32 directions[4][2];
    u32 direction_count = 0;
    if (sees_all) {
      const s32 all[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
      for (u32 d = 0; d < 4; d++) {
        directions[d][0] = all[d][0];
        directions[d][1] = all[d][1];
      }
      direction_count = 4;
    } else {
      const f32 fx = model.facing_direction.x;
      const f32 fz = model.facing_direction.z;
      if (Abs(fx) < 0.1f && Abs(fz) < 0.1f) continue;
      if (Abs(fx) > Abs(fz)) {
        directions[0][0] = fx > 0 ? 1 : -1;
        directions[0][1] = 0;
      } else {
        directions[0][0] = 0;
        directions[0][1] = fz > 0 ? 1 : -1;
      }
      direction_count = 1;
    }

    for (u32 d = 0; d < direction_count; d++) {
      const s32 dx = directions[d][0];
      const s32 dz = directions[d][1];
      // The line stops at the first wall.
      s32 length = 0;
      while (length < static_cast<s32>(sight)) {
        Tile tile;
        const s32 x = tile_x + dx * (length + 1);
        const s32 z = tile_z + dz * (length + 1);
        if (TileEditor::Read(blocks, block_count, x, z, &tile) &&
            tile.is_impassable) {
          break;
        }
        length++;
      }
      if (length == 0) continue;
      const s32 x0 = dx > 0 ? tile_x + 1 : (dx < 0 ? tile_x - length : tile_x);
      const s32 z0 = dz > 0 ? tile_z + 1 : (dz < 0 ? tile_z - length : tile_z);
      AddTileQuad(x0, z0, dx != 0 ? length : 1, dz != 0 ? length : 1, ground,
                  WithAlpha(1.0f, 0.45f, 0.0f, alpha));
    }
  }
}

void WorldOverlay::DrawLabel(const Vec3& world, u32 number, Color color) {
  if (!show_script_numbers || label_count_ >= kMaxLabels) return;
  s32 x;
  s32 y;
  if (!Project(world, x, y)) return;
  c16 text[8];
  FormatNumber(number, text);
  DrawTextAtPixel(x, y - 16, text, color, true);
  label_count_++;
}

void WorldOverlay::DrawEventLabels(f32 ground) {
  if (!show_script_numbers) return;
  const f32 unit = static_cast<f32>(WorldLayout::kUnitsPerTile);
  if (show_signs) {
    for (u32 i = 0; i < TriggerEditor::GetSignCount(); i++) {
      const SignEvent* sign = TriggerEditor::GetSign(i);
      if (sign->id == TriggerEditor::kNoScript ||
          Abs(static_cast<f32>(sign->tiles.height) - ground) >= kHeightWindow) {
        continue;
      }
      const Vec3 center((sign->tiles.tile_x + sign->tiles.width * 0.5f) * unit,
                        ground,
                        (sign->tiles.tile_z + sign->tiles.depth * 0.5f) * unit);
      DrawLabel(center, sign->id, Color(0.6f, 1.0f, 1.0f, 1.0f));
    }
  }
  if (show_triggers) {
    for (u32 i = 0; i < TriggerEditor::GetTriggerCount(); i++) {
      const TriggerEvent* trigger = TriggerEditor::GetTrigger(i);
      if (trigger->id == TriggerEditor::kNoScript ||
          Abs(static_cast<f32>(trigger->tiles.height) - ground) >= kHeightWindow) {
        continue;
      }
      const Vec3 center(
          (trigger->tiles.tile_x + trigger->tiles.width * 0.5f) * unit, ground,
          (trigger->tiles.tile_z + trigger->tiles.depth * 0.5f) * unit);
      DrawLabel(center, trigger->id, Color(1.0f, 0.7f, 1.0f, 1.0f));
    }
  }
}

void WorldOverlay::DrawCharacterMarks(f32 ground) {
  ModelManager* manager = ModelManager::GetInstanceOrNull();
  if (manager == nullptr) return;
  for (u32 i = 0; i < ModelManager::kMaxModels; i++) {
    Model& model = manager->GetModel(i);
    if (!model.IsUsed() || model.id == ModelManager::kPlayerId) continue;
    if (Abs(model.world_pos.coords.y - ground) >= kHeightWindow) continue;
    const CharacterPlacement* placement = MapCharacter::FindPlacement(model.id);
    const bool is_trainer =
        placement != nullptr && IsTrainerKind(placement->interaction_kind);
    s32 x;
    s32 y;
    if (!Project(model.world_pos.coords, x, y)) continue;
    DrawMark(x, y, is_trainer ? Color(1.0f, 0.45f, 0.0f, 0.95f)
                              : Color(0.6f, 0.7f, 1.0f, 0.95f));
    marks_drawn++;
    if (placement != nullptr && static_cast<u32>(placement->script_id) != 0 &&
        show_script_numbers && label_count_ < kMaxLabels) {
      c16 text[8];
      FormatNumber(static_cast<u32>(placement->script_id), text);
      DrawTextAtPixel(x, y - 16, text, Color(1.0f, 0.85f, 0.6f, 1.0f), true);
      label_count_++;
    }
  }
}

void WorldOverlay::DrawLegend() {
  struct Entry {
    bool is_shown;
    Color color;
    const c16* text;
  };
  const Entry entries[] = {
      {show_tiles, ColorOf(kWall, 0.9f), u"Wall"},
      {show_tiles, ColorOf(kWater, 0.9f), u"Water"},
      {show_tiles, ColorOf(kEncounter, 0.9f), u"Wild Pokemon"},
      {show_trainer_blockers, ColorOf(kBlocker, 0.9f), u"Blocks trainers"},
      {show_decoration_spots, ColorOf(kDecoration, 0.9f), u"Decorations"},
      {show_signs, Color(0.1f, 0.9f, 1.0f, 0.9f), u"Sign"},
      {show_triggers, Color(1.0f, 0.2f, 1.0f, 0.9f), u"Trigger"},
      {show_warps, Color(0.7f, 1.0f, 0.1f, 0.9f), u"Warp"},
      {show_trainer_sight, Color(1.0f, 0.45f, 0.0f, 0.9f), u"Trainer sight"},
  };
  s32 y = kScreenHeight - 6;
  for (u32 i = sizeof(entries) / sizeof(entries[0]); i-- > 0;) {
    if (!entries[i].is_shown) continue;
    y -= 12;
    sys::Graphics::DrawRect(4, y + 2, 8, 8, entries[i].color);
    DrawTextAtPixel(16, y - 1, entries[i].text, Color(1, 1, 1, 1), false);
  }
}

void WorldOverlay::DrawTop() {
  auto& self = GetInstance();
  const bool has_camera = self.has_camera_;
  // The next frame needs a new camera: the overworld can end at any time.
  self.has_camera_ = false;
  if (!self.is_enabled) return;
  self.marks_drawn = 0;
  self.strips_drawn = 0;
  self.quad_count_ = 0;
  self.label_count_ = 0;
  if (!has_camera) return;
  self.camera_frames++;
  if (!core::ProcessManager::IsOverworldActive()) return;
  auto& menu = ui::MainApplication::GetInstance();
  if (menu.IsOpened() && !menu.IsWalking()) return;

  auto& player = ModelManager::GetInstance().GetPlayer();
  const f32 ground = player.world_pos.coords.y;
  const s32 player_x = static_cast<s32>(player.map_pos.coords.x);
  const s32 player_z = static_cast<s32>(player.map_pos.coords.z);

  // The tiles and the zones are quads: they come first, so that the marks
  // are over them.
  self.DrawTileLayer(player_x, player_z, ground);
  self.DrawEventLayer(player_x, player_z, ground);
  if (self.show_trainer_sight) self.AddSightQuads(ground);
  self.FlushQuads();

  s32 x;
  s32 y;
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
  if (self.show_characters) self.DrawCharacterMarks(ground);
  self.DrawEventLabels(ground);

  if (self.show_player && self.Project(player.world_pos.coords, x, y)) {
    DrawMark(x, y, Color(1, 1, 1, 0.9f));
    self.marks_drawn++;
  }
  if (self.show_legend) self.DrawLegend();
}

} // namespace overworld
