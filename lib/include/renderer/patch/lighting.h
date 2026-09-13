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

struct LightingSettings {
  f32 outline_scale = 1.0f;
  bool use_outline = false;
  Color outline_color = Color(0, 0, 0, 1);
  bool use_ambient_light = false;
  Color ambient_color = Color(1, 1, 1, 1);
  bool use_diffuse_light = false;
  Color diffuse_color = Color(1, 1, 1, 1);
};
static_assert(std::is_standard_layout<LightingSettings>::value,
              "LightingSettings must have standard layout");

struct Lighting : public LightingSettings {
  MAKE_SINGLETON(Lighting)

  void SetAmbient(f32 r, f32 g, f32 b, f32 a = 1.0f) {
    use_ambient_light = true;
    ambient_color = Color(r, g, b, a);
  }

  void ResetAmbient() {
    use_ambient_light = false;
  }

  void SetDiffuse(f32 r, f32 g, f32 b, f32 a = 1.0f) {
    use_diffuse_light = true;
    diffuse_color = Color(r, g, b, a);
  }

  void ResetDiffuse() {
    use_diffuse_light = false;
  }

  static void Initialize();

private:
  static void ChangeOutlineScaleHook(void* outline_manager, f32 screen_width,
                                     f32 screen_height, f32 scale);
  static void ChangeAmbientLightColorHook(void* light_manager, Color* color);
  static void ChangeDiffuseLightColorHook(void* light_manager, Color* color);
};

} // namespace renderer
