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
 * @file menu_layout.h
 * @brief The layout of the screens of the menu (ui::MainApplication). Only
 *        the files of the menu use it.
 */

#pragma once

#include "common.h"
#include "ui/widget/icon.h"

namespace ui {
namespace layout {
// The layout of the bottom screen (320 x 240 pixels).
constexpr s32 kEditorY = 58; // The top of the editor.
constexpr s32 kFooterY = 193; // The line of the process and the event.
constexpr s32 kNavY = 206; // The top of the navigation buttons.
constexpr s32 kNavHeight = 30;
constexpr u32 kNavCount = 5; // The buttons of the navigation bar.
constexpr u32 kWrapLength = 58; // The characters of a description line.

// The grid of values: 6 columns and 3 rows of cells, then a bar with the
// page buttons, the views and the filter. An icon is a texture of 64 x 32
// pixels in the center of its cell: the transparent sides of the texture
// go over the next cells.
constexpr s32 kPickerX = 10;
constexpr s32 kPickerCellWidth = 50;
constexpr s32 kPickerCellHeight = 33;
constexpr u32 kPickerColumns = 6;
constexpr u32 kPickerRows = 3;
constexpr s32 kPickerBarY = kEditorY + kPickerRows * kPickerCellHeight + 3;
constexpr s32 kPickerBarHeight = 28;

// While a button stays pressed for this number of frames, the lists scroll
// fast: they show only the icons that are ready, and do not load others.
constexpr u32 kHoldNoLoadFrames = 10;

// The icon of a line of the top screen: the texture at half size.
constexpr s32 kLineIconWidth = IconPool::kWidth / 2;
constexpr s32 kLineIconHeight = IconPool::kHeight / 2;

// The top screen. Its texts use a space of 512 x 256 (the size of the
// texture of the screen), but its rectangles use the 400 x 240 pixels. The
// layout of the lines is in the space of the texts; RectX() and RectY()
// give the pixels of the same place.
constexpr s32 kTextWidth = 512;
constexpr s32 kTextHeight = 256;
INLINE s32 RectX(s32 x) { return x * 400 / kTextWidth; }
INLINE s32 RectY(s32 y) { return y * 240 / kTextHeight; }
// The right edge of the values (381 pixels), and the space between a name
// and its value.
constexpr s32 kValueRight = 488;
constexpr s32 kValueGap = 16;
} // namespace layout
} // namespace ui
