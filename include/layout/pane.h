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

#ifndef SANGO_PLUGIN_LAYOUT_PANE_H
#define SANGO_PLUGIN_LAYOUT_PANE_H

#include "core/core.h"

namespace layout {

struct Pane {
  void* vtable;
  Pane* next_pane;
  Pane* prev_pane;
  Pane* parent;
  struct {
    u32 count;
    Pane* next_pane;
    Pane* prev_pane;
  } children, animations;
  Vec3 position;
  Vec3 rotation;
  Vec2 scale;
  Vec2 scale2;
  Mtx34 matrix;
  Mtx34 matrix2;
  void* _0;
  u8 alpha;
  u8 alpha2;
  u8 _1;
  u8 flags;  // bit 0 : visible
  char id[17];
  u8 _2[11];
};

}  // namespace layout

#endif  // SANGO_PLUGIN_LAYOUT_PANE_H
