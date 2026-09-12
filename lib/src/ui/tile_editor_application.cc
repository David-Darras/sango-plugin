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

#include "ui/tile_editor_application.h"

#include "game/overworld/model_manager.h"
#include "system/device.h"
#include "system/graphics.h"
#include "ui/application_manager.h"
#include "utils.h"

namespace ui {
#include "game/overworld/tile.inc"

static const c8* PROPERTY_NAMES[] = {
    "Ground", "Footstep", "Battle Background", "Diagonal",
    // "Shadow",
    //   "Trainer Blocked", "Decorations", "Decoration Cursor", "Reflection 0",
    //   "Reflection 1", "Reflection 2", "Reflection 3"
};

static u32 GetProperty(const overworld::Tile& tile, u32 property) {
  switch (property) {
    case 0:
      return tile.ground_id;
    case 1:
      return tile.foot_step_id;
    case 2:
      return tile.battle_background_id;
    case 3:
      return tile.allows_diagonal_movement;
    default:
      return 0;
      // case 4: return tile.has_shadow;
      // case 5: return tile.blocks_trainer_movement;
      // case 6: return tile.can_place_decorations;
      // case 7: return tile.can_move_decoration_cursor;
      // case 8: return tile.has_reflection_0;
      // case 9: return tile.has_reflection_1;
      // case 10: return tile.has_reflection_2;
      // default: return tile.has_reflection_3;
  }
}

static u32 GetPropertyMax(u32 property) {
  switch (property) {
    case 0:
      return 0xFF;
    case 1:
      return 0x1F;
    case 2:
      return 0x7F;
    default:
      return 1;
  }
}

static void SetProperty(overworld::Tile& tile, u32 property, u32 value) {
  switch (property) {
    case 0:
      tile.ground_id = value;
      break;
    case 1:
      tile.foot_step_id = value;
      break;
    case 2:
      tile.battle_background_id = value;
      break;
    case 3:
      tile.allows_diagonal_movement = value;
      break;
    default:
      break;
  }
}

static overworld::Tile ToTile(u32 attr) {
  overworld::Tile tile;
  *(u32*)&tile = attr;
  return tile;
}

static u32 ToAttr(const overworld::Tile& tile) {
  return *(const u32*)&tile;
}

void TileEditorApplication::Open() {
  auto& app = GetInstance();
  auto& player = overworld::ModelManager::GetInstance().GetPlayer();
  app.cursor_x_ = static_cast<s32>(player.map_pos.coords.x);
  app.cursor_z_ = static_cast<s32>(player.map_pos.coords.z);
  app.saved_ = false;
  ApplicationManager::GetInstance().Push(app);
}

void TileEditorApplication::Update(Controller& controller) {
  if (controller.IsKeyReleased(Key::kB)) {
    ApplicationManager::GetInstance().Pop();
    return;
  }

  const struct {
    Key key;
    s32 dx;
    s32 dz;
  } moves[] = {
      {Key::kLeft, -1, 0}, {Key::kRight, 1, 0},
      {Key::kUp, 0, -1}, {Key::kDown, 0, 1}
  };
  for (const auto& move : moves) {
    if (!controller.IsKeyPressed(move.key) &&
        !controller.IsKeyRepeated(move.key)) {
      continue;
    }
    cursor_x_ += move.dx;
    cursor_z_ += move.dz;
  }
  auto& layout = overworld::WorldLayout::GetInstance();
  const s32 max_x = layout.width * overworld::WorldLayout::kTilesPerBlockSide -
                    1;
  const s32 max_z = layout.height * overworld::WorldLayout::kTilesPerBlockSide -
                    1;
  if (cursor_x_ < 0) cursor_x_ = 0;
  if (cursor_z_ < 0) cursor_z_ = 0;
  if (cursor_x_ > max_x) cursor_x_ = max_x;
  if (cursor_z_ > max_z) cursor_z_ = max_z;

  if (controller.IsKeyPressed(Key::kL)) {
    property_ = (property_ + kPropertyCount - 1) % kPropertyCount;
  }
  if (controller.IsKeyPressed(Key::kR)) {
    property_ = (property_ + 1) % kPropertyCount;
  }
  if (controller.IsKeyPressed(Key::kA) || controller.IsKeyRepeated(Key::kA)) {
    Adjust(1);
  }
  if (controller.IsKeyPressed(Key::kX) || controller.IsKeyRepeated(Key::kX)) {
    Adjust(-1);
  }
  if (controller.IsKeyPressed(Key::kY)) {
    feature::TileEditor::Block blocks[feature::TileEditor::kMaxBlocks];
    const u32 count = feature::TileEditor::CollectBlocks(
        blocks, feature::TileEditor::kMaxBlocks);
    has_brush_ = feature::TileEditor::ReadRaw(blocks, count, cursor_x_,
                                              cursor_z_, &brush_);
  }
  if (controller.IsKeyPressed(Key::kStart) ||
      controller.IsKeyRepeated(Key::kStart)) {
    Paint();
  }
  if (controller.IsKeyPressed(Key::kSelect)) {
    saved_ = feature::TileEditor::Save();
  }
}

void TileEditorApplication::Adjust(s32 delta) {
  feature::TileEditor::Block blocks[feature::TileEditor::kMaxBlocks];
  const u32 count = feature::TileEditor::CollectBlocks(
      blocks, feature::TileEditor::kMaxBlocks);
  u32 attr = 0;
  if (!feature::TileEditor::ReadRaw(blocks, count, cursor_x_, cursor_z_,
                                    &attr)) {
    return;
  }
  overworld::Tile tile = ToTile(attr);
  const s32 max = GetPropertyMax(property_);
  s32 value = static_cast<s32>(GetProperty(tile, property_)) + delta;
  if (value < 0) value = max;
  if (value > max) value = 0;
  SetProperty(tile, property_, value);
  feature::TileEditor::Write(cursor_x_, cursor_z_, ToAttr(tile));
  saved_ = false;
}

void TileEditorApplication::Paint() {
  if (!has_brush_) return;
  feature::TileEditor::Write(cursor_x_, cursor_z_, brush_);
  saved_ = false;
}

Color TileEditorApplication::ColorOf(const overworld::Tile& tile) const {
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

void TileEditorApplication::DrawBottom(Graphics& graphics) {
  feature::TileEditor::Block blocks[feature::TileEditor::kMaxBlocks];
  const u32 count = feature::TileEditor::CollectBlocks(
      blocks, feature::TileEditor::kMaxBlocks);
  const s32 origin_x = cursor_x_ - kColumns / 2;
  const s32 origin_z = cursor_z_ - kRows / 2;
  const Color unknown(0.0f, 0.0f, 0.0f, 1.0f);

  for (s32 row = 0; row < kRows; row++) {
    Color run_color = unknown;
    s32 run_start = 0;
    for (s32 column = 0; column <= kColumns; column++) {
      Color color = unknown;
      if (column < kColumns) {
        overworld::Tile tile;
        if (feature::TileEditor::Read(blocks, count, origin_x + column,
                                      origin_z + row, &tile)) {
          color = ColorOf(tile);
        }
      }
      const bool same = column < kColumns && color.r == run_color.r &&
                        color.g == run_color.g && color.b == run_color.b;
      if (column == 0) {
        run_color = color;
        continue;
      }
      if (same) continue;
      Graphics::DrawRect(run_start * kTileSize, row * kTileSize,
                         (column - run_start) * kTileSize, kTileSize,
                         run_color);
      run_start = column;
      run_color = color;
    }
  }

  auto& player = overworld::ModelManager::GetInstance().GetPlayer();
  const s32 player_column = static_cast<s32>(player.map_pos.coords.x) -
                            origin_x;
  const s32 player_row = static_cast<s32>(player.map_pos.coords.z) - origin_z;
  if (player_column >= 0 && player_column < kColumns && player_row >= 0 &&
      player_row < kRows) {
    Graphics::DrawRectStroke(player_column * kTileSize, player_row * kTileSize,
                             kTileSize, kTileSize, 2,
                             Color(1.0f, 0.2f, 0.2f, 1.0f));
  }
  Graphics::DrawRectStroke((kColumns / 2) * kTileSize, (kRows / 2) * kTileSize,
                           kTileSize, kTileSize, 1,
                           Color(1.0f, 1.0f, 1.0f, 1.0f));
}

void TileEditorApplication::DrawTop(Graphics& graphics) {
  feature::TileEditor::Block blocks[feature::TileEditor::kMaxBlocks];
  const u32 count = feature::TileEditor::CollectBlocks(
      blocks, feature::TileEditor::kMaxBlocks);
  u32 attr = 0;
  overworld::Tile seen;
  const bool known = feature::TileEditor::ReadRaw(blocks, count, cursor_x_,
                                                  cursor_z_, &attr) &&
                     feature::TileEditor::Read(blocks, count, cursor_x_,
                                               cursor_z_, &seen);
  const overworld::Tile raw = ToTile(attr);

  Graphics::DrawRect(0, 0, 400, 240, Color(0.0f, 0.0f, 0.0f, 0.6f));
  Graphics::SetTextScale(0.6f, 0.6f);
  const Color text(1.0f, 1.0f, 1.0f, 1.0f);
  const Color selected(0.0f, 1.0f, 1.0f, 1.0f);
  const Color muted(0.7f, 0.7f, 0.7f, 1.0f);
  c16 line[128];
  s32 y = 5;

  Utils::Format(line, u"Tile %d, %d   layout %u   edits %u%s", cursor_x_,
                cursor_z_, overworld::WorldLayout::GetInstance().id,
                feature::TileEditor::GetEditCount(),
                saved_ ? "  (saved)" : "");
  Graphics::DrawText(5, y, line, text);
  y += 17;

  const auto& report = feature::TileEditor::GetReport();
  Utils::Format(line,
                u"slots %u  blocks %u  captured %u  offsets %u/%u/%u/%u  %s",
                report.slots, report.blocks, report.captured,
                report.slots_offset, report.translate_offset,
                report.id_offset, report.pack_offset,
                report.slot_class != nullptr ? report.slot_class : "");
  Graphics::DrawText(5, 208, line, muted);
  if (known) {
    Utils::Format(line, u"raw %08X  seen %08X", attr,
                  *(const u32*)&seen);
    Graphics::DrawText(250, 5, line, muted);
  }
  if (!known) {
    Graphics::DrawText(5, y, u"No terrain under the cursor", muted);
  } else {
    Utils::Format(line, u"Walkable %s   Water %s   Encounters %s",
                  seen.is_impassable ? "no" : "yes",
                  seen.is_water ? "yes" : "no",
                  seen.permits_encounters ? "yes" : "no");
    Graphics::DrawText(5, y, line, muted);
  }
  y += 22;

  for (u32 i = 0; i < kPropertyCount; i++) {
    const u32 value = known ? GetProperty(raw, i) : 0;
    const c8* label = "";
    if (i == kGround && value < SIZE(GROUNDS)) label = GROUNDS[value];
    if (i == kFootstep && value < SIZE(FOOT_STEPS)) label = FOOT_STEPS[value];
    if (GetPropertyMax(i) == 1) label = value ? "yes" : "no";
    if (GetPropertyMax(i) == 1) {
      Utils::Format(line, u"%s %s: %s", i == property_ ? ">" : " ",
                    PROPERTY_NAMES[i], label);
    } else {
      Utils::Format(line, u"%s %s: %u %s", i == property_ ? ">" : " ",
                    PROPERTY_NAMES[i], value, label);
    }
    Graphics::DrawText(5, y, line, i == property_ ? selected : text);
    y += 14;
  }

  y += 6;
  if (has_brush_) {
    const overworld::Tile brush = ToTile(brush_);
    Utils::Format(line, u"Brush: ground %u %s", brush.ground_id,
                  brush.ground_id < SIZE(GROUNDS)
                    ? GROUNDS[brush.ground_id]
                    : "");
    Graphics::DrawText(5, y, line, muted);
  }
  Graphics::DrawText(5, 215,
                     u"D-pad move  L/R property  A/X change  Y pick\n"
                     u"START paint B close",
                     muted);
}
} // namespace ui