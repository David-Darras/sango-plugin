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
 * @file lighting.h
 * @brief Changes the lights and the outlines of the 3D models.
 */

#pragma once

#include "common.h"
#include <type_traits>

namespace renderer {

/// The settings of renderer::Lighting.
struct LightingSettings {
  f32 outline_scale = 0.0f; ///< The width of the outlines. 0: the width of the game.
  bool use_outline = true; ///< false: no outlines.
  Color outline_color = Color(0, 0, 0, 1);
  bool use_ambient_light = false;
  Color ambient_color = Color(1, 1, 1, 1);
  bool use_diffuse_light = false;
  Color diffuse_color = Color(1, 1, 1, 1);
};
static_assert(std::is_standard_layout<LightingSettings>::value,
              "LightingSettings must have standard layout");

/// Changes the lights and the outlines of the 3D models.
struct Lighting : public LightingSettings {
  MAKE_SINGLETON(Lighting)

  /// Replaces the ambient light color.
  void SetAmbient(f32 r, f32 g, f32 b, f32 a = 1.0f) {
    use_ambient_light = true;
    ambient_color = Color(r, g, b, a);
  }

  /// Uses the ambient light of the game again.
  void ResetAmbient() {
    use_ambient_light = false;
  }

  /// Replaces the diffuse light color.
  void SetDiffuse(f32 r, f32 g, f32 b, f32 a = 1.0f) {
    use_diffuse_light = true;
    diffuse_color = Color(r, g, b, a);
  }

  /// Uses the diffuse light of the game again.
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
