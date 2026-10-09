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
 * @file bclim.h
 * @brief The BCLIM format: an image of a 2D layout.
 */

#pragma once

#include "common.h"
#include "renderer/constant/texture_format.h"

namespace renderer {
/**
 * @brief The footer of a BCLIM file (CTR Layout IMage).
 *
 * The pixel data comes first. This structure is at the end of the file.
 * Use the size of the file to find it:
 *
 * @code
 * uptr file = garc->GetFileAddress(item_id);
 * u32 size = garc->GetFileSize(item_id);
 * auto* footer = (const BclimFooter*)(file + size - sizeof(BclimFooter));
 * @endcode
 */
struct BclimFooter {
  u32 signature; ///< The signature of the block: 'CLIM'.
  u16 byte_order; ///< 0xFEFF: little-endian.
  u16 header_size;
  u32 version; ///< 0x02020000.
  u32 file_size; ///< The size of the full file (pixel data and footer).
  u16 block_count; ///< Always 1 (the "imag" block).
  u16 _0;
  u32 imag_signature; ///< The signature of the block: 'imag'.
  u32 imag_size; ///< The size of the next members: always 0x10.
  u16 width;
  u16 height;
  u16 needed_alignment;
  TextureFormat format;
  u8 flags;
  u32 pixel_data_size; ///< The size of the pixel data before this footer.

  /// The size of one pixel: 2 bytes. The item icons use RGB565 or RGBA4
  /// only.
  static constexpr u32 kBytesPerPixel = 2;

  /// Returns the address of the pixel data.
  INLINE uptr GetPixelData() const {
    return (uptr)this - pixel_data_size;
  }

  /**
   * @brief Returns the byte offset of the pixel (x, y).
   *
   * The GPU of the 3DS (PICA200) does not store the pixels row by row. It
   * groups them in tiles of 8 x 8 pixels. In a tile, the order mixes the bits
   * of x and y (Z-order, or Morton order). This function does the
   * conversion.
   */
  INLINE u32 GetPixelOffset(u32 x, u32 y) const {
    u32 tiles_per_row = width / 8;
    u32 tile_x = x / 8, tile_y = y / 8;
    u32 local_x = x % 8, local_y = y % 8;

    u32 morton = 0;
    for (u32 bit = 0; bit < 3; bit++) {
      morton |= ((local_x >> bit) & 1) << (2 * bit);
      morton |= ((local_y >> bit) & 1) << (2 * bit + 1);
    }

    u32 tile_index = tile_y * tiles_per_row + tile_x;
    return (tile_index * 64 + morton) * kBytesPerPixel;
  }

  /// Returns the raw value of the pixel (x, y).
  INLINE u16 GetPixel(u32 x, u32 y) const {
    return *(u16*)(GetPixelData() + GetPixelOffset(x, y));
  }

  /// Sets the raw value of the pixel (x, y).
  INLINE void SetPixel(u32 x, u32 y, u16 raw_color) const {
    *(u16*)(GetPixelData() + GetPixelOffset(x, y)) = raw_color;
  }

  /// Sets the pixel (x, y) from 8-bit components, in RGB565 (5-6-5 bits).
  /// Use it only when the format is TextureFormat::kRgb565.
  INLINE void SetPixelRgb565(u32 x, u32 y, u8 r, u8 g, u8 b) const {
    u16 packed = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    SetPixel(x, y, packed);
  }

  /// Sets the pixel (x, y) from 8-bit components, in RGBA4 (4-4-4-4 bits).
  /// Use it only when the format is TextureFormat::kRgba4.
  INLINE void SetPixelRgba4(u32 x, u32 y, u8 r, u8 g, u8 b, u8 a) const {
    u16 packed = ((r >> 4) << 12) | ((g >> 4) << 8) | ((b >> 4) << 4) |
                (a >> 4);
    SetPixel(x, y, packed);
  }
};
} // namespace renderer
