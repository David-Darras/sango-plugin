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
 * @file map_edit_events.cc
 * @brief The mode Events of the touch map editor: select, place, move and
 *        change the signs, the triggers, the warps and the characters.
 *
 * The declarations are in ui/map_edit_application.h.
 */

#include "ui/map_edit_application.h"

#include "core/utils.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
#include "overworld/patch/trigger_editor.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/touch_screen.h"

namespace ui {

namespace {
using overworld::CharacterPlacement;
using overworld::SignEvent;
using overworld::TriggerEditor;
using overworld::TriggerEvent;
using overworld::WarpEvent;

constexpr s32 kCanvasSize = 240;
constexpr s32 kUnit = static_cast<s32>(overworld::WorldLayout::kUnitsPerTile);
constexpr u32 kZoomCount = 5;
constexpr u32 kStepCount = 4;
const s32 kStepSizes[kStepCount] = {1, 10, 100, 1000};

struct Range {
  s32 min;
  s32 max;
};

const c8* const kSignNames[] = {"Script", "Tile X", "Tile Z", "Width",
                                "Depth",  "Height", "Talk from", "Kind"};
const Range kSignRanges[] = {{0, 65535}, {0, 2000}, {0, 2000}, {1, 64},
                             {1, 64},    {-4000, 4000}, {0, 6}, {0, 10}};

const c8* const kTriggerNames[] = {"Script", "Tile X", "Tile Z", "Width",
                                   "Depth",  "Height", "When",   "Work",
                                   "Value"};
const Range kTriggerRanges[] = {{0, 65535}, {0, 2000}, {0, 2000}, {1, 64},
                                {1, 64},    {-4000, 4000}, {0, 12},
                                {0, 65535}, {0, 65535}};

const c8* const kWarpNames[] = {"Destination map", "Destination exit",
                                "Tile X", "Tile Z", "Width", "Depth",
                                "Exit facing", "Exit kind"};
const Range kWarpRanges[] = {{0, 65535}, {0, 65535}, {0, 2000}, {0, 2000},
                             {1, 64},    {1, 64},    {0, 3},    {0, 255}};

const c8* const kCharacterNames[] = {"Tile X", "Tile Z", "Facing", "Script",
                                     "Movement", "Kind", "Trainer range",
                                     "Parameter 1", "Parameter 2",
                                     "Hide flag", "Model"};
const Range kCharacterRanges[] = {{0, 2000}, {0, 2000}, {0, 3}, {0, 65535},
                                  {0, 65535}, {0, 65535}, {0, 65535},
                                  {0, 65535}, {0, 65535}, {0, 65535},
                                  {0, 65535}};

s32 Clamp(s32 value, const Range& range) {
  if (value < range.min) return range.min;
  if (value > range.max) return range.max;
  return value;
}

// A trainer sees along a line: its kind is one of these (see the world
// overlay).
bool IsTrainerKind(u16 kind) {
  return kind == 1 || kind == 2 || kind == 3 || kind == 6 ||
         (kind >= 9 && kind <= 19) || kind == 21;
}
} // namespace

u32 MapEditApplication::GetFieldCount() const {
  switch (selected_kind_) {
    case kSignEvent:
      return sizeof(kSignNames) / sizeof(kSignNames[0]);
    case kTriggerEvent:
      return sizeof(kTriggerNames) / sizeof(kTriggerNames[0]);
    case kWarpEvent:
      return sizeof(kWarpNames) / sizeof(kWarpNames[0]);
    case kCharacterEvent:
      return sizeof(kCharacterNames) / sizeof(kCharacterNames[0]);
    default:
      return 0;
  }
}

s32 MapEditApplication::GetField(u32 field) const {
  switch (selected_kind_) {
    case kSignEvent: {
      const SignEvent* s = TriggerEditor::GetSign(selected_index_);
      if (s == nullptr) return 0;
      switch (field) {
        case 0: return s->id;
        case 1: return s->tiles.tile_x;
        case 2: return s->tiles.tile_z;
        case 3: return s->tiles.width;
        case 4: return s->tiles.depth;
        case 5: return s->tiles.height;
        case 6: return static_cast<u8>(s->talk_facing);
        default: return s->kind;
      }
    }
    case kTriggerEvent: {
      const TriggerEvent* t = TriggerEditor::GetTrigger(selected_index_);
      if (t == nullptr) return 0;
      switch (field) {
        case 0: return t->id;
        case 1: return t->tiles.tile_x;
        case 2: return t->tiles.tile_z;
        case 3: return t->tiles.width;
        case 4: return t->tiles.depth;
        case 5: return t->tiles.height;
        case 6: return t->check_kind;
        case 7: return t->work_id;
        default: return t->param;
      }
    }
    case kWarpEvent: {
      const WarpEvent* w = TriggerEditor::GetWarp(selected_index_);
      if (w == nullptr) return 0;
      switch (field) {
        case 0: return static_cast<u16>(w->map_id);
        case 1: return w->exit_id;
        case 2: return w->world.x / kUnit;
        case 3: return w->world.z / kUnit;
        case 4: return (w->world.width + kUnit - 1) / kUnit;
        case 5: return (w->world.depth + kUnit - 1) / kUnit;
        case 6: return static_cast<u8>(w->exit_facing);
        default: return w->exit_kind;
      }
    }
    case kCharacterEvent: {
      const CharacterPlacement* c = TriggerEditor::GetCharacter(selected_index_);
      if (c == nullptr) return 0;
      switch (field) {
        case 0: return c->position.tile_x;
        case 1: return c->position.tile_z;
        case 2: return static_cast<u8>(c->facing);
        case 3: return static_cast<s32>(static_cast<u32>(c->script_id));
        case 4: return c->movement_id;
        case 5: return c->interaction_kind;
        case 6: return c->param0;
        case 7: return c->param1;
        case 8: return c->param2;
        case 9: return c->hide_when_flag_set;
        default: return static_cast<s32>(static_cast<u32>(c->model_id));
      }
    }
    default:
      return 0;
  }
}

void MapEditApplication::SetField(u32 field, s32 value) {
  switch (selected_kind_) {
    case kSignEvent: {
      SignEvent* s = TriggerEditor::GetSign(selected_index_);
      if (s == nullptr || field >= GetFieldCount()) return;
      value = Clamp(value, kSignRanges[field]);
      switch (field) {
        case 0: s->id = static_cast<u16>(value); break;
        case 1: s->tiles.tile_x = static_cast<s16>(value); break;
        case 2: s->tiles.tile_z = static_cast<s16>(value); break;
        case 3: s->tiles.width = static_cast<u16>(value); break;
        case 4: s->tiles.depth = static_cast<u16>(value); break;
        case 5: s->tiles.height = value; break;
        case 6: s->talk_facing = static_cast<overworld::Facing>(value); break;
        default: s->kind = static_cast<u16>(value); break;
      }
      break;
    }
    case kTriggerEvent: {
      TriggerEvent* t = TriggerEditor::GetTrigger(selected_index_);
      if (t == nullptr || field >= GetFieldCount()) return;
      value = Clamp(value, kTriggerRanges[field]);
      switch (field) {
        case 0: t->id = static_cast<u16>(value); break;
        case 1: t->tiles.tile_x = static_cast<u16>(value); break;
        case 2: t->tiles.tile_z = static_cast<u16>(value); break;
        case 3: t->tiles.width = static_cast<u16>(value); break;
        case 4: t->tiles.depth = static_cast<u16>(value); break;
        case 5: t->tiles.height = static_cast<s16>(value); break;
        case 6: t->check_kind = static_cast<u16>(value); break;
        case 7: t->work_id = static_cast<u16>(value); break;
        default: t->param = static_cast<u16>(value); break;
      }
      break;
    }
    case kWarpEvent: {
      WarpEvent* w = TriggerEditor::GetWarp(selected_index_);
      if (w == nullptr || field >= GetFieldCount()) return;
      value = Clamp(value, kWarpRanges[field]);
      switch (field) {
        case 0: w->map_id = static_cast<overworld::MapId>(value); break;
        case 1: w->exit_id = static_cast<u16>(value); break;
        case 2: w->world.x = static_cast<s16>(value * kUnit); break;
        case 3: w->world.z = static_cast<s16>(value * kUnit); break;
        case 4: w->world.width = static_cast<u16>(value * kUnit); break;
        case 5: w->world.depth = static_cast<u16>(value * kUnit); break;
        case 6: w->exit_facing = static_cast<overworld::Facing>(value); break;
        default: w->exit_kind = static_cast<u8>(value); break;
      }
      break;
    }
    case kCharacterEvent: {
      CharacterPlacement* c = TriggerEditor::GetCharacter(selected_index_);
      if (c == nullptr || field >= GetFieldCount()) return;
      value = Clamp(value, kCharacterRanges[field]);
      switch (field) {
        case 0: c->position.tile_x = static_cast<u16>(value); break;
        case 1: c->position.tile_z = static_cast<u16>(value); break;
        case 2: c->facing = static_cast<overworld::Facing>(value); break;
        case 3: c->script_id = static_cast<script::ScriptId>(value); break;
        case 4: c->movement_id = static_cast<u16>(value); break;
        case 5: c->interaction_kind = static_cast<u16>(value); break;
        case 6: c->param0 = static_cast<u16>(value); break;
        case 7: c->param1 = static_cast<u16>(value); break;
        case 8: c->param2 = static_cast<u16>(value); break;
        case 9: c->hide_when_flag_set = static_cast<u16>(value); break;
        default: c->model_id = static_cast<overworld::ModelId>(value); break;
      }
      break;
    }
    default:
      break;
  }
}

void MapEditApplication::ChangeField(s32 delta) {
  const u32 count = GetFieldCount();
  if (count == 0 || selected_field_ >= count) return;
  SetField(selected_field_,
           GetField(selected_field_) + delta * kStepSizes[step_index_]);
  is_events_used_ = true;
}

void MapEditApplication::MoveSelectedTo(s32 x, s32 z) {
  if (selected_kind_ == kNoEvent) return;
  // The fields 1 and 2 are the tile for the zones, 0 and 1 for a character,
  // 2 and 3 for a warp.
  switch (selected_kind_) {
    case kSignEvent:
    case kTriggerEvent:
      SetField(1, x);
      SetField(2, z);
      break;
    case kWarpEvent:
      SetField(2, x);
      SetField(3, z);
      break;
    case kCharacterEvent:
      SetField(0, x);
      SetField(1, z);
      break;
    default:
      break;
  }
  is_events_used_ = true;
}

bool MapEditApplication::SelectAt(s32 x, s32 z) {
  if (!TriggerEditor::IsActive()) return false;
  // The characters come first, then the warps, the triggers and the signs.
  for (u32 i = 0; i < TriggerEditor::GetCharacterCount(); i++) {
    const CharacterPlacement* c = TriggerEditor::GetCharacter(i);
    if (c != nullptr && c->position.tile_x == x && c->position.tile_z == z) {
      selected_kind_ = kCharacterEvent;
      selected_index_ = i;
      selected_field_ = 0;
      return true;
    }
  }
  for (u32 i = 0; i < TriggerEditor::GetWarpCount(); i++) {
    const WarpEvent* w = TriggerEditor::GetWarp(i);
    const s32 x0 = w->world.x / kUnit;
    const s32 z0 = w->world.z / kUnit;
    const s32 width = (w->world.width + kUnit - 1) / kUnit;
    const s32 depth = (w->world.depth + kUnit - 1) / kUnit;
    if (x >= x0 && x < x0 + (width < 1 ? 1 : width) && z >= z0 &&
        z < z0 + (depth < 1 ? 1 : depth)) {
      selected_kind_ = kWarpEvent;
      selected_index_ = i;
      selected_field_ = 0;
      return true;
    }
  }
  for (u32 i = 0; i < TriggerEditor::GetTriggerCount(); i++) {
    const TriggerEvent* t = TriggerEditor::GetTrigger(i);
    if (x >= t->tiles.tile_x && x < t->tiles.tile_x + t->tiles.width &&
        z >= t->tiles.tile_z && z < t->tiles.tile_z + t->tiles.depth) {
      selected_kind_ = kTriggerEvent;
      selected_index_ = i;
      selected_field_ = 0;
      return true;
    }
  }
  for (u32 i = 0; i < TriggerEditor::GetSignCount(); i++) {
    const SignEvent* s = TriggerEditor::GetSign(i);
    if (x >= s->tiles.tile_x && x < s->tiles.tile_x + s->tiles.width &&
        z >= s->tiles.tile_z && z < s->tiles.tile_z + s->tiles.depth) {
      selected_kind_ = kSignEvent;
      selected_index_ = i;
      selected_field_ = 0;
      return true;
    }
  }
  return false;
}

void MapEditApplication::PlaceAt(s32 x, s32 z) {
  if (!TriggerEditor::IsActive()) {
    SetMessage(u"No events");
    return;
  }
  bool is_added = false;
  switch (place_kind_) {
    case 0:
      is_added = TriggerEditor::AddSign() != nullptr;
      selected_kind_ = kSignEvent;
      selected_index_ = TriggerEditor::GetSignCount() - 1;
      break;
    case 1:
      is_added = TriggerEditor::AddTrigger() != nullptr;
      selected_kind_ = kTriggerEvent;
      selected_index_ = TriggerEditor::GetTriggerCount() - 1;
      break;
    default:
      is_added = TriggerEditor::AddWarp() != nullptr;
      selected_kind_ = kWarpEvent;
      selected_index_ = TriggerEditor::GetWarpCount() - 1;
      break;
  }
  if (!is_added) {
    selected_kind_ = kNoEvent;
    SetMessage(u"Full");
    return;
  }
  selected_field_ = 0;
  MoveSelectedTo(x, z);
  SetMessage(u"Added");
}

void MapEditApplication::RemoveAt(s32 x, s32 z) {
  if (!SelectAt(x, z)) return;
  bool is_removed = false;
  switch (selected_kind_) {
    case kSignEvent:
      is_removed = TriggerEditor::RemoveSign(selected_index_);
      break;
    case kTriggerEvent:
      is_removed = TriggerEditor::RemoveTrigger(selected_index_);
      break;
    case kWarpEvent:
      is_removed = TriggerEditor::RemoveWarp(selected_index_);
      break;
    default:
      break;
  }
  if (is_removed) {
    selected_kind_ = kNoEvent;
    is_events_used_ = true;
    SetMessage(u"Removed");
  } else {
    SetMessage(u"Added only");
  }
}

void MapEditApplication::ReloadMap() {
  const overworld::Position& position =
      overworld::ModelManager::GetInstance().GetPlayer().world_pos;
  overworld::MapManager::ChangeMap(overworld::MapManager::GetInstance().GetMap(),
                                   position, overworld::Facing::kUp, true,
                                   false);
}

void MapEditApplication::Navigate(sys::Controller& controller) {
  if ((IsPressed(kZoomOut) || controller.IsKeyPressed(Key::kL)) &&
      zoom_index_ + 1u < kZoomCount) {
    zoom_index_++;
  }
  if ((IsPressed(kZoomIn) || controller.IsKeyPressed(Key::kR)) &&
      zoom_index_ > 0) {
    zoom_index_--;
  }
  if (IsPressed(kHome)) GoHome();
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
}

void MapEditApplication::UpdateEvents(sys::Controller& controller) {
  Navigate(controller);

  if (IsPressed(kToolButton)) {
    event_tool_ = static_cast<EventTool>((event_tool_ + 1) % kEventToolCount);
  }
  if (IsPressed(kAttributeButton)) place_kind_ = (place_kind_ + 1) % 3;
  const u32 field_count = GetFieldCount();
  if (field_count != 0) {
    if (IsPressed(kSizeDown)) {
      selected_field_ = (selected_field_ + field_count - 1) % field_count;
    }
    if (IsPressed(kSizeUp)) selected_field_ = (selected_field_ + 1) % field_count;
  }

  // The value changes at each frame while the button is down, after a delay.
  const bool value_down = buttons_[kValueDown].IsDown();
  const bool value_up = buttons_[kValueUp].IsDown();
  if (value_down || value_up) {
    value_hold_++;
    if (value_hold_ == 1 || (value_hold_ > 20 && value_hold_ % 3 == 0)) {
      ChangeField(value_up ? 1 : -1);
    }
  } else {
    value_hold_ = 0;
  }

  if (IsPressed(kUndo)) step_index_ = (step_index_ + 1) % kStepCount;
  if (IsPressed(kRedo)) {
    SetMessage(u"Reloading");
    ReloadMap();
  }
  if (IsPressed(kSave)) {
    SetMessage(TriggerEditor::Save() ? u"Saved" : u"Error");
  }

  sys::TouchScreen& touch = sys::TouchScreen::GetInstance();
  const bool is_down = touch.IsDown();
  s32 tile_x = 0;
  s32 tile_z = 0;
  const bool is_on_canvas =
      is_down && GetTileAt(touch.GetX(), touch.GetY(), tile_x, tile_z);
  if (is_on_canvas) {
    cursor_x_ = tile_x;
    cursor_z_ = tile_z;
    if (!was_down_) {
      switch (event_tool_) {
        case kSelect:
          if (!SelectAt(tile_x, tile_z)) selected_kind_ = kNoEvent;
          break;
        case kPlace:
          PlaceAt(tile_x, tile_z);
          break;
        case kMoveEvent:
          MoveSelectedTo(tile_x, tile_z);
          break;
        case kRemove:
          RemoveAt(tile_x, tile_z);
          break;
        default:
          break;
      }
    } else if (event_tool_ == kMoveEvent) {
      MoveSelectedTo(tile_x, tile_z);
    }
  }
  was_down_ = is_on_canvas;
  ClampView();
}

void MapEditApplication::DrawEventMarkers(s32 origin_x, s32 origin_z, s32 cell,
                                          s32 count) {
  if (!TriggerEditor::IsActive()) return;
  const s32 limit = count * cell;
  // Draws the frame of a zone of tiles, cut at the borders of the map.
  auto frame = [&](s32 x, s32 z, s32 width, s32 depth, Color color,
                   s32 thickness) {
    s32 left = (x - origin_x) * cell;
    s32 top = (z - origin_z) * cell;
    s32 right = left + width * cell;
    s32 bottom = top + depth * cell;
    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > limit) right = limit;
    if (bottom > limit) bottom = limit;
    if (right - left < 2 || bottom - top < 2) return;
    sys::Graphics::DrawRectStroke(left, top, right - left, bottom - top,
                                  thickness, color);
  };

  for (u32 i = 0; i < TriggerEditor::GetSignCount(); i++) {
    const SignEvent* s = TriggerEditor::GetSign(i);
    frame(s->tiles.tile_x, s->tiles.tile_z, s->tiles.width, s->tiles.depth,
          Color(0.1f, 0.9f, 1.0f, 1.0f), 1);
  }
  for (u32 i = 0; i < TriggerEditor::GetTriggerCount(); i++) {
    const TriggerEvent* t = TriggerEditor::GetTrigger(i);
    frame(t->tiles.tile_x, t->tiles.tile_z, t->tiles.width, t->tiles.depth,
          Color(1.0f, 0.2f, 1.0f, 1.0f), 1);
  }
  for (u32 i = 0; i < TriggerEditor::GetWarpCount(); i++) {
    const WarpEvent* w = TriggerEditor::GetWarp(i);
    const s32 width = (w->world.width + kUnit - 1) / kUnit;
    const s32 depth = (w->world.depth + kUnit - 1) / kUnit;
    frame(w->world.x / kUnit, w->world.z / kUnit, width < 1 ? 1 : width,
          depth < 1 ? 1 : depth, Color(0.7f, 1.0f, 0.1f, 1.0f), 1);
  }
  for (u32 i = 0; i < TriggerEditor::GetCharacterCount(); i++) {
    const CharacterPlacement* c = TriggerEditor::GetCharacter(i);
    const Color color = IsTrainerKind(c->interaction_kind)
                            ? Color(1.0f, 0.5f, 0.0f, 1.0f)
                            : Color(0.5f, 0.6f, 1.0f, 1.0f);
    frame(c->position.tile_x, c->position.tile_z, 1, 1, color, 2);
  }

  // The selected event: a yellow frame.
  const Color selected(1.0f, 0.95f, 0.1f, 1.0f);
  switch (selected_kind_) {
    case kSignEvent:
      if (const SignEvent* s = TriggerEditor::GetSign(selected_index_)) {
        frame(s->tiles.tile_x, s->tiles.tile_z, s->tiles.width, s->tiles.depth,
              selected, 2);
      }
      break;
    case kTriggerEvent:
      if (const TriggerEvent* t = TriggerEditor::GetTrigger(selected_index_)) {
        frame(t->tiles.tile_x, t->tiles.tile_z, t->tiles.width, t->tiles.depth,
              selected, 2);
      }
      break;
    case kWarpEvent:
      if (const WarpEvent* w = TriggerEditor::GetWarp(selected_index_)) {
        const s32 width = (w->world.width + kUnit - 1) / kUnit;
        const s32 depth = (w->world.depth + kUnit - 1) / kUnit;
        frame(w->world.x / kUnit, w->world.z / kUnit, width < 1 ? 1 : width,
              depth < 1 ? 1 : depth, selected, 2);
      }
      break;
    case kCharacterEvent:
      if (const CharacterPlacement* c =
              TriggerEditor::GetCharacter(selected_index_)) {
        frame(c->position.tile_x, c->position.tile_z, 1, 1, selected, 2);
      }
      break;
    default:
      break;
  }
}

void MapEditApplication::DrawEventsTop() {
  static const c8* const kToolNames[] = {"Select", "Place", "Move", "Remove"};
  static const c8* const kPlaceNames[] = {"sign", "trigger", "warp"};
  static const c8* const kKindNames[] = {"", "Sign", "Trigger", "Warp",
                                         "Character"};

  sys::Graphics::DrawRect(0, 0, 400, 240, Color(0.0f, 0.0f, 0.0f, 0.6f));
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  const Color text(1.0f, 1.0f, 1.0f, 1.0f);
  const Color muted(0.7f, 0.7f, 0.7f, 1.0f);
  const Color accent(0.0f, 1.0f, 1.0f, 1.0f);
  c16 line[128];
  s32 y = 5;

  core::Utils::Format(line, u"Events   Tool %s%s   Step %d", kToolNames[event_tool_],
                      event_tool_ == kPlace ? kPlaceNames[place_kind_] : "",
                      static_cast<int>(kStepSizes[step_index_]));
  sys::Graphics::DrawText(5, y, line, text);
  y += 18;

  if (!TriggerEditor::IsActive()) {
    sys::Graphics::DrawText(5, y, u"The events of this map are not loaded.",
                            muted);
  } else {
    core::Utils::Format(line, u"Signs %d  Triggers %d  Warps %d  Characters %d",
                        static_cast<int>(TriggerEditor::GetSignCount()),
                        static_cast<int>(TriggerEditor::GetTriggerCount()),
                        static_cast<int>(TriggerEditor::GetWarpCount()),
                        static_cast<int>(TriggerEditor::GetCharacterCount()));
    sys::Graphics::DrawText(5, y, line, muted);
    y += 18;
    const u32 count = GetFieldCount();
    if (selected_kind_ == kNoEvent || count == 0) {
      sys::Graphics::DrawText(5, y, u"Touch an event to select it.", muted);
    } else {
      bool is_added = false;
      switch (selected_kind_) {
        case kSignEvent:
          is_added = selected_index_ >= TriggerEditor::GetShippedSignCount();
          break;
        case kTriggerEvent:
          is_added = selected_index_ >= TriggerEditor::GetShippedTriggerCount();
          break;
        case kWarpEvent:
          is_added = selected_index_ >= TriggerEditor::GetShippedWarpCount();
          break;
        default:
          break;
      }
      core::Utils::Format(line, u"%s %d%s", kKindNames[selected_kind_],
                          static_cast<int>(selected_index_ + 1),
                          is_added ? " (added)" : "");
      sys::Graphics::DrawText(5, y, line, accent);
      y += 16;
      const c8* const* names = kSignNames;
      switch (selected_kind_) {
        case kTriggerEvent: names = kTriggerNames; break;
        case kWarpEvent: names = kWarpNames; break;
        case kCharacterEvent: names = kCharacterNames; break;
        default: break;
      }
      for (u32 f = 0; f < count; f++) {
        core::Utils::Format(line, u"%s %s: %d", f == selected_field_ ? ">" : " ",
                            names[f], static_cast<int>(GetField(f)));
        sys::Graphics::DrawText(5, y, line, f == selected_field_ ? accent : text);
        y += 14;
      }
    }
  }
  sys::Graphics::DrawText(5, 190,
                          u"Field-/+ choose a field. Val-/+ change it.", muted);
  sys::Graphics::DrawText(5, 208,
                          u"A character changes when the map loads: Reload.",
                          muted);
}

} // namespace ui
