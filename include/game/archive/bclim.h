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
#include "game/constant/texture_format.h"

/// CLIM = CTR Layout IMage. Unlike Garc, this header sits at the END of
/// the file as a footer: pixel data comes first, then this struct. Locate
/// it using the file's total size (e.g. from Garc::GetFileSize):
///   uptr file = garc->GetFileAddress(item_id);
///   u32 size = garc->GetFileSize(item_id);
///   auto* footer = (const BclimFooter*)(file + size - sizeof(BclimFooter));
struct BclimFooter {
  u32 signature; // Identifies the block: 'CLIM'
  u16 byte_order;
  u16 header_size;
  u32 version;
  u32 file_size; // Size of the whole .bclim file (pixel data + this footer)
  u16 block_count; // Always 1 (the "imag" block below)
  u16 _0; // Padding
  u32 imag_signature; // Identifies the block: 'imag'
  u32 imag_size; // Size of the fields below, always 0x10
  u16 width;
  u16 height;
  u16 needed_alignment;
  TextureFormat format;
  u8 flags;
  u32 pixel_data_size; // Size of the pixel data before this footer

  // Every pixel here is 2 bytes (RGB565 or RGBA4, the only formats seen
  // for item icons)
  static constexpr u32 kBytesPerPixel = 2;

  INLINE uptr GetPixelData() const {
    return (uptr)this - pixel_data_size;
  }

  // PICA200 doesn't store texture data row by row: pixels are grouped in
  // 8x8 tiles, and inside a tile they're ordered by interleaving the bits
  // of the local x/y coordinates (Z-order / Morton order) instead of going
  // left to right. This turns (x, y) into the right byte offset so the
  // caller never has to think about the swizzle.
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

  INLINE u16 GetPixel(u32 x, u32 y) const {
    return *(u16*)(GetPixelData() + GetPixelOffset(x, y));
  }

  INLINE void SetPixel(u32 x, u32 y, u16 raw_color) const {
    *(u16*)(GetPixelData() + GetPixelOffset(x, y)) = raw_color;
  }

  // Packs 8-bit components down to RGB565 (5-6-5 bits). Only meaningful
  // when format is TextureFormat::kRgb565.
  INLINE void SetPixelRgb565(u32 x, u32 y, u8 r, u8 g, u8 b) const {
    u16 packed = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
    SetPixel(x, y, packed);
  }

  // Packs 8-bit components down to RGBA4 (4-4-4-4 bits). Only meaningful
  // when format is TextureFormat::kRgba4.
  INLINE void SetPixelRgba4(u32 x, u32 y, u8 r, u8 g, u8 b, u8 a) const {
    u16 packed = ((r >> 4) << 12) | ((g >> 4) << 8) | ((b >> 4) << 4) |
                (a >> 4);
    SetPixel(x, y, packed);
  }
};
