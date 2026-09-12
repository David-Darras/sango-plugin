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

#pragma once
#include "common.h"

class Graphics;

namespace ui {
class MainApplication;
class PageItem;

class Painter {
public:
  virtual ~Painter() = default;

  virtual void DrawPageBackground(MainApplication& app) = 0;
  virtual void DrawPageItems(MainApplication& app) = 0;
  virtual bool ShowBottom() { return false; }
  /// Drawn on the bottom screen before the menu, whether it is open or not.
  virtual void DrawBottomOverlay(Graphics& graphics) {}

protected:
  /// What a painter needs to lay the current page out.
  static constexpr u32 kLineHeight = 16;
  static u32 GetCursor(MainApplication& app);
  static u32 GetOffset(MainApplication& app);
  static u32 GetDisplayCount(MainApplication& app);
  static PageItem& GetEntry(MainApplication& app, u32 index);
};

class MainAppPainter : public Painter {
  MAKE_SINGLETON(MainAppPainter)
public:
  void DrawPageBackground(MainApplication& app) override;
  void DrawPageItems(MainApplication& app) override;
  bool ShowBottom() override { return true; }
};
} // namespace ui