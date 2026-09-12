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
#include "ui/painter.h"

namespace kaizo {
/// The compact menu of the ROM hack: a small panel in the top-right corner,
/// plus a splash screen on the bottom screen for the first second.
class Painter : public ui::Painter {
  MAKE_SINGLETON(Painter)
public:
  void DrawPageBackground(ui::MainApplication& app) override;
  void DrawPageItems(ui::MainApplication& app) override;
  void DrawBottomOverlay(Graphics& graphics) override;
};
} // namespace kaizo
