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
 * @file image.h
 * @brief An image of the SD card, drawn in a rectangle.
 */

#pragma once

#include "common.h"
#include "ui/widget/texture.h"

namespace ui {
/**
 * @brief An image of the SD card (a TGA file), drawn as a texture.
 *
 * The file must be a TGA image without compression, with 24 or 32 bits for
 * each pixel, 512 x 512 pixels at most: for example 400 x 240 for the top
 * screen, 320 x 240 for the bottom screen. The image draws at its real
 * size. The texture of the GPU needs sizes that are powers of 2: the image
 * goes at the top-left corner of the texture, with transparent pixels
 * around it. tools/png_to_tga.py makes a TGA image from a PNG image.
 *
 * Prepare() reads the file and makes the texture (ui::Texture). Draw()
 * draws nothing before Prepare(), or when the file does not exist.
 *
 * @code
 * static ui::Image background(u"sdmc:/sango/menu_top.tga");
 * background.Draw(0, 0, Color(1, 1, 1, 1));
 * @endcode
 */
class Image {
public:
  /// The largest width and height of an image, in pixels.
  static constexpr u32 kMaxSize = 512;

  explicit Image(const c16* path) : path_(path) {}

  /**
   * @brief Reads the file and makes the texture, the first time only.
   * @return true when it read the file now.
   */
  bool Prepare();

  /**
   * @brief Draws the image. (x, y) is its top-left corner, in pixels.
   * @param color A color that multiplies the image (white: no change).
   * @return false when there is no image.
   */
  bool Draw(s32 x, s32 y, Color color);

  /**
   * @brief Draws the image in the center of a screen.
   * @param screen_width The width of the screen: 400 (top) or 320 (bottom).
   * @param screen_height The height of the screen: 240.
   * @param color A color that multiplies the image. Its alpha is the
   *        opacity of the image.
   * @return false when there is no image.
   */
  bool DrawCentered(s32 screen_width, s32 screen_height, Color color);

  /// Returns true when the image is ready to draw.
  bool IsLoaded() const { return texture_ != nullptr; }

private:
  /// Reads the file and makes the texture. Returns false on an error.
  bool Load();


  const c16* path_;
  Texture* texture_ = nullptr;
  u16 width_ = 0; ///< The width of the image.
  u16 height_ = 0; ///< The height of the image.
  u16 texture_width_ = 0; ///< The width of the texture (a power of 2).
  u16 texture_height_ = 0; ///< The height of the texture (a power of 2).
  bool is_tried_ = false; ///< true: Load() ran one time.
};
} // namespace ui
