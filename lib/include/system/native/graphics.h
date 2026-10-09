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
 * @file graphics.h
 * @brief Draws on the screens: texts and rectangles.
 *
 * The menu of the plugin uses these functions. plugin::DrawFrame() prepares
 * the two screens before it calls them.
 */

#pragma once

#include "common.h"
#include "system/constant/screen.h"
#include "system/native/core.h"

namespace sys {

/// The graphics system of the game.
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

  /// Returns the image buffer of a screen.
  INLINE void* GetFramebuffer(Screen screen) {
    return ((void* (*)(Graphics*, Screen))renderer::address::kGraphicsGetFramebuffer)(
        this, screen);
  }

  /// Selects the image buffer for the next drawings.
  INLINE bool BindFramebuffer(void* framebuffer) {
    return ((bool (*)(Graphics*, void*))renderer::address::kGraphicsBindFramebuffer)(
        this, framebuffer);
  }

  /// Limits the next drawings to a rectangle.
  STATIC_INLINE void EnableScissor(u32 x, u32 y, u32 width, u32 height) {
    ((void (*)(u32, u32, u32, u32))renderer::address::kGraphicsEnableScissor)(x, y, width,
      height);
  }

  /// Removes the limit of EnableScissor().
  STATIC_INLINE void DisableScissor() {
    ((void (*)())renderer::address::kGraphicsDisableScissor)();
  }

  /// Starts the drawings on an image buffer.
  STATIC_INLINE void BeginRender(void* framebuffer) {
    ((void (*)(void*))renderer::address::kGraphicsBeginRender)(framebuffer);
  }

  /// Draws a UTF-16 text. (x, y) is the top-left corner in pixels.
  STATIC_INLINE void DrawText(s32 x, s32 y, const c16* str,
                              const Color color = {1.0f, 1.0f, 1.0f, 1.0f},
                              void* pFont = nullptr) {
    Color clr = color;
    ((void (*)(s32, s32, const c16*, Color*, void*))renderer::address::kGraphicsDrawText)(
        x, y, str, &clr, pFont);
  }

  /// Sets the size of the next texts. The menu uses 0.6.
  STATIC_INLINE void SetTextScale(f32 x, f32 y) {
    ((void (*)(f32, f32))renderer::address::kGraphicsSetTextScale)(x, y);
  }

  /**
   * @brief Returns the width of a text with the current text scale.
   *
   * The width is in the units of DrawText(): pixels on the bottom screen,
   * and a space of 512 x 256 on the top screen. The function draws nothing.
   * When the address is not known (XY), it returns an estimate.
   */
  STATIC_INLINE s32 GetTextWidth(const c16* str) {
    if (renderer::address::kGraphicsGetTextWidth == 0) {
      s32 length = 0;
      while (str[length] != 0) length++;
      return length * 9;
    }
    return (s32)((f32 (*)(const c16*, void*))
                     renderer::address::kGraphicsGetTextWidth)(str,
                                                                  nullptr);
  }

  /// Fills the screen with a color.
  STATIC_INLINE void FillScreen(f32 r, f32 g, f32 b, f32 a) {
    const Color color{r, g, b, a};
    DrawRect(0, 0, 400, 240, color);
  }

  /// Fills the screen with a color.
  STATIC_INLINE void FillScreen(Color color) {
    DrawRect(0, 0, 400, 240, color);
  }

  /// The drawing settings of the game. DrawRect() fills it.
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
  /// Draws a filled rectangle.
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
  /// Draws the border of a rectangle. `thickness` is in pixels.
  DrawRectStroke(s32 x, s32 y, s32 width, s32 height, s32 thickness,
                 Color color) {
    DrawRect(x, y, width, thickness, color);
    DrawRect(x, y + height - thickness, width, thickness, color);
    DrawRect(x, y, thickness, height, color);
    DrawRect(x + width - thickness, y, thickness, height, color);
  }
};

} // namespace sys

