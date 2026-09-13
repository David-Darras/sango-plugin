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
#include <type_traits>

namespace renderer {
struct Picture;
}

namespace renderer {

struct PictureFilterSettings {
  bool is_enabled = false;
  Vec2 scale = Vec2(1, 1);
  u8 alpha = 255;
  Color8 top_left_color = Color8(1, 1, 1, 1);
  Color8 top_right_color = Color8(1, 1, 1, 1);
  Color8 bottom_left_color = Color8(1, 1, 1, 1);
  Color8 bottom_right_color = Color8(1, 1, 1, 1);
};
static_assert(std::is_standard_layout<PictureFilterSettings>::value,
              "PictureFilterSettings must have standard layout");

struct PictureFilter : public PictureFilterSettings {
  MAKE_SINGLETON(PictureFilter)

  static void Initialize();

private:
  static u32 DrawHook(Picture* picture, u32 p0, u32 p1, u32 p2);
};

} // namespace renderer
