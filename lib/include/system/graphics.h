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
#include "core.h"
#include "font_manager.h"

/**
 * @brief Identifiers for the available hardware screens.
 */
enum class Screen : u8 {
  kTop, ///< The upper display.
  kBottom ///< The lower display.
};

/**
 * @brief Static wrapper for graphics-related operations and rendering.
 * * This class provides methods to manipulate framebuffers, manage scissor
 * testing, and perform basic text/primitive rendering via internal engine
 * calls.
 */
class Graphics {
  SINGLETON(Graphics)
  /** @brief Scale factor used to simulate large primitives using specific
 * glyphs. */
  static constexpr f32 kScalePrimitiveX = 25.0f;
  static constexpr f32 kScalePrimitiveY = 20.0f;

  /** @brief Default scale factor for standard text rendering. */
  static constexpr f32 kScaleDefaultX = 0.6f;
  static constexpr f32 kScaleDefaultY = 0.6f;

public:
  /**
 * @brief Retrieves the singleton instance of the Graphics controller.
 * @return A reference to the active Graphics instance.
 */
  STATIC_INLINE Graphics& GetInstance() {
    return Core::GetInstance().GetGraphics();
  }

  /**
 * @brief Gets the raw framebuffer pointer for a specific screen.
 * @param screen The target screen (Top or Bottom).
 * @return A pointer to the memory-mapped framebuffer.
 */
  INLINE void* GetFramebuffer(Screen screen) {
    return ((void* (*)(Graphics*, Screen))ADDRESS_GRAPHICS_GET_FRAMEBUFFER)(
        this, screen);
  }

  /**
 * @brief Binds a framebuffer for subsequent drawing operations.
 * @param framebuffer Pointer to the framebuffer to bind.
 * @return True if the binding was successful, false otherwise.
 */
  INLINE bool BindFramebuffer(void* framebuffer) {
    return ((bool (*)(Graphics*, void*))ADDRESS_GRAPHICS_BIND_FRAMEBUFFER)(
        this, framebuffer);
  }

  /**
 * @brief Enables scissor testing to restrict drawing to a rectangular area.
 * @param x      The X coordinate of the scissor box.
 * @param y      The Y coordinate of the scissor box.
 * @param width  The width of the scissor box.
 * @param height The height of the scissor box.
 */
  STATIC_INLINE void EnableScissor(u32 x, u32 y, u32 width, u32 height) {
    ((void (*)(u32, u32, u32, u32))ADDRESS_GRAPHICS_ENABLE_SCISSOR)(x, y, width,
      height);
  }

  /**
 * @brief Disables the current scissor test.
 */
  STATIC_INLINE void DisableScissor() {
    ((void (*)())ADDRESS_GRAPHICS_DISABLE_SCISSOR)();
  }

  /**
 * @brief Initiates the rendering sequence for a specific framebuffer.
 * @param framebuffer Pointer to the target framebuffer.
 */
  STATIC_INLINE void BeginRender(void* framebuffer) {
    ((void (*)(void*))ADDRESS_GRAPHICS_BEGIN_RENDER)(framebuffer);
  }

  /**
 * @brief Renders a UTF-16 string at the specified coordinates.
 * @param x      X position on screen.
 * @param y      Y position on screen.
 * @param str    The UTF-16 string to display.
 * @param color  The RGBA color to apply (defaults to white).
 * @param pFont  Optional pointer to a custom font.
 */
  STATIC_INLINE void DrawText(s32 x, s32 y, const c16* str,
                              const Color color = {1.0f, 1.0f, 1.0f, 1.0f},
                              void* pFont = nullptr) {
    Color clr = color;
    ((void (*)(s32, s32, const c16*, Color*, void*))ADDRESS_GRAPHICS_DRAW_TEXT)(
        x, y, str, &clr, pFont);
  }

  /**
 * @brief Sets the global text scaling factor.
 * @param x Scale factor on the X axis.
 * @param y Scale factor on the Y axis.
 */
  STATIC_INLINE void SetTextScale(f32 x, f32 y) {
    ((void (*)(f32, f32))ADDRESS_GRAPHICS_SET_TEXT_SCALE)(x, y);
  }

  /**
 * @brief Fills the entire screen with a solid color.
 * Uses a large-scale glyph to perform the fill operation.
 * * @param r Red component (0.0 - 1.0).
 * @param g Green component (0.0 - 1.0).
 * @param b Blue component (0.0 - 1.0).
 * @param a Alpha component (0.0 - 1.0).
*/
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
    ((void (*)(const Material*))ADDRESS_GRAPHICS_SET_MATERIAL)(&mat);

    const Color c = color;
    ((void (*)(s32, s32, s32, s32, const Color*))ADDRESS_GRAPHICS_DRAW_RECT)(
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