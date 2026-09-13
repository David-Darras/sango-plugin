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
#include "system/native/core.h"

namespace sys {

enum class Screen : u8 {
  kTop = 0,
  kBottom = 1
};

class Graphics {
  SINGLETON(Graphics)
  static constexpr f32 kScalePrimitiveX = 25.0f;
  static constexpr f32 kScalePrimitiveY = 20.0f;

  static constexpr f32 kScaleDefaultX = 0.6f;
  static constexpr f32 kScaleDefaultY = 0.6f;

public:
  STATIC_INLINE Graphics& GetInstance() {
    return Core::GetInstance().GetGraphics();
  }

  INLINE void* GetFramebuffer(Screen screen) {
    return ((void* (*)(Graphics*, Screen))renderer::address::kGraphicsGetFramebuffer)(
        this, screen);
  }

  INLINE bool BindFramebuffer(void* framebuffer) {
    return ((bool (*)(Graphics*, void*))renderer::address::kGraphicsBindFramebuffer)(
        this, framebuffer);
  }

  STATIC_INLINE void EnableScissor(u32 x, u32 y, u32 width, u32 height) {
    ((void (*)(u32, u32, u32, u32))renderer::address::kGraphicsEnableScissor)(x, y, width,
      height);
  }

  STATIC_INLINE void DisableScissor() {
    ((void (*)())renderer::address::kGraphicsDisableScissor)();
  }

  STATIC_INLINE void BeginRender(void* framebuffer) {
    ((void (*)(void*))renderer::address::kGraphicsBeginRender)(framebuffer);
  }

  STATIC_INLINE void DrawText(s32 x, s32 y, const c16* str,
                              const Color color = {1.0f, 1.0f, 1.0f, 1.0f},
                              void* pFont = nullptr) {
    Color clr = color;
    ((void (*)(s32, s32, const c16*, Color*, void*))renderer::address::kGraphicsDrawText)(
        x, y, str, &clr, pFont);
  }

  STATIC_INLINE void SetTextScale(f32 x, f32 y) {
    ((void (*)(f32, f32))renderer::address::kGraphicsSetTextScale)(x, y);
  }

  STATIC_INLINE void FillScreen(f32 r, f32 g, f32 b, f32 a) {
    const Color color{r, g, b, a};
    DrawRect(0, 0, 400, 240, color);
  }

  STATIC_INLINE void FillScreen(Color color) {
    DrawRect(0, 0, 400, 240, color);
  }

  struct Material {
    u32 _0;
    u8 _1;
    u8 _2[3];
    u32 _3;
    u32 _4;
    u32 _5;
    f32 _6[3];
    f32 _7[3];
    f32 _8[3];
    f32 color[4];
    void* _9;
    f32 _10[2];
  };

  STATIC_INLINE void
  DrawRect(s32 x, s32 y, s32 width, s32 height, Color color) {
    Material mat = {};
    mat._0 = 0;
    mat._1 = 1;
    mat._3 = 0x8006;
    mat._4 = 0x0302;
    mat._5 = 0x0303;
    mat._6[0] = 1.f;
    mat._7[0] = mat._7[1] =
                mat._7[2] = .7f;
    mat._8[0] = mat._8[1] =
                mat._8[2] = .3f;
    mat.color[0] = mat.color[1] =
                   mat.color[2] = mat.color[3] = 1.f;
    ((void (*)(const Material*))renderer::address::kGraphicsSetMaterial)(&mat);

    const Color c = color;
    ((void (*)(s32, s32, s32, s32, const Color*))renderer::address::kGraphicsDrawRect)(
        x, y, width, height, &c);

    SetTextScale(0.6, 0.6);
  }

  STATIC_INLINE void
  DrawRectStroke(s32 x, s32 y, s32 width, s32 height, s32 thickness,
                 Color color) {
    DrawRect(x, y, width, thickness, color);
    DrawRect(x, y + height - thickness, width, thickness, color);
    DrawRect(x, y, thickness, height, color);
    DrawRect(x + width - thickness, y, thickness, height, color);
  }
};

} // namespace sys

using sys::Screen;
