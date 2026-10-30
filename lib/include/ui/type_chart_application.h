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
 * @file type_chart_application.h
 * @brief The table of the type chart.
 */

#pragma once

#include "common.h"
#include "system/native/controller.h"
#include "ui/application.h"

namespace sys {
class Controller;
class Graphics;
}

namespace ui {
/**
 * @brief Shows the type chart as a table on the bottom screen.
 *
 * A row is an attacking type, and a column is a defending type. The colour of a
 * cell is its effect: black for x0, red for x1/2, grey for x1 and green for x2.
 * The screen shows a part of the table (11 rows and 11 columns), as the view of
 * a video game: when the cursor goes to the border, the view moves by one cell.
 * A touch on a cell changes its value (x0, x1/2, x1, x2, and again). The
 * D-pad moves the cursor, A and X change the value, Y puts x1.
 */
class TypeChartApplication : public Application {
  MAKE_SINGLETON(TypeChartApplication)

public:
  /// Opens the table.
  static void Open();

  void Update(sys::Controller& controller) override;
  void DrawTop(sys::Graphics& graphics) override;
  void DrawBottom(sys::Graphics& graphics) override;

private:
  static constexpr u32 kMaxTypes = 32;

  /// Makes the list of the types: the types of the game, then the new types.
  void MakeTypeList();
  /// Changes the value of the cell under the cursor by `delta` steps.
  void Change(s32 delta);
  /// Returns the cell under a touch, or false when the touch is not on a cell.
  bool GetCellAt(s32 touch_x, s32 touch_y, s32& row, s32& column) const;
  /// Gives the place of the table on the bottom screen: it is in the center.
  void GetOrigin(s32 visible, s32& x, s32& y) const;
  /// The number of rows and of columns that the screen shows.
  s32 GetVisibleCount() const;
  /// Moves the view by one cell when the cursor goes to its border.
  void ScrollToCursor();

  u8 types_[kMaxTypes] = {};
  u32 type_count_ = 0;
  s32 cursor_row_ = 0;
  s32 cursor_column_ = 0;
  s32 view_row_ = 0; ///< The first row that the screen shows.
  s32 view_column_ = 0; ///< The first column that the screen shows.
  bool was_down_ = false;
  u32 change_count_ = 0;
};
} // namespace ui
