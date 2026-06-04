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
#include "core/core.h"
#include "feature/hook_manager.h"

namespace feature {
struct LightHookContext {
  MAKE_SINGLETON(LightHookContext)
  f32 outline_scale = 1.0f;
  bool use_outline = false;
  Color outline_color = Color(0, 0, 0, 1);
  bool use_ambient_light = false;
  Color ambient_color = Color(1, 1, 1, 1);
  bool use_diffuse_light = false;
  Color diffuse_color = Color(1, 1, 1, 1);

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kChangeOutlineScale,
                            ADDRESS_RENDERER_CHANGE_OUTLINE_SCALE,
                            (uptr)ChangeOutlineScaleHook);
    HookManager::Initialize(HookID::kChangeAmbientLightColor,
                            ADDRESS_RENDERER_CHANGE_AMBIENT_LIGHT_COLOR,
                            (uptr)ChangeAmbientLightColorHook);
    HookManager::Initialize(HookID::kChangeDiffuseLightColor,
                            ADDRESS_RENDERER_CHANGE_DIFFUSE_LIGHT_COLOR,
                            (uptr)ChangeDiffuseLightColorHook);
  }

  static void ChangeOutlineScaleHook(void* outline_manager,
                                     f32 screen_width,
                                     f32 screen_height, f32 scale) {
    auto& ctx = GetInstance();

    if (ctx.use_outline) {
      ((void (*)(void*, f32, f32, f32, f32))
        ADDRESS_RENDERER_CHANGE_OUTLINE_COLOR)(
          outline_manager,
          ctx.outline_color.r,
          ctx.outline_color.g,
          ctx.outline_color.b,
          ctx.outline_color.a);
    } else {
      ctx.outline_scale = scale;
    }

    HookManager::Call<void>(HookID::kChangeOutlineScale, outline_manager,
                            screen_width, screen_height,
                            ctx.outline_scale);
  }

  static void ChangeAmbientLightColorHook(void* light_manager,
                                          Color* color) {
    auto& ctx = GetInstance();

    if (!ctx.use_ambient_light) {
      ctx.ambient_color = *color;
    }

    HookManager::Call<void>(HookID::kChangeAmbientLightColor, light_manager,
                            &ctx.ambient_color);
  }

  static void ChangeDiffuseLightColorHook(void* light_manager,
                                          Color* color) {
    auto& ctx = GetInstance();

    if (!ctx.use_diffuse_light) {
      ctx.diffuse_color = *color;
    }

    HookManager::Call<void>(HookID::kChangeDiffuseLightColor, light_manager,
                            &ctx.diffuse_color);
  }
};
} // namespace feature