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
 * @file texture.h
 * @brief A texture of the plugin, for sys::Graphics::DrawRectWithTexture().
 */

#pragma once

#include "common.h"

namespace ui {
/**
 * @brief A texture of the plugin: RGBA8 pixels that the GPU reads.
 *
 * To draw a rectangle with a texture, the game reads only four values of
 * its texture object: the physical address of the pixels, the width, the
 * height and the format. This structure has the same layout, so the plugin
 * makes its textures without the texture functions of the game.
 *
 * The pixels are in a resident GPU heap of the game
 * (core::GameFile::AllocateDevice()): this heap exists for all the game, so
 * the texture stays valid. The texture stays in memory (the menu makes a
 * few textures one time).
 *
 * The pixels use the native format of the GPU: 8 x 8 tiles, the pixels of
 * a tile in Morton order, and the bytes A, B, G, R for each pixel. The first
 * row of the texture is the top row of the rectangle.
 *
 * @code
 * ui::Texture* texture = ui::Texture::Create(64, 32);
 * // Write texture->pixels, then:
 * texture->Flush();
 * sys::Graphics::DrawRectWithTexture(x, y, 64, 32, color, texture);
 * @endcode
 */
struct Texture {
  /// The format value of RGBA8 pixels in the native format of the GPU.
  static constexpr u32 kFormatRgba8 = 0x6752;

  f32 _0;
  u32 _1;
  u32 physical_address; ///< The address of the pixels for the GPU.
  u8* pixels; ///< The address of the pixels for the CPU.
  u32 width; ///< A power of 2, from 8 to 512.
  u32 height; ///< A power of 2, from 8 to 512.
  u32 format; ///< kFormatRgba8.
  u32 _2;

  /**
   * @brief Makes a texture with transparent pixels.
   * @param width A power of 2, from 8 to 512.
   * @param height A power of 2, from 8 to 512.
   * @return The texture, or null when there is no memory.
   */
  static Texture* Create(u32 width, u32 height);

  /// Returns the size of the pixels, in bytes.
  u32 GetSize() const { return width * height * 4; }

  /// Writes the cache of the CPU to the memory. Call it after a change of
  /// the pixels: the GPU reads the memory.
  void Flush();
};
static_assert(sizeof(Texture) == 0x20,
              "Texture must have the layout of a texture of the game");
} // namespace ui
