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

#ifndef SANGO_PLUGIN_FEATURE_TEXT_BOX_H
#define SANGO_PLUGIN_FEATURE_TEXT_BOX_H

#include "common.h"
#include "core/core.h"
#include "feature/hook_manager.h"
#include "renderer/text_box.h"

namespace feature {
struct TextBoxHookContext {
  MAKE_SINGLETON(TextBoxHookContext)
  bool is_enabled = false;
  Vec2 scale = Vec2(1, 1);
  Color8 top_color = Color8(0, 0, 0, 1);
  Color8 bottom_color = Color8(0, 0, 0, 1);

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kDrawTextBox,
                            ADDRESS_RENDERER_DRAW_TEXT_BOX, (uptr)DrawHook);
  }

  static u32 DrawHook(renderer::TextBox* text_box, u32 writer) {
    u32 res = HookManager::Call<u32>(HookID::kDrawTextBox, text_box, writer);

    TextBoxHookContext& ctx = GetInstance();
    if (ctx.is_enabled) {
      text_box->pane.scale = ctx.scale;
      text_box->top_color = ctx.top_color;
      text_box->bottom_color = ctx.bottom_color;
    }

    return res;
  }
};
} // namespace feature

#endif // SANGO_PLUGIN_FEATURE_TEXT_BOX_H