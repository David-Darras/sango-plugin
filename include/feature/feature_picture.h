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
#include "feature/hook_manager.h"
#include "game/renderer/picture.h"
#include "game/renderer/text_box.h"

namespace feature {
struct Picture {
  MAKE_SINGLETON(Picture)
  bool is_enabled = false;
  Vec2 scale = Vec2(1, 1);
  u8 alpha = 255;
  Color8 top_left_color = Color8(1, 1, 1, 1);
  Color8 top_right_color = Color8(1, 1, 1, 1);
  Color8 bottom_left_color = Color8(1, 1, 1, 1);
  Color8 bottom_right_color = Color8(1, 1, 1, 1);

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kDrawPicture,
                            ADDRESS_RENDERER_DRAW_PICTURE, (uptr)DrawHook);
  }

  static u32 DrawHook(renderer::Picture* picture, u32 p0, u32 p1, u32 p2) {
    Picture& ctx = GetInstance();

    if (ctx.is_enabled) {
      picture->pane.scale = ctx.scale;
      picture->pane.alpha = ctx.alpha;

      picture->top_left_color.r = ctx.top_left_color.r;
      picture->top_left_color.g = ctx.top_left_color.g;
      picture->top_left_color.b = ctx.top_left_color.b;

      picture->top_right_color.r = ctx.top_right_color.r;
      picture->top_right_color.g = ctx.top_right_color.g;
      picture->top_right_color.b = ctx.top_right_color.b;

      picture->bottom_left_color.r = ctx.bottom_left_color.r;
      picture->bottom_left_color.g = ctx.bottom_left_color.g;
      picture->bottom_left_color.b = ctx.bottom_left_color.b;

      picture->bottom_right_color.r = ctx.bottom_right_color.r;
      picture->bottom_right_color.g = ctx.bottom_right_color.g;
      picture->bottom_right_color.b = ctx.bottom_right_color.b;
    }

    return HookManager::Call<u32>(HookID::kDrawPicture, picture, p0, p1, p2);
  }
};
} // namespace feature