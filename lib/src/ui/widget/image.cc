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
 * @file image.cc
 * @brief An image of the SD card, drawn in a rectangle.
 *
 * The declarations are in ui/widget/image.h.
 */

#include "ui/widget/image.h"

#include "system/native/file.h"
#include "system/native/graphics.h"

namespace ui {
namespace {
constexpr u32 kTgaHeaderSize = 18;
constexpr u8 kTgaTrueColor = 2; ///< An image without a palette or compression.
constexpr u8 kTgaTopOrigin = 1 << 5; ///< The first row is the top row.

// The GPU needs textures with sizes that are powers of 2 (8 to 512).
// Returns the smallest of these sizes that contains `value`.
u32 GetTextureSize(u32 value) {
  u32 size = 8;
  while (size < value) size *= 2;
  return size;
}

// The place of a pixel in the 8 x 8 tiles of the GPU: the bits of x and y
// alternate (Morton order).
u32 GetTileOffset(u32 x, u32 y) {
  return (x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
         ((x & 4) << 2) | ((y & 4) << 3);
}
} // namespace

bool Image::Load() {
  is_tried_ = true;

  // The header, then the pixels.
  u8 header[kTgaHeaderSize];
  sys::File file;
  file.Open(path_, sys::File::kRead);
  if (!file.IsOpen()) return false;
  if (file.Read(header, sizeof(header)) != (s32)sizeof(header)) return false;

  const u32 id_length = header[0];
  const u32 width = header[12] | (header[13] << 8);
  const u32 height = header[14] | (header[15] << 8);
  const u32 bytes_per_pixel = header[16] / 8;
  const bool is_top_origin = (header[17] & kTgaTopOrigin) != 0;
  if (header[1] != 0 || header[2] != kTgaTrueColor) return false;
  if (bytes_per_pixel != 3 && bytes_per_pixel != 4) return false;
  if (width == 0 || height == 0 || width > kMaxSize || height > kMaxSize) {
    return false;
  }

  const u32 file_size = width * height * bytes_per_pixel;
  u8* file_pixels = new u8[file_size];
  if (file_pixels == nullptr) return false;
  if (file.Read(file_pixels, file_size, id_length) != (s32)file_size) {
    delete[] file_pixels;
    return false;
  }
  file.Close();

  // The texture: the image at its top-left corner, and transparent pixels
  // on the right and at the bottom, up to the next sizes that are powers of
  // 2. The GPU format: 8 x 8 tiles, the first tile row at the bottom of the
  // rectangle of sys::Graphics::DrawRectWithTexture(), and the bytes A, B,
  // G, R for each pixel.
  const u32 texture_width = GetTextureSize(width);
  const u32 texture_height = GetTextureSize(height);
  const u32 texture_bytes = texture_width * texture_height * 4;
  u8* pixels = new u8[texture_bytes];
  if (pixels == nullptr) {
    delete[] file_pixels;
    return false;
  }
  for (u32 i = 0; i < texture_bytes; i++) pixels[i] = 0;
  for (u32 row = 0; row < height; row++) {
    // The row from the top of the image, then from the bottom of the
    // texture: the top row of the image is the last row of the texture.
    const u32 top_row = is_top_origin ? row : height - 1 - row;
    const u32 y = texture_height - 1 - top_row;
    const u8* source = file_pixels + row * width * bytes_per_pixel;
    for (u32 x = 0; x < width; x++, source += bytes_per_pixel) {
      const u32 tile = (y / 8) * (texture_width / 8) + x / 8;
      u8* target = pixels + (tile * 64 + GetTileOffset(x % 8, y % 8)) * 4;
      // TGA gives B, G, R (and A).
      target[0] = bytes_per_pixel == 4 ? source[3] : 0xFF;
      target[1] = source[0];
      target[2] = source[1];
      target[3] = source[2];
    }
  }
  delete[] file_pixels;

  // The GPU reads the pixels from memory, maybe after this function: keep
  // them, and write the cache of the CPU to the memory first.
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (u32)pixels, texture_bytes);
  texture_ = sys::Graphics::CreateTexture(texture_width, texture_height,
                                         pixels);
  if (texture_ == nullptr) {
    delete[] pixels;
    return false;
  }
  pixels_ = pixels;
  width_ = width;
  height_ = height;
  texture_width_ = texture_width;
  texture_height_ = texture_height;
  return texture_ != nullptr;
}

bool Image::Prepare() {
  if (is_tried_) return false;
  Load();
  return true;
}

bool Image::Draw(s32 x, s32 y, Color color) {
  if (texture_ == nullptr) return false;
  // The rectangle of the full texture: the transparent pixels are on the
  // right and at the bottom of the image.
  sys::Graphics::DrawRectWithTexture(x, y, texture_width_, texture_height_,
                                     color, texture_);
  return true;
}

bool Image::DrawCentered(s32 screen_width, s32 screen_height, Color color) {
  return Draw((screen_width - (s32)width_) / 2,
              (screen_height - (s32)height_) / 2, color);
}
} // namespace ui
