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
 * @file color_picker.h
 * @brief A color picker for the bottom screen, like the one of a paint
 *        program.
 */

#pragma once

#include "common.h"
#include "core/color.h"
#include "ui/widget/texture.h"

namespace ui {
/**
 * @brief A color picker on the bottom screen.
 *
 * It has a square for the saturation (to the right) and the brightness (to
 * the top), a bar for the hue, a bar for the alpha, two swatches (the new
 * color and the color of the start), the hexadecimal code, and a palette of
 * 12 colors. The player drags the stylus in the square and in the bars, or
 * touches a color of the palette.
 *
 * The square and the bars are textures that the picker makes: the picker
 * draws them with three rectangles, so the command list of the GPU stays
 * small (see sys::Graphics::kTextLimit).
 *
 * The picker keeps the hue, the saturation and the brightness. A gray color
 * has no hue: the picker keeps the last hue, so the hue bar does not jump.
 */
class ColorPicker {
public:
  /// The top-left corner of the square, on the bottom screen.
  static constexpr s32 kX = 10;
  static constexpr s32 kY = 62;
  /// The panel on the right of the bars.
  static constexpr s32 kPanelX = kX + 172;
  static constexpr s32 kPanelWidth = 128;
  /// The place of the button that the menu draws under the picker: it
  /// switches to the numpad.
  static constexpr s32 kSwitchX = kPanelX;
  static constexpr s32 kSwitchY = kY + 112;
  static constexpr s32 kSwitchWidth = kPanelWidth;
  static constexpr s32 kSwitchHeight = 16;

  /// Sets the color of the start (the old swatch) and the current color.
  void Begin(const Color& color);

  /// Sets the current color when it changed outside the picker (for example
  /// with the Left and Right buttons). It does nothing for the same color.
  void Sync(const Color& color);

  /// Forgets the touch that is in progress. Call it when the picker appears.
  void ResetTouch();

  /**
   * @brief Reads the touch screen. Call it one time for each frame.
   * @return true when the player changed the color in this frame.
   */
  bool Update();

  /// Returns true at the frame when a new touch starts to change the color.
  /// The menu keeps the old color for Undo then.
  bool IsStarted() const { return is_started_; }

  /// Returns the current color.
  Color GetColor() const;

  /// Draws the picker.
  void Draw();

private:
  enum Drag : u8 {
    kDragNone,
    kDragSquare,
    kDragHue,
    kDragAlpha,
  };

  static constexpr s32 kSquareSize = 120;
  static constexpr s32 kBarWidth = 16;
  static constexpr u32 kPaletteCount = 12;

  /// Reads the position of the stylus in a bar or in the square.
  void ApplyTouch(s32 x, s32 y);
  /// Returns the rectangle of a palette color.
  void GetPaletteCell(u32 index, s32& x, s32& y) const;
  /// Draws the textures again when they are not up to date.
  void UpdateTextures();
  /// Sets the three values of the color from the red, green and blue values.
  void SetRgb(f32 r, f32 g, f32 b);

  f32 hue_ = 0.0f; ///< 0 to 360.
  f32 saturation_ = 1.0f; ///< 0 to 1.
  f32 value_ = 1.0f; ///< 0 to 1.
  f32 alpha_ = 1.0f; ///< 0 to 1.
  Color start_;
  Drag drag_ = kDragNone;
  bool was_down_ = false;
  bool is_started_ = false;
  bool is_square_dirty_ = true; ///< The square must use the new hue.
  bool is_alpha_dirty_ = true; ///< The alpha bar must use the new color.
  bool is_textures_tried_ = false;
  Texture* square_ = nullptr;
  Texture* hue_bar_ = nullptr;
  Texture* alpha_bar_ = nullptr;
};
} // namespace ui
