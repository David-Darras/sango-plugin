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

#include "common.h"
#include "renderer/light_manager.h"

#include "core/core.h"
#include "hack/hook.h"
#include "hack/hook_manager.h"
#include "ui/menu_context.h"

namespace renderer {
void LightManager::SetupHooks(HookManager& hook_manager) {
  hook_manager.Add(HookID::kChangeOutlineScale,
                   ADDRESS_RENDERER_CHANGE_OUTLINE_SCALE,
                   (uptr)ChangeOutlineScaleHook);
  hook_manager.Add(HookID::kChangeAmbientLightColor,
                   ADDRESS_RENDERER_CHANGE_AMBIENT_LIGHT_COLOR,
                   (uptr)ChangeAmbientLightColorHook);
  hook_manager.Add(HookID::kChangeDiffuseLightColor,
                   ADDRESS_RENDERER_CHANGE_DIFFUSE_LIGHT_COLOR,
                   (uptr)ChangeDiffuseLightColorHook);
}

void LightManager::ChangeOutlineScaleHook(void* outline_manager,
                                          f32 screen_width,
                                          f32 screen_height, f32 scale) {
  auto& ctx = ui::TopMenuContext::GetInstance().renderer.light;

  ((void (*)(void*, f32, f32, f32, f32))ADDRESS_RENDERER_CHANGE_OUTLINE_COLOR)(
      outline_manager,
      ctx.outline_color.r,
      ctx.outline_color.g,
      ctx.outline_color.b,
      ctx.outline_color.a);

  HookManager::GetInstance()
      .Get(HookID::kChangeOutlineScale)
      ->CallOriginal<void>(outline_manager, screen_width, screen_height,
                           ctx.outline_scale);
}

void LightManager::ChangeAmbientLightColorHook(void* light_manager,
                                               Color* color) {
  auto& ctx = ui::TopMenuContext::GetInstance().renderer.light;

  HookManager::GetInstance()
      .Get(HookID::kChangeAmbientLightColor)
      ->CallOriginal<void>(light_manager, &ctx.ambient_color);
}

void LightManager::ChangeDiffuseLightColorHook(void* light_manager,
                                               Color* color) {
  auto& ctx = ui::TopMenuContext::GetInstance().renderer.light;

  HookManager::GetInstance()
      .Get(HookID::kChangeDiffuseLightColor)
      ->CallOriginal<void>(light_manager, &ctx.diffuse_color);
}
} // namespace renderer