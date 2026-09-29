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
#include "ui/application.h"

namespace ui {
class FreeCameraApplication : public Application {
  MAKE_SINGLETON(FreeCameraApplication)

public:
  static void Open();

  void Update(sys::Controller& controller) override;
  void DrawTop(sys::Graphics& graphics) override {}
  void DrawBottom(sys::Graphics& graphics) override;

private:
  bool is_battle_ = false;
  u32 held_frames_ = 0;
  u32 turn_held_frames_ = 0;
};
} // namespace ui
