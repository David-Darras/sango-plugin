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
 * @file map_edit_application.cc
 * @brief A touch editor for the attributes of the tiles of a world.
 *
 * The declarations are in ui/map_edit_application.h.
 */

#include "ui/map_edit_application.h"

#include <cstring>

#include "core/utils.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/tile.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/map_editor.h"
#include "overworld/patch/trigger_editor.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/touch_screen.h"
#include "ui/application_manager.h"
#include "ui/theme.h"

namespace ui {
#include "overworld/data/tile.inc"

namespace {
using overworld::MapEditor;
using overworld::Tile;

// The bottom screen: the map on the left, the buttons on the right.
constexpr s32 kCanvasSize = 240;
constexpr s32 kPanelX = 246;
constexpr s32 kButtonHeight = 22;
constexpr s32 kInfoY = 188;

const s32 kCellSizes[] = {4, 6, 8, 12, 16};
const u8 kBrushSizes[] = {1, 2, 3, 5, 9};
constexpr u32 kZoomCount = sizeof(kCellSizes) / sizeof(kCellSizes[0]);
constexpr u32 kBrushCount = sizeof(kBrushSizes) / sizeof(kBrushSizes[0]);

// The fill works in a window around the first tile, with a limit of tiles.
constexpr s32 kFillRadius = 40;
constexpr u32 kMaxFillTiles = 4000;
constexpr u32 kMaxRectSide = 200;

struct ButtonSpec {
  s32 x;
  s32 y;
  s32 width;
};

const ButtonSpec kButtonSpecs[] = {
    {246, 4, 70},   // Tool
    {246, 30, 70},  // Attribute
    {246, 56, 34},  // Val-
    {282, 56, 34},  // Val+
    {246, 82, 34},  // Size-
    {282, 82, 34},  // Size+
    {246, 108, 34}, // Zoom-
    {282, 108, 34}, // Zoom+
    {246, 134, 34}, // Undo
    {282, 134, 34}, // Redo
    {246, 160, 34}, // Home
    {282, 160, 34}, // Save
    {282, 212, 34}, // Done
    {246, 212, 34}, // Mode
};

Tile ToTile(u32 attr) {
  Tile tile;
  memcpy(&tile, &attr, sizeof(attr));
  return tile;
}

u32 ToAttr(const Tile& tile) {
  u32 attr;
  memcpy(&attr, &tile, sizeof(attr));
  return attr;
}

Color ColorOf(const Tile& tile) {
  if (tile.is_impassable) return Color(0.22f, 0.22f, 0.28f, 1.0f);
  if (tile.is_water) return Color(0.2f, 0.45f, 0.95f, 1.0f);
  if (tile.permits_encounters) return Color(0.2f, 0.7f, 0.25f, 1.0f);
  switch (tile.ground_id) {
    case 0x07:
      return Color(0.55f, 0.75f, 0.95f, 1.0f);
    case 0x02:
    case 0x09:
    case 0x2B:
      return Color(0.9f, 0.8f, 0.5f, 1.0f);
    default:
      return Color(0.82f, 0.82f, 0.72f, 1.0f);
  }
}

bool IsSameColor(const Color& a, const Color& b) {
  return a.r == b.r && a.g == b.g && a.b == b.b;
}

s32 Abs(s32 value) { return value < 0 ? -value : value; }
} // namespace

void MapEditApplication::Open() {
  auto& app = GetInstance();
  for (u32 i = 0; i < kButtonCount; i++) {
    app.buttons_[i].Initialize(kButtonSpecs[i].x, kButtonSpecs[i].y,
                               kButtonSpecs[i].width, kButtonHeight);
    app.buttons_[i].Reset();
  }
  app.GoHome();
  app.cursor_x_ = app.view_x_;
  app.cursor_z_ = app.view_z_;
  app.was_down_ = false;
  app.message_ = nullptr;
  app.message_frames_ = 0;
  app.is_full_ = false;
  ApplicationManager::GetInstance().Push(app);
}

void MapEditApplication::GoHome() {
  auto& player = overworld::ModelManager::GetInstance().GetPlayer();
  view_x_ = static_cast<s32>(player.map_pos.coords.x);
  view_z_ = static_cast<s32>(player.map_pos.coords.z);
  ClampView();
}

void MapEditApplication::ClampView() {
  auto& layout = overworld::WorldLayout::GetInstance();
  const s32 max_x =
      static_cast<s32>(layout.width) * overworld::WorldLayout::kTilesPerBlockSide - 1;
  const s32 max_z =
      static_cast<s32>(layout.height) * overworld::WorldLayout::kTilesPerBlockSide - 1;
  if (view_x_ > max_x) view_x_ = max_x;
  if (view_z_ > max_z) view_z_ = max_z;
  if (view_x_ < 0) view_x_ = 0;
  if (view_z_ < 0) view_z_ = 0;
}

void MapEditApplication::SetMessage(const c16* text) {
  message_ = text;
  message_frames_ = 90;
}

void MapEditApplication::Close() {
  auto& editor = MapEditor::GetInstance();
  if (editor.auto_save && MapEditor::IsDirty()) MapEditor::Save();
  if (is_events_used_) overworld::TriggerEditor::Save();
  ApplicationManager::GetInstance().Pop();
}

bool MapEditApplication::IsPressed(ButtonId id) const {
  return buttons_[id].IsReleased();
}

bool MapEditApplication::UsesValue() const {
  return attribute_ == kGround || attribute_ == kFootstep ||
         attribute_ == kBattleBackground;
}

u32 MapEditApplication::GetValueMax() const {
  switch (attribute_) {
    case kGround:
      return 0xFF;
    case kFootstep:
      return 0x1F;
    case kBattleBackground:
      return 0x7F;
    default:
      return 0;
  }
}

u32 MapEditApplication::Compute(u32 old) const {
  Tile tile = ToTile(old);
  switch (attribute_) {
    // The flags wall, water and wild Pokemon come from the ground number.
    case kWall:
      return MapEditor::SetLowFlags(old, 1);
    case kWalkable:
      return MapEditor::SetLowFlags(old, 0);
    case kWater:
      return MapEditor::SetLowFlags(old, 2);
    case kWildOn:
      return MapEditor::SetLowFlags(old, (MapEditor::DecodeAttr(old) & 2u) | 4u);
    case kWildOff:
      return MapEditor::SetLowFlags(old, MapEditor::DecodeAttr(old) & 3u);
    case kBlockTrainersOn:
      tile.blocks_trainer_movement = 1;
      break;
    case kBlockTrainersOff:
      tile.blocks_trainer_movement = 0;
      break;
    case kDecorationOn:
      tile.can_place_decorations = 1;
      tile.can_move_decoration_cursor = 1;
      break;
    case kDecorationOff:
      tile.can_place_decorations = 0;
      tile.can_move_decoration_cursor = 0;
      break;
    case kGround:
      tile.ground_id = value_;
      break;
    case kFootstep:
      tile.foot_step_id = value_;
      break;
    case kBattleBackground:
      tile.battle_background_id = value_;
      break;
    case kStamp:
      return has_stamp_ ? stamp_ : old;
    default:
      break;
  }
  return ToAttr(tile);
}

bool MapEditApplication::ApplyToTile(s32 x, s32 z) {
  u32 old;
  if (!MapEditor::ReadAttr(x, z, old)) return true; // The tile is not loaded.
  const u32 attr = Compute(old);
  if (attr == old) return true;
  if (!MapEditor::SetTile(x, z, attr)) return !MapEditor::IsFull();
  return true;
}

void MapEditApplication::Stamp(s32 x, s32 z) {
  const s32 size = kBrushSizes[size_index_];
  const s32 half = size / 2;
  for (s32 dz = 0; dz < size; dz++) {
    for (s32 dx = 0; dx < size; dx++) {
      if (!ApplyToTile(x - half + dx, z - half + dz)) {
        is_full_ = true;
        return;
      }
    }
  }
}

void MapEditApplication::Stroke(s32 from_x, s32 from_z, s32 to_x, s32 to_z) {
  const s32 steps = Abs(to_x - from_x) > Abs(to_z - from_z)
                        ? Abs(to_x - from_x)
                        : Abs(to_z - from_z);
  for (s32 i = 1; i <= steps; i++) {
    Stamp(from_x + (to_x - from_x) * i / steps,
          from_z + (to_z - from_z) * i / steps);
  }
}

void MapEditApplication::FillArea(s32 x, s32 z) {
  u32 target;
  if (!MapEditor::ReadAttr(x, z, target)) return;
  constexpr s32 kSide = 2 * kFillRadius + 1;
  static u8 visited[(kSide * kSide + 7) / 8];
  static u32 stack[4096];
  memset(visited, 0, sizeof(visited));
  u32 top = 0;
  stack[top++] = (static_cast<u32>(x) << 16) | static_cast<u32>(z);
  u32 changed = 0;
  while (top != 0 && changed < kMaxFillTiles) {
    const u32 packed = stack[--top];
    const s32 tile_x = static_cast<s32>(packed >> 16);
    const s32 tile_z = static_cast<s32>(packed & 0xFFFF);
    const s32 dx = tile_x - x + kFillRadius;
    const s32 dz = tile_z - z + kFillRadius;
    if (dx < 0 || dz < 0 || dx >= kSide || dz >= kSide) continue;
    const u32 bit = static_cast<u32>(dx + dz * kSide);
    if (visited[bit / 8] & (1u << (bit % 8))) continue;
    visited[bit / 8] |= static_cast<u8>(1u << (bit % 8));
    u32 attr;
    if (!MapEditor::ReadAttr(tile_x, tile_z, attr) || attr != target) continue;
    if (!ApplyToTile(tile_x, tile_z)) {
      is_full_ = true;
      return;
    }
    changed++;
    if (top + 4 <= sizeof(stack) / sizeof(stack[0])) {
      const s32 next[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
      for (const auto& n : next) {
        const s32 nx = tile_x + n[0];
        const s32 nz = tile_z + n[1];
        if (nx < 0 || nz < 0) continue;
        stack[top++] = (static_cast<u32>(nx) << 16) | static_cast<u32>(nz);
      }
    }
  }
}

void MapEditApplication::ApplyRect(s32 x0, s32 z0, s32 x1, s32 z1) {
  if (x1 < x0) { const s32 t = x0; x0 = x1; x1 = t; }
  if (z1 < z0) { const s32 t = z0; z0 = z1; z1 = t; }
  if (static_cast<u32>(x1 - x0) >= kMaxRectSide) x1 = x0 + kMaxRectSide - 1;
  if (static_cast<u32>(z1 - z0) >= kMaxRectSide) z1 = z0 + kMaxRectSide - 1;
  for (s32 z = z0; z <= z1; z++) {
    for (s32 x = x0; x <= x1; x++) {
      if (!ApplyToTile(x, z)) {
        is_full_ = true;
        return;
      }
    }
  }
}

void MapEditApplication::Pick(s32 x, s32 z) {
  u32 attr;
  if (!MapEditor::ReadAttr(x, z, attr)) return;
  stamp_ = attr;
  has_stamp_ = true;
  attribute_ = kStamp;
  tool_ = kPaint;
  value_ = ToTile(attr).ground_id;
  SetMessage(u"Picked");
}

bool MapEditApplication::GetTileAt(s32 touch_x, s32 touch_y, s32& tile_x,
                                   s32& tile_z) const {
  if (touch_x < 0 || touch_x >= kCanvasSize || touch_y < 0 ||
      touch_y >= kCanvasSize) {
    return false;
  }
  const s32 cell = kCellSizes[zoom_index_];
  const s32 count = kCanvasSize / cell;
  tile_x = view_x_ - count / 2 + touch_x / cell;
  tile_z = view_z_ - count / 2 + touch_y / cell;
  return true;
}

void MapEditApplication::Update(sys::Controller& controller) {
  if (message_frames_ != 0 && --message_frames_ == 0) message_ = nullptr;
  if (controller.IsKeyPressed(Key::kB) || controller.IsKeyPressed(Key::kStart)) {
    Close();
    return;
  }
  for (u32 i = 0; i < kButtonCount; i++) buttons_[i].Update();
  if (IsPressed(kDone)) {
    Close();
    return;
  }
  if (IsPressed(kModeButton)) {
    MapEditor::EndStroke();
    was_down_ = false;
    mode_ = mode_ == kTiles ? kEvents : kTiles;
    SetMessage(mode_ == kEvents ? u"Events" : u"Tiles");
  }
  if (mode_ == kEvents) {
    UpdateEvents(controller);
    return;
  }

  if (IsPressed(kToolButton)) tool_ = static_cast<Tool>((tool_ + 1) % kToolCount);
  if (IsPressed(kAttributeButton)) {
    attribute_ = static_cast<Attribute>((attribute_ + 1) % kAttributeCount);
    if (value_ > GetValueMax()) value_ = GetValueMax();
  }

  // The value changes at each frame while the button is down, after a delay.
  const bool value_down = buttons_[kValueDown].IsDown();
  const bool value_up = buttons_[kValueUp].IsDown();
  if (value_down || value_up) {
    value_hold_++;
    if (value_hold_ == 1 || (value_hold_ > 20 && value_hold_ % 3 == 0)) {
      if (UsesValue()) {
        if (value_up && value_ < GetValueMax()) value_++;
        if (value_down && value_ > 0) value_--;
      }
    }
  } else {
    value_hold_ = 0;
  }

  if (IsPressed(kSizeDown) && size_index_ > 0) size_index_--;
  if (IsPressed(kSizeUp) && size_index_ + 1u < kBrushCount) size_index_++;
  if ((IsPressed(kZoomOut) || controller.IsKeyPressed(Key::kL)) &&
      zoom_index_ + 1u < kZoomCount) {
    zoom_index_++;
  }
  if ((IsPressed(kZoomIn) || controller.IsKeyPressed(Key::kR)) &&
      zoom_index_ > 0) {
    zoom_index_--;
  }
  if (IsPressed(kUndo)) SetMessage(MapEditor::Undo() ? u"Undone" : u"Nothing");
  if (IsPressed(kRedo)) SetMessage(MapEditor::Redo() ? u"Redone" : u"Nothing");
  if (IsPressed(kHome)) GoHome();
  if (IsPressed(kSave)) SetMessage(MapEditor::Save() ? u"Saved" : u"Error");

  const struct {
    Key key;
    s32 dx;
    s32 dz;
  } moves[] = {{Key::kLeft, -1, 0}, {Key::kRight, 1, 0},
               {Key::kUp, 0, -1}, {Key::kDown, 0, 1}};
  for (const auto& move : moves) {
    if (controller.IsKeyPressed(move.key) || controller.IsKeyRepeated(move.key)) {
      view_x_ += move.dx;
      view_z_ += move.dz;
    }
  }

  sys::TouchScreen& touch = sys::TouchScreen::GetInstance();
  const bool is_down = touch.IsDown();
  const s32 touch_x = touch.GetX();
  const s32 touch_y = touch.GetY();
  s32 tile_x = 0;
  s32 tile_z = 0;
  const bool is_on_canvas =
      is_down && GetTileAt(touch_x, touch_y, tile_x, tile_z);

  if (is_on_canvas) {
    cursor_x_ = tile_x;
    cursor_z_ = tile_z;
    if (!was_down_) {
      // The stylus touches the map.
      switch (tool_) {
        case kPaint:
          MapEditor::BeginStroke();
          Stamp(tile_x, tile_z);
          last_x_ = tile_x;
          last_z_ = tile_z;
          break;
        case kRect:
          start_x_ = tile_x;
          start_z_ = tile_z;
          break;
        case kFill:
          MapEditor::BeginStroke();
          FillArea(tile_x, tile_z);
          MapEditor::EndStroke();
          break;
        case kPick:
          Pick(tile_x, tile_z);
          break;
        case kMove:
          anchor_view_x_ = view_x_;
          anchor_view_z_ = view_z_;
          anchor_touch_x_ = touch_x;
          anchor_touch_y_ = touch_y;
          break;
        default:
          break;
      }
    } else if (tool_ == kPaint && (tile_x != last_x_ || tile_z != last_z_)) {
      Stroke(last_x_, last_z_, tile_x, tile_z);
      last_x_ = tile_x;
      last_z_ = tile_z;
    } else if (tool_ == kMove) {
      const s32 cell = kCellSizes[zoom_index_];
      view_x_ = anchor_view_x_ - (touch_x - anchor_touch_x_) / cell;
      view_z_ = anchor_view_z_ - (touch_y - anchor_touch_y_) / cell;
    }
  } else if (was_down_) {
    // The stylus leaves the map.
    if (tool_ == kPaint) {
      MapEditor::EndStroke();
    } else if (tool_ == kRect) {
      MapEditor::BeginStroke();
      ApplyRect(start_x_, start_z_, cursor_x_, cursor_z_);
      MapEditor::EndStroke();
    }
  }
  was_down_ = is_on_canvas;

  if (is_full_) {
    is_full_ = false;
    SetMessage(u"Full: save");
  }
  ClampView();
}

void MapEditApplication::DrawTop(sys::Graphics& graphics) {
  if (mode_ == kEvents) {
    DrawEventsTop();
    return;
  }
  uint32_t attr = 0;
  const bool is_known = MapEditor::ReadAttr(cursor_x_, cursor_z_, attr);
  const Tile tile = ToTile(MapEditor::DecodeAttr(attr));

  sys::Graphics::DrawRect(0, 0, 400, 240, Color(0.0f, 0.0f, 0.0f, 0.6f));
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  const Color text(1.0f, 1.0f, 1.0f, 1.0f);
  const Color muted(0.7f, 0.7f, 0.7f, 1.0f);
  const Color accent(0.0f, 1.0f, 1.0f, 1.0f);
  c16 line[128];
  s32 y = 5;

  core::Utils::Format(line, u"Tile %d, %d   World %d   Runs %d   Tiles %d%s",
                      cursor_x_, cursor_z_,
                      static_cast<int>(overworld::WorldLayout::GetInstance().id),
                      static_cast<int>(MapEditor::GetRunCount()),
                      static_cast<int>(MapEditor::GetTileCount()),
                      MapEditor::IsDirty() ? "   (not saved)" : "");
  sys::Graphics::DrawText(5, y, line, text);
  y += 20;

  if (!is_known) {
    sys::Graphics::DrawText(5, y, u"No terrain under the stylus", muted);
  } else {
    core::Utils::Format(line, u"Walkable %s   Water %s   Wild Pokemon %s",
                        tile.is_impassable ? "no" : "yes",
                        tile.is_water ? "yes" : "no",
                        tile.permits_encounters ? "yes" : "no");
    sys::Graphics::DrawText(5, y, line, text);
    y += 16;
    core::Utils::Format(line, u"Blocks trainers %s   Decoration %s",
                        tile.blocks_trainer_movement ? "yes" : "no",
                        tile.can_place_decorations ? "yes" : "no");
    sys::Graphics::DrawText(5, y, line, text);
    y += 16;
    core::Utils::Format(line, u"Ground %u %s", tile.ground_id,
                        tile.ground_id < SIZE(GROUNDS) ? GROUNDS[tile.ground_id]
                                                       : "");
    sys::Graphics::DrawText(5, y, line, muted);
    y += 14;
    core::Utils::Format(line, u"Footstep %u %s   Battle background %u",
                        tile.foot_step_id,
                        tile.foot_step_id < SIZE(FOOT_STEPS)
                            ? FOOT_STEPS[tile.foot_step_id]
                            : "",
                        tile.battle_background_id);
    sys::Graphics::DrawText(5, y, line, muted);
    y += 14;
    core::Utils::Format(line, u"Raw %08X", attr);
    sys::Graphics::DrawText(5, y, line, muted);
  }

  static const c8* const kToolNames[] = {"Paint", "Rect", "Fill", "Pick",
                                         "Move"};
  static const c8* const kAttributeNames[] = {
      "Wall",       "Walkable",       "Water",         "Wild Pokemon on",
      "Wild Pokemon off", "Blocks trainers on", "Blocks trainers off",
      "Decoration on", "Decoration off", "Ground",    "Footstep",
      "Battle background", "Stamp (picked tile)"};
  core::Utils::Format(line, u"Tool %s   Attribute %s%s   Brush %d x %d",
                      kToolNames[tool_], kAttributeNames[attribute_],
                      UsesValue() ? " (value)" : "",
                      static_cast<int>(kBrushSizes[size_index_]),
                      static_cast<int>(kBrushSizes[size_index_]));
  sys::Graphics::DrawText(5, 190, line, accent);
  sys::Graphics::DrawText(5, 208,
                          u"Stylus paints. D-pad scrolls. L/R zoom. B closes.",
                          muted);
}

void MapEditApplication::DrawPanel() {
  const Theme& theme = Theme::GetInstance();
  // A solid ground: the buttons stay readable over the screen of the game.
  sys::Graphics::DrawRect(kPanelX - 4, 0, 320 - kPanelX + 4, 240,
                          Color(0.08f, 0.08f, 0.1f, 1.0f));
  // The buttons.
  static const c16* const kToolLabels[] = {u"Paint", u"Rect", u"Fill", u"Pick",
                                           u"Move"};
  static const c16* const kAttributeLabels[] = {
      u"Wall",      u"Walkable",   u"Water",     u"Wild +",   u"Wild -",
      u"Block Tr +", u"Block Tr -", u"Deco +",    u"Deco -",   u"Ground",
      u"Footstep",  u"Battle BG",  u"Stamp"};
  static const c16* const kEventToolLabels[] = {u"Select", u"Place", u"Move",
                                                u"Remove"};
  static const c16* const kPlaceLabels[] = {u"Sign", u"Trigger", u"Warp"};
  const bool is_events = mode_ == kEvents;
  const c16* labels[kButtonCount] = {
      is_events ? kEventToolLabels[event_tool_] : kToolLabels[tool_],
      is_events ? kPlaceLabels[place_kind_] : kAttributeLabels[attribute_],
      u"Val-",  u"Val+", is_events ? u"Field-" : u"Size-",
      is_events ? u"Field+" : u"Size+", u"Zoom-", u"Zoom+",
      is_events ? u"Step" : u"Undo", is_events ? u"Reload" : u"Redo",
      u"Home",  u"Save",  u"Done", is_events ? u"Events" : u"Tiles"};
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  for (u32 i = 0; i < kButtonCount; i++) {
    const Button& button = buttons_[i];
    const bool is_down = button.IsDown();
    Color fill = is_down ? theme.selected_text_color
                         : theme.unselected_text_color;
    fill.a = is_down ? 0.7f : 0.15f;
    sys::Graphics::DrawRect(button.GetX(), button.GetY(), button.GetWidth(),
                            button.GetHeight(), fill);
    const s32 text_width = sys::Graphics::GetTextWidth(labels[i]);
    sys::Graphics::DrawText(button.GetX() +
                                (static_cast<s32>(button.GetWidth()) -
                                 text_width) / 2,
                            button.GetY() + 5, labels[i],
                            theme.unselected_text_color);
  }

  sys::Graphics::SetTextScale(0.4f, 0.4f);
  if (message_ != nullptr) {
    sys::Graphics::DrawText(kPanelX, kInfoY, message_, theme.selected_text_color);
  } else {
    c16 line[32];
    if (mode_ == kEvents) {
      static const int kSteps[] = {1, 10, 100, 1000};
      core::Utils::Format(line, u"Step %d", kSteps[step_index_]);
    } else if (UsesValue()) {
      core::Utils::Format(line, u"Value %d  Size %d", static_cast<int>(value_),
                          static_cast<int>(kBrushSizes[size_index_]));
    } else {
      core::Utils::Format(line, u"Size %d", static_cast<int>(kBrushSizes[size_index_]));
    }
    sys::Graphics::DrawText(kPanelX, kInfoY, line, theme.unselected_text_color);
  }
}

void MapEditApplication::DrawBottom(sys::Graphics& graphics) {
  const Theme& theme = Theme::GetInstance();
  sys::Graphics::FillScreen(theme.background_color);
  DrawPanel();

  const s32 cell = kCellSizes[zoom_index_];
  const s32 count = kCanvasSize / cell;
  const s32 origin_x = view_x_ - count / 2;
  const s32 origin_z = view_z_ - count / 2;
  // The tiles are drawn as rectangles that do not overlap. A group of tiles of
  // the same color in a row is a span, and a span that has the same place and
  // color in the next row becomes taller: a big area of the same ground costs
  // one rectangle, and the number of drawings stays small.
  struct Span {
    s32 start;
    s32 end;
    s32 top;
    Color color;
    bool is_matched;
  };
  constexpr u32 kMaxSpans = 64;
  const Color unknown(0.0f, 0.0f, 0.0f, 1.0f);
  Span active[kMaxSpans];
  u32 active_count = 0;
  for (s32 row = 0; row <= count; row++) {
    Span current[kMaxSpans];
    u32 current_count = 0;
    if (row < count) {
      s32 run_start = 0;
      Color run_color = unknown;
      for (s32 column = 0; column <= count; column++) {
        Color color = unknown;
        if (column < count) {
          uint32_t attr;
          if (MapEditor::ReadAttr(origin_x + column, origin_z + row, attr)) {
            color = ColorOf(ToTile(MapEditor::DecodeAttr(attr)));
          }
        }
        if (column == 0) {
          run_color = color;
          continue;
        }
        if (column < count && IsSameColor(color, run_color)) continue;
        if (current_count < kMaxSpans) {
          Span& span = current[current_count++];
          span.start = run_start;
          span.end = column;
          span.top = row;
          span.color = run_color;
          span.is_matched = false;
        }
        run_start = column;
        run_color = color;
      }
    }
    // A span of the row above that goes on in this row grows.
    for (u32 i = 0; i < current_count; i++) {
      for (u32 j = 0; j < active_count; j++) {
        if (!active[j].is_matched && active[j].start == current[i].start &&
            active[j].end == current[i].end &&
            IsSameColor(active[j].color, current[i].color)) {
          current[i].top = active[j].top;
          active[j].is_matched = true;
          break;
        }
      }
    }
    // The other spans of the row above end here: draw them.
    for (u32 j = 0; j < active_count; j++) {
      if (active[j].is_matched) continue;
      sys::Graphics::DrawRect(active[j].start * cell, active[j].top * cell,
                              (active[j].end - active[j].start) * cell,
                              (row - active[j].top) * cell, active[j].color);
    }
    for (u32 i = 0; i < current_count; i++) active[i] = current[i];
    active_count = current_count;
  }

  // The borders of the blocks of 40 tiles.
  const s32 tiles = overworld::WorldLayout::kTilesPerBlockSide;
  const Color border(1.0f, 1.0f, 1.0f, 0.3f);
  for (s32 i = 0; i < count; i++) {
    if ((origin_x + i) % tiles == 0 && origin_x + i >= 0) {
      sys::Graphics::DrawRect(i * cell, 0, 1, count * cell, border);
    }
    if ((origin_z + i) % tiles == 0 && origin_z + i >= 0) {
      sys::Graphics::DrawRect(0, i * cell, count * cell, 1, border);
    }
  }

  auto& player = overworld::ModelManager::GetInstance().GetPlayer();
  const s32 player_column = static_cast<s32>(player.map_pos.coords.x) - origin_x;
  const s32 player_row = static_cast<s32>(player.map_pos.coords.z) - origin_z;
  if (player_column >= 0 && player_column < count && player_row >= 0 &&
      player_row < count) {
    sys::Graphics::DrawRectStroke(player_column * cell, player_row * cell, cell,
                                  cell, 1, Color(1.0f, 0.2f, 0.2f, 1.0f));
  }
  const s32 cursor_column = cursor_x_ - origin_x;
  const s32 cursor_row = cursor_z_ - origin_z;
  if (cursor_column >= 0 && cursor_column < count && cursor_row >= 0 &&
      cursor_row < count) {
    sys::Graphics::DrawRectStroke(cursor_column * cell, cursor_row * cell, cell,
                                  cell, 1, Color(1.0f, 1.0f, 1.0f, 1.0f));
  }
  if (mode_ == kEvents) DrawEventMarkers(origin_x, origin_z, cell, count);
  if (mode_ == kTiles && tool_ == kRect && was_down_) {
    const s32 x0 = (start_x_ < cursor_x_ ? start_x_ : cursor_x_) - origin_x;
    const s32 z0 = (start_z_ < cursor_z_ ? start_z_ : cursor_z_) - origin_z;
    const s32 x1 = (start_x_ < cursor_x_ ? cursor_x_ : start_x_) - origin_x;
    const s32 z1 = (start_z_ < cursor_z_ ? cursor_z_ : start_z_) - origin_z;
    sys::Graphics::DrawRectStroke(x0 * cell, z0 * cell, (x1 - x0 + 1) * cell,
                                  (z1 - z0 + 1) * cell, 2,
                                  Color(1.0f, 0.9f, 0.1f, 1.0f));
  }
}

} // namespace ui
