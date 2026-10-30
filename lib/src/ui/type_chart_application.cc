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
 * @file type_chart_application.cc
 * @brief The table of the type chart.
 *
 * The declarations are in ui/type_chart_application.h.
 */

#include "ui/type_chart_application.h"

#include "battle/constant/type_multiplier.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/type_chart.h"
#include "core/utils.h"
#include "pokemon/constant/type.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/touch_screen.h"
#include "ui/application_manager.h"
#include "ui/main_application.h"

namespace ui {
namespace {
constexpr u32 kGameTypeCount = 18;
// The space for the names of the types, on the left and at the top.
constexpr s32 kHeader = 22;
constexpr s32 kScreenSize = 240;

const battle::TypeMultiplier kValues[] = {
    battle::TypeMultiplier::k0, battle::TypeMultiplier::k05,
    battle::TypeMultiplier::k1, battle::TypeMultiplier::k2,
};
const c8* const kValueNames[] = {"x0", "x1/2", "x1", "x2"};

Color GetValueColor(u32 value_index) {
  switch (value_index) {
    case 0:
      return Color(0.04f, 0.04f, 0.06f, 1.0f);
    case 1:
      return Color(0.85f, 0.25f, 0.25f, 1.0f);
    case 3:
      return Color(0.25f, 0.80f, 0.30f, 1.0f);
    default:
      return Color(0.95f, 0.95f, 0.95f, 1.0f);
  }
}

u32 GetValueIndex(battle::TypeMultiplier value) {
  for (u32 i = 0; i < SIZE(kValues); i++) {
    if (kValues[i] == value) return i;
  }
  return 2;
}

// The initials of the types of the game: two letters, with the capital first.
const c8* const kGameInitials[kGameTypeCount] = {
    "No", "Fi", "Fl", "Po", "Gd", "Ro", "Bu", "Gh", "St",
    "Fr", "Wa", "Gr", "El", "Ps", "Ic", "Dr", "Dk", "Fa"};

// Writes the initials of a type of the list: for the types of the game, the
// table above; for a new type, the first two letters of its name (the first and
// the third letter when the two letters are already the initials of a type
// before it in the list).
void MakeInitials(const u8* types, u32 index, const c8* const* names,
                  c16* out) {
  auto make = [&](u32 type, u32 second, c16* text) {
    if (type < kGameTypeCount) {
      text[0] = static_cast<c16>(kGameInitials[type][0]);
      text[1] = static_cast<c16>(kGameInitials[type][1]);
    } else {
      const c8* name = names[type];
      const c8 first = name[0];
      const c8 other = name[0] != '\0' && name[1] != '\0' ? name[second] : '\0';
      text[0] = static_cast<c16>(first >= 'a' && first <= 'z' ? first - 'a' + 'A'
                                                              : first);
      text[1] = static_cast<c16>(other >= 'A' && other <= 'Z' ? other - 'A' + 'a'
                                                              : other);
    }
    text[2] = 0;
  };
  make(types[index], 1, out);
  for (u32 before = 0; before < index; before++) {
    c16 other[3];
    make(types[before], 1, other);
    if (other[0] == out[0] && other[1] == out[1] && types[index] >= kGameTypeCount) {
      make(types[index], 2, out);
      return;
    }
  }
}

// The table shows at most 11 rows and 11 columns: the drawings of one frame
// are limited, and a cell of 19 pixels or more is easy to touch.
constexpr s32 kMaxVisible = 11;
// The size of the side of a cell.
s32 GetCellSize(s32 visible) {
  return visible <= 0 ? 8 : (kScreenSize - kHeader) / visible;
}
} // namespace

void TypeChartApplication::MakeTypeList() {
  type_count_ = 0;
  for (u32 type = 0; type < kGameTypeCount; type++) {
    types_[type_count_++] = static_cast<u8>(type);
  }
  for (u32 type = kGameTypeCount + 1; type < kMaxTypes; type++) {
    if (battle::GameExtension::FindType(type) != nullptr) {
      types_[type_count_++] = static_cast<u8>(type);
    }
  }
}

void TypeChartApplication::Open() {
  auto& app = GetInstance();
  app.MakeTypeList();
  app.cursor_row_ = 0;
  app.cursor_column_ = 0;
  app.view_row_ = 0;
  app.view_column_ = 0;
  app.was_down_ = false;
  app.change_count_ = 0;
  ApplicationManager::GetInstance().Push(app);
}

void TypeChartApplication::GetOrigin(s32 visible, s32& x, s32& y) const {
  const s32 total = kHeader + visible * GetCellSize(visible);
  x = (320 - total) / 2;
  y = (kScreenSize - total) / 2;
}

s32 TypeChartApplication::GetVisibleCount() const {
  const s32 count = static_cast<s32>(type_count_);
  return count < kMaxVisible ? count : kMaxVisible;
}

void TypeChartApplication::ScrollToCursor() {
  const s32 visible = GetVisibleCount();
  const s32 last_start = static_cast<s32>(type_count_) - visible;
  if (cursor_column_ < view_column_) view_column_ = cursor_column_;
  if (cursor_column_ >= view_column_ + visible) {
    view_column_ = cursor_column_ - visible + 1;
  }
  if (cursor_row_ < view_row_) view_row_ = cursor_row_;
  if (cursor_row_ >= view_row_ + visible) view_row_ = cursor_row_ - visible + 1;
  if (view_column_ > last_start) view_column_ = last_start;
  if (view_row_ > last_start) view_row_ = last_start;
  if (view_column_ < 0) view_column_ = 0;
  if (view_row_ < 0) view_row_ = 0;
}

bool TypeChartApplication::GetCellAt(s32 touch_x, s32 touch_y, s32& row,
                                     s32& column) const {
  const s32 visible = GetVisibleCount();
  const s32 cell = GetCellSize(visible);
  s32 origin_x = 0;
  s32 origin_y = 0;
  GetOrigin(visible, origin_x, origin_y);
  const s32 x = touch_x - origin_x - kHeader;
  const s32 y = touch_y - origin_y - kHeader;
  if (x < 0 || y < 0 || x >= visible * cell || y >= visible * cell) {
    return false;
  }
  column = view_column_ + x / cell;
  row = view_row_ + y / cell;
  return true;
}

void TypeChartApplication::Change(s32 delta) {
  if (type_count_ == 0) return;
  const TypeId attacking = static_cast<TypeId>(types_[cursor_row_]);
  const TypeId defending = static_cast<TypeId>(types_[cursor_column_]);
  const s32 count = static_cast<s32>(SIZE(kValues));
  s32 index = static_cast<s32>(
      GetValueIndex(battle::TypeChart::Get(attacking, defending)));
  index = ((index + delta) % count + count) % count;
  battle::TypeChart::Set(attacking, defending, kValues[index]);
  change_count_++;
}

void TypeChartApplication::Update(sys::Controller& controller) {
  if (controller.IsKeyReleased(Key::kB)) {
    ApplicationManager::GetInstance().Pop();
    return;
  }
  if (type_count_ == 0) return;

  const struct {
    Key key;
    s32 dx;
    s32 dy;
  } moves[] = {{Key::kLeft, -1, 0}, {Key::kRight, 1, 0}, {Key::kUp, 0, -1},
               {Key::kDown, 0, 1}};
  for (const auto& move : moves) {
    if (!controller.IsKeyPressed(move.key) &&
        !controller.IsKeyRepeated(move.key)) {
      continue;
    }
    cursor_column_ += move.dx;
    cursor_row_ += move.dy;
  }
  const s32 last = static_cast<s32>(type_count_) - 1;
  if (cursor_column_ < 0) cursor_column_ = 0;
  if (cursor_row_ < 0) cursor_row_ = 0;
  if (cursor_column_ > last) cursor_column_ = last;
  if (cursor_row_ > last) cursor_row_ = last;
  ScrollToCursor();

  if (controller.IsKeyPressed(Key::kA)) Change(1);
  if (controller.IsKeyPressed(Key::kX)) Change(-1);
  if (controller.IsKeyPressed(Key::kY)) {
    // x1 for the cell.
    battle::TypeChart::Set(static_cast<TypeId>(types_[cursor_row_]),
                           static_cast<TypeId>(types_[cursor_column_]),
                           battle::TypeMultiplier::k1);
    change_count_++;
  }

  // A touch on a cell moves the cursor there and changes the value, one time
  // for each touch.
  sys::TouchScreen& touch = sys::TouchScreen::GetInstance();
  const bool is_down = touch.IsDown();
  s32 row = 0;
  s32 column = 0;
  if (is_down && !was_down_ &&
      GetCellAt(touch.GetX(), touch.GetY(), row, column)) {
    cursor_row_ = row;
    cursor_column_ = column;
    Change(1);
  }
  was_down_ = is_down;
}

void TypeChartApplication::DrawBottom(sys::Graphics& graphics) {
  // The GPU can draw a limited number of commands in one frame: the table shows
  // a part of the chart (the view), with few drawings. One rectangle for the
  // white cells (x1), then one rectangle for each run of cells of the same
  // colour in a row, then the thin lines between the cells. The table is in the
  // center of the screen, over a translucent black, as the top screen.
  sys::Graphics::DrawRect(0, 0, 320, 240, Color(0.0f, 0.0f, 0.0f, 0.6f));
  if (type_count_ == 0) return;
  const s32 visible = GetVisibleCount();
  const s32 cell = GetCellSize(visible);
  const s32 size = visible * cell;
  s32 origin_x = 0;
  s32 origin_y = 0;
  GetOrigin(visible, origin_x, origin_y);
  const s32 grid_x = origin_x + kHeader;
  const s32 grid_y = origin_y + kHeader;

  u32 name_count = 0;
  const c8** names = GetTypeNames(&name_count);
  (void)name_count;
  const Color letters(1.0f, 1.0f, 1.0f, 1.0f);
  // The initials of the types (white): two letters, as No, Fi, Fl.
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  for (s32 i = 0; i < visible; i++) {
    c16 initials[3];
    MakeInitials(types_, view_column_ + i, names, initials);
    sys::Graphics::DrawText(grid_x + i * cell + 3, origin_y + 4, initials,
                            letters);
    MakeInitials(types_, view_row_ + i, names, initials);
    sys::Graphics::DrawText(origin_x + 3, grid_y + i * cell + 4, initials,
                            letters);
  }

  sys::Graphics::DrawRect(grid_x, grid_y, size, size, GetValueColor(2));
  for (s32 row = 0; row < visible; row++) {
    s32 run_start = 0;
    u32 run_value = 2;
    for (s32 column = 0; column <= visible; column++) {
      u32 value = 2;
      if (column < visible) {
        value = GetValueIndex(battle::TypeChart::Get(
            static_cast<TypeId>(types_[view_row_ + row]),
            static_cast<TypeId>(types_[view_column_ + column])));
      }
      if (column > 0 && value == run_value && column < visible) continue;
      if (column > 0 && run_value != 2) {
        sys::Graphics::DrawRect(grid_x + run_start * cell, grid_y + row * cell,
                                (column - run_start) * cell, cell,
                                GetValueColor(run_value));
      }
      run_start = column;
      run_value = value;
    }
  }
  // The lines between the cells.
  const Color line(0.0f, 0.0f, 0.0f, 0.55f);
  for (s32 i = 0; i <= visible; i++) {
    sys::Graphics::DrawRect(grid_x + i * cell - 1, grid_y, 1, size, line);
    sys::Graphics::DrawRect(grid_x, grid_y + i * cell - 1, size, 1, line);
  }

  // The cursor: a frame of 2 pixels, in cyan with a black line around it, so it
  // shows on the white, the green, the red and the black cells.
  const s32 cursor_x = grid_x + (cursor_column_ - view_column_) * cell;
  const s32 cursor_y = grid_y + (cursor_row_ - view_row_) * cell;
  sys::Graphics::DrawRectStroke(cursor_x - 2, cursor_y - 2, cell + 3, cell + 3,
                                1, Color(0.0f, 0.0f, 0.0f, 1.0f));
  sys::Graphics::DrawRectStroke(cursor_x - 1, cursor_y - 1, cell + 1, cell + 1,
                                2, Color(0.0f, 0.9f, 1.0f, 1.0f));
}

void TypeChartApplication::DrawTop(sys::Graphics& graphics) {
  sys::Graphics::DrawRect(0, 0, 400, 240, Color(0.0f, 0.0f, 0.0f, 0.6f));
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  const Color text(1.0f, 1.0f, 1.0f, 1.0f);
  const Color muted(0.7f, 0.7f, 0.7f, 1.0f);
  sys::Graphics::DrawText(5, 5, u"Type Chart", Color(1.0f, 0.8f, 0.2f, 1.0f));
  if (type_count_ == 0) return;

  u32 name_count = 0;
  const c8** names = GetTypeNames(&name_count);
  const u32 attacking = types_[cursor_row_];
  const u32 defending = types_[cursor_column_];
  const auto value = battle::TypeChart::Get(static_cast<TypeId>(attacking),
                                            static_cast<TypeId>(defending));
  c16 line[96];
  core::Utils::Format(line, u"%s attacks %s: %s",
                      attacking < name_count ? names[attacking] : "?",
                      defending < name_count ? names[defending] : "?",
                      kValueNames[GetValueIndex(value)]);
  sys::Graphics::DrawText(5, 30, line, text);
  core::Utils::Format(line, u"Cells changed: %u", change_count_);
  sys::Graphics::DrawText(5, 52, line, muted);
  // The part of the table on the bottom screen.
  const s32 visible = GetVisibleCount();
  core::Utils::Format(line, u"View: rows %d-%d, columns %d-%d of %u",
                      view_row_ + 1, view_row_ + visible, view_column_ + 1,
                      view_column_ + visible, type_count_);
  sys::Graphics::DrawText(5, 74, line, muted);
  sys::Graphics::DrawText(5, 150,
                          u"Row: the attacking type.  Column: the defending "
                          u"type.",
                          muted);
  sys::Graphics::DrawText(5, 172,
                          u"Touch a cell: x0, x1/2, x1, x2, and again.",
                          muted);
  sys::Graphics::DrawText(5, 194,
                          u"D-pad move  A next  X back  Y x1  B close",
                          muted);
}
} // namespace ui
