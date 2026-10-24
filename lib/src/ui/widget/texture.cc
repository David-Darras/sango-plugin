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
 * @file texture.cc
 * @brief A texture of the plugin, for sys::Graphics::DrawRectWithTexture().
 *
 * The declarations are in ui/widget/texture.h.
 */

#include "ui/widget/texture.h"

#include <3ds.h>
#include <cstring>

#include "core/game_file.h"
#include "system/address.h"

namespace ui {
namespace {
// The GPU reads the pixels at an address with this alignment.
constexpr u32 kAlignment = 128;

// Returns the physical address of GPU memory at `address`, or 0. The
// memory is in the linear memory (FCRAM) or in VRAM.
u32 GetPhysicalAddress(const void* address) {
  const u32 value = (u32)address;
  if (value >= 0x14000000 && value < 0x1C000000) {
    return value + 0x0C000000; // FCRAM: 0x20000000 for the GPU.
  }
  if (value >= 0x30000000 && value < 0x38000000) {
    return value - 0x10000000; // FCRAM (the newer linear address).
  }
  if (value >= 0x1F000000 && value < 0x1F600000) {
    return value - 0x07000000; // VRAM: 0x18000000 for the GPU.
  }
  return 0;
}
} // namespace

Texture* Texture::Create(u32 width, u32 height) {
  const u32 size = width * height * 4;
  auto* pixels = (u8*)core::GameFile::AllocateDevice(size, kAlignment);
  if (pixels == nullptr) return nullptr;
  // The memory stays (see GameFile::AllocateDevice()).
  const u32 physical_address = GetPhysicalAddress(pixels);
  if (physical_address == 0) return nullptr;

  auto* texture = new Texture();
  if (texture == nullptr) return nullptr;
  memset(texture, 0, sizeof(Texture));
  texture->physical_address = physical_address;
  texture->pixels = pixels;
  texture->width = width;
  texture->height = height;
  texture->format = kFormatRgba8;
  memset(pixels, 0, size);
  texture->Flush();
  return texture;
}

void Texture::SetPixel(u32 x, u32 y, u8 r, u8 g, u8 b, u8 a) {
  // The bits of x and y alternate inside a tile of 8 x 8 pixels.
  const u32 in_tile = (x & 1) | ((y & 1) << 1) | ((x & 2) << 1) |
                      ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3);
  const u32 tile = (y / 8) * (width / 8) + x / 8;
  u8* target = pixels + (tile * 64 + in_tile) * 4;
  target[0] = a;
  target[1] = b;
  target[2] = g;
  target[3] = r;
}

void Texture::Flush() {
  svcFlushProcessDataCache(CUR_PROCESS_HANDLE, (u32)pixels, GetSize());
}
} // namespace ui
