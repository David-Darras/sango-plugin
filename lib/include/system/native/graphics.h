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

  /**
   * @brief Returns true when the 2D functions of the game can draw now.
   *
   * The drawings add commands to the command list of the game. Some parts
   * of the game (the menus of the Pokemon and the Bag) do not use this list:
   * its address is not valid, and a drawing writes into random memory.
   * The function checks the list and the address of its next command.
   */
  STATIC_INLINE bool CanDraw() {
    if (renderer::address::kGraphicsState == 0) return true;
    const uptr list =
        *(const uptr*)(renderer::address::kGraphicsState + kStateListOffset);
    if (!IsDataAddress(list)) return false;
    const uptr start = *(const uptr*)(list + kListStartOffset);
    const uptr cursor = *(const uptr*)renderer::address::kGraphicsCommandCursor;
    return IsDataAddress(start) && IsDataAddress(cursor) && cursor >= start &&
           cursor - start < kMaxListSize && (cursor & 3) == 0;
  }

  /// Starts the drawings on an image buffer.
  STATIC_INLINE void BeginRender(void* framebuffer) {
    ((void (*)(void*))renderer::address::kGraphicsBeginRender)(framebuffer);
  }

  /**
   * @brief The texts that the GPU can draw in one frame, on the two screens.
   *
   * Each text adds many commands to the command list of the GPU, and the
   * list has a fixed size: about 100 texts in one frame make the game
   * crash, 88 texts are safe. DrawText() does not draw the texts after
   * kTextLimit. A shadow is a text too: DrawTextWithShadow() adds it only
   * when the texts of the frame (estimated with the last frame) and the
   * shadows stay under kShadowLimit.
   */
  static constexpr u32 kTextLimit = 88;

  /// The command list in the drawing state, and the start of its buffer in
  /// the command list.
  static constexpr u32 kStateListOffset = 0x9C;
  static constexpr u32 kListStartOffset = 0x4;
  /// The largest size of a command list that CanDraw() accepts.
  static constexpr u32 kMaxListSize = 0x100000;

  /// Returns true for an address in the heap or in the linear memory (FCRAM
  /// or VRAM) of the game.
  STATIC_INLINE bool IsDataAddress(uptr address) {
    return (address >= 0x08000000 && address < 0x10000000) ||
           (address >= 0x14000000 && address < 0x1C000000) ||
           (address >= 0x1F000000 && address < 0x1F600000) ||
           (address >= 0x30000000 && address < 0x38000000);
  }
  static constexpr u32 kShadowLimit = 80;

  /**
   * @brief The size of the command list, in bytes, after which the drawings
   *        stop. The list has a fixed size (about 256 KB). core::FrameTrace
   *        showed the bottom screen at 255 672 bytes with only 88 texts: the
   *        count of texts is not enough, because a text costs from 1 KB to
   *        4 KB. A drawing past the end of the list freezes the game.
   *
   * The shadows stop first, then the texts, then the rectangles: the menu
   * loses its shadows before its words.
   */
  static constexpr u32 kShadowBytesLimit = 190000;
  static constexpr u32 kTextBytesLimit = 215000;
  static constexpr u32 kRectBytesLimit = 240000;

  /// The texts and the rectangles of the current frame, and the texts of the
  /// last frame.
  struct TextBudget {
    u32 texts; ///< The texts of this frame, without the shadows.
    u32 shadows; ///< The shadows of this frame.
    u32 last_texts; ///< The texts of the last frame, without the shadows.
    u32 rects; ///< The rectangles of this frame (core::FrameTrace shows it).
  };

  STATIC_INLINE TextBudget& GetTextBudget() {
    static TextBudget budget = {0, 0, 0, 0};
    return budget;
  }

  /**
   * @brief Returns the number of bytes in the command list of the GPU.
   *
   * Each drawing adds commands to the list, and the list has a fixed size.
   * core::FrameTrace writes the value to the SD card: it shows how near the
   * list is to its end. The function returns 0 when the address is not known
   * (XY) or when the game does not draw with the list (see CanDraw()).
   */
  STATIC_INLINE u32 GetCommandBytes() {
    if (renderer::address::kGraphicsState == 0 || !CanDraw()) return 0;
    const uptr list =
        *(const uptr*)(renderer::address::kGraphicsState + kStateListOffset);
    const uptr start = *(const uptr*)(list + kListStartOffset);
    const uptr cursor = *(const uptr*)renderer::address::kGraphicsCommandCursor;
    return (u32)(cursor - start);
  }

  /// Starts the count of the texts of a new frame. plugin::DrawFrame()
  /// calls it before the drawings.
  STATIC_INLINE void StartFrame() {
    TextBudget& budget = GetTextBudget();
    budget.last_texts = budget.texts;
    budget.texts = 0;
    budget.shadows = 0;
    budget.rects = 0;
  }

  /// Draws a UTF-16 text. (x, y) is the top-left corner in pixels.
  STATIC_INLINE void DrawText(s32 x, s32 y, const c16* str,
                              const Color color = {1.0f, 1.0f, 1.0f, 1.0f},
                              void* pFont = nullptr) {
    TextBudget& budget = GetTextBudget();
    if (budget.texts + budget.shadows >= kTextLimit) return;
    if (GetCommandBytes() > kTextBytesLimit) return;
    budget.texts++;
    Color clr = color;
    ((void (*)(s32, s32, const c16*, Color*, void*))renderer::address::kGraphicsDrawText)(
        x, y, str, &clr, pFont);
  }

  /**
   * @brief Draws a text with a dark shadow under it.
   *
   * The shadow doubles the cost of the text. The function draws it only
   * when the texts of the last frame and the shadows of this frame stay
   * under kShadowLimit.
   */
  STATIC_INLINE void DrawTextWithShadow(s32 x, s32 y, const c16* str,
                                        Color color) {
    TextBudget& budget = GetTextBudget();
    // The texts of this frame: at least the texts of the last frame. At the
    // first frame of the menu, the last frame has no texts: no shadow.
    const u32 texts = budget.texts > budget.last_texts ? budget.texts
                                                       : budget.last_texts;
    if (budget.last_texts != 0 && texts + budget.shadows < kShadowLimit &&
        GetCommandBytes() < kShadowBytesLimit) {
      budget.shadows++;
      Color shadow(0, 0, 0, 0.6f * color.a);
      ((void (*)(s32, s32, const c16*, Color*, void*))
           renderer::address::kGraphicsDrawText)(x + 1, y + 1, str, &shadow,
                                                 nullptr);
    }
    DrawText(x, y, str, color);
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

  /**
   * @brief Adds spaces at the end of a text, until the text reaches a
   *        width.
   *
   * With it, one text can show several columns (for example the name and
   * the value of an entry): one text costs less than several texts.
   * @param text The text (UTF-16). It changes.
   * @param capacity The size of `text`, in characters.
   * @param width The width to reach, with the current text scale.
   */
  STATIC_INLINE void AppendSpaces(c16* text, u32 capacity, s32 width) {
    // The width of one space: the font can trim a space at the end.
    const s32 space = GetTextWidth(u"| |") - GetTextWidth(u"||");
    if (space <= 0) return;
    u32 length = 0;
    while (text[length] != 0) length++;
    s32 count = (width - GetTextWidth(text) + space / 2) / space;
    while (count-- > 0 && length + 1 < capacity) text[length++] = u' ';
    text[length] = 0;
    // Measure again: remove the spaces that go past the width.
    while (length > 0 && text[length - 1] == u' ' &&
           GetTextWidth(text) > width + space / 2) {
      text[--length] = 0;
    }
  }

  /// Adds a text at the end of `text`, when there is space.
  STATIC_INLINE void AppendText(c16* text, u32 capacity, const c16* add) {
    u32 length = 0;
    while (text[length] != 0) length++;
    while (*add != 0 && length + 1 < capacity) text[length++] = *add++;
    text[length] = 0;
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
    DrawRectWithTexture(x, y, width, height, color, nullptr);
  }

  /**
   * @brief Draws a rectangle with a color, or with a texture.
   * @param color The color. With a texture, it multiplies the texture.
   * @param texture A texture (ui::Texture), or null for a color only. The
   *        texture fills the rectangle.
   */
  STATIC_INLINE void DrawRectWithTexture(s32 x, s32 y, s32 width, s32 height,
                                         Color color, void* texture) {
    if (GetCommandBytes() > kRectBytesLimit) return;
    GetTextBudget().rects++;
    Material mat = {};
    mat._0 = texture != nullptr ? 1 : 0; // 1: the texture shader.
    mat._9 = texture;
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

