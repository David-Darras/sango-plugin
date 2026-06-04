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

namespace ui {
class MainApplication;

class Painter {
public:
  virtual ~Painter() = default;

  virtual void DrawPageBackground(MainApplication& app) = 0;
  virtual void DrawPageItems(MainApplication& app) = 0;
  virtual bool ShowBottom() { return false; }
};

class MainAppPainter : public Painter {
  MAKE_SINGLETON(MainAppPainter)
public:
  void DrawPageBackground(MainApplication& app) override;
  void DrawPageItems(MainApplication& app) override;
  bool ShowBottom() override { return true; }
};

class RetroAppPainter : public Painter {
  MAKE_SINGLETON(RetroAppPainter)
public:
  void DrawPageBackground(MainApplication& app) override;
  void DrawPageItems(MainApplication& app) override;
};
} // namespace ui