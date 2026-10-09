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
 * @file picture_filter.cc
 * @brief Changes the pictures of the 2D layouts (the menus).
 *
 * The declarations are in renderer/patch/picture_filter.h.
 */

#include "renderer/patch/picture_filter.h"

#include "core/hook.h"
#include "renderer/native/picture.h"

namespace renderer {

namespace {
core::Hook<u32(Picture*, u32, u32, u32)> draw_hook;
} // namespace

void PictureFilter::Initialize() {
  draw_hook.Install(address::kDrawPicture, DrawHook);
}

u32 PictureFilter::DrawHook(Picture* picture, u32 p0, u32 p1, u32 p2) {
  auto& ctx = GetInstance();

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

  return draw_hook(picture, p0, p1, p2);
}

} // namespace renderer
