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
 * @file lighting.cc
 * @brief Changes the lights and the outlines of the 3D models.
 *
 * The declarations are in renderer/patch/lighting.h.
 */

#include "renderer/patch/lighting.h"

#include "core/hook.h"

namespace renderer {

namespace {
core::Hook<void(void*, f32, f32, f32)> change_outline_scale_hook;
core::Hook<void(void*, Color*)> change_ambient_light_color_hook;
core::Hook<void(void*, Color*)> change_diffuse_light_color_hook;
} // namespace

void Lighting::Initialize() {
  change_outline_scale_hook.Install(address::kChangeOutlineScale,
                                    ChangeOutlineScaleHook);
  change_ambient_light_color_hook.Install(address::kChangeAmbientLightColor,
                                          ChangeAmbientLightColorHook);
  change_diffuse_light_color_hook.Install(address::kChangeDiffuseLightColor,
                                          ChangeDiffuseLightColorHook);
}

void Lighting::ChangeOutlineScaleHook(void* outline_manager, f32 screen_width,
                                  f32 screen_height, f32 scale) {
  auto& ctx = GetInstance();
  static bool was_enabled = false;
  auto set_color = (void (*)(void*, f32, f32, f32, f32))address::kChangeOutlineColor;

  if (ctx.use_outline) {
    set_color(outline_manager, ctx.outline_color.r, ctx.outline_color.g,
              ctx.outline_color.b, ctx.outline_color.a);
  } else {
    if (was_enabled) set_color(outline_manager, 0.0f, 0.0f, 0.0f, 1.0f);
    ctx.outline_scale = scale;
  }
  was_enabled = ctx.use_outline;

  change_outline_scale_hook(outline_manager, screen_width, screen_height,
                            ctx.outline_scale);
}

void Lighting::ChangeAmbientLightColorHook(void* light_manager, Color* color) {
  auto& ctx = GetInstance();

  if (!ctx.use_ambient_light) {
    ctx.ambient_color = *color;
  }

  change_ambient_light_color_hook(light_manager, &ctx.ambient_color);
}

void Lighting::ChangeDiffuseLightColorHook(void* light_manager, Color* color) {
  auto& ctx = GetInstance();

  if (!ctx.use_diffuse_light) {
    ctx.diffuse_color = *color;
  }

  change_diffuse_light_color_hook(light_manager, &ctx.diffuse_color);
}

} // namespace renderer
