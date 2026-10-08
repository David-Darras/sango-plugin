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
 * @file painter.h
 * @brief The objects that draw the menu.
 */

#pragma once
#include "common.h"

namespace sys {
class Graphics;
}

namespace ui {
class MainApplication;
class PageItem;

/// Draws the menu. Make a subclass to change the look of the menu.
class Painter {
public:
  virtual ~Painter() = default;

  /// Draws the background of the page.
  virtual void DrawPageBackground(MainApplication& app) = 0;
  /// Draws the entries of the page.
  virtual void DrawPageItems(MainApplication& app) = 0;
  /// Returns true to show the numpad and the keyboard on the bottom screen.
  virtual bool ShowBottom() { return false; }
  /// Draws on the bottom screen before the menu, also when the menu is closed.
  virtual void DrawBottomOverlay(sys::Graphics& graphics) {}

protected:
  /// The data of the current page for a painter.
  static constexpr u32 kLineHeight = 16;
  static u32 GetCursor(MainApplication& app);
  static u32 GetOffset(MainApplication& app);
  static u32 GetDisplayCount(MainApplication& app);
  static PageItem& GetEntry(MainApplication& app, u32 index);
};

/// The painter of the overlay.
class MainAppPainter : public Painter {
  MAKE_SINGLETON(MainAppPainter)
public:
  void DrawPageBackground(MainApplication& app) override;
  void DrawPageItems(MainApplication& app) override;
  bool ShowBottom() override { return true; }
};
} // namespace ui