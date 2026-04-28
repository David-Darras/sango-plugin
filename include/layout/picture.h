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

#ifndef SANGO_PLUGIN_LAYOUT_PICTURE_H
#define SANGO_PLUGIN_LAYOUT_PICTURE_H
#include "pane.h"

namespace layout {

struct Picture {
  Pane pane;
  bool is_initialized;
  u8 _0[3 + 4];
  Vec4 _1[6];
  void* material;
  Color8 colors[4];
};

}  // namespace layout

#endif  // SANGO_PLUGIN_LAYOUT_PICTURE_H
