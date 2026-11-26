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
 * @file smooth_icons.cc
 * @brief Pokémon icons with 16 levels of transparency.
 *
 * The declarations are in pokemon/patch/smooth_icons.h.
 */

#include "pokemon/patch/smooth_icons.h"

#include "core/hook.h"
#include "core/utils.h"
#include "pokemon/address.h"
#include "system/native/file.h"

namespace pokemon {

#ifdef GAME_ORAS
namespace {

/// The list of the icon files of Sango Studio.
constexpr const c16* kIconList = u"sdmc:/sango-studio/species/icon/index.bin";

/// The footer of an image: its signature "CLIM", the offset of the number of
/// its format and the offset of the size of its pixels.
constexpr u32 kFooterSignature = 0x4D494C43;
constexpr u32 kFooterFormat = 32;
constexpr u32 kFooterSize = 36;
constexpr u32 kFormatRgba4 = 8;

/// An image that the game reads: its pixels, and the place of their size in
/// the footer of the file.
struct Image {
  u16* pixels;
  u32 unknown;
  const u32* size;
};

/// The function of the game for the dark icon: the image, a work buffer and
/// the size of the buffer.
core::Hook<void(const Image*, u16*, u32)> dark_hook;
/// The function of the game for the icons of the 3D screens: the image and the
/// heap of the work buffer.
core::Hook<void(const Image*, void*)> model_hook;

/// True when the file of the image has the format RGBA4.
bool IsSmooth(const Image* image) {
  const u8* footer =
      reinterpret_cast<const u8*>(image->size) - kFooterSize;
  return *reinterpret_cast<const u32*>(footer) == kFooterSignature &&
         *reinterpret_cast<const u32*>(footer + kFooterFormat) == kFormatRgba4;
}

/// The game darkens the colors as RGBA5551. For an RGBA4 icon, this one
/// halves the red, the green and the blue, and keeps the transparency.
void DarkHook(const Image* image, u16* work, u32 work_size) {
  if (!IsSmooth(image)) {
    dark_hook(image, work, work_size);
    return;
  }
  core::GameFunction<void(const Image*, u16*, u32)>::Call(
      address::kExpandIcon, image, work, work_size);
  const u32 count = *image->size / sizeof(u16);
  for (u32 i = 0; i < count; i++) {
    const u16 pixel = image->pixels[i];
    image->pixels[i] = ((pixel >> 1) & 0x7770) | (pixel & 0x000F);
  }
}

/// These screens read the texture as RGBA5551: an RGBA4 icon changes to this
/// format. A pixel that is at least half opaque stays.
void ModelHook(const Image* image, void* heap) {
  const bool is_smooth = IsSmooth(image);
  model_hook(image, heap);
  if (!is_smooth) return;
  const u32 count = *image->size / sizeof(u16);
  for (u32 i = 0; i < count; i++) {
    const u16 pixel = image->pixels[i];
    const u16 r = (pixel >> 12) & 0xF;
    const u16 g = (pixel >> 8) & 0xF;
    const u16 b = (pixel >> 4) & 0xF;
    const u16 a = (pixel >> 3) & 1;
    image->pixels[i] = ((r << 1 | r >> 3) << 11) | ((g << 1 | g >> 3) << 6) |
                       ((b << 1 | b >> 3) << 1) | a;
  }
}

} // namespace
#endif

void SmoothIcons::Load() {
#ifdef GAME_ORAS
  // Without icons of the studio, the game stays as it is.
  // plugin::InitializeEngine() mounted the SD card.
  sys::File file;
  file.Open(kIconList, sys::File::kRead);
  if (!file.IsOpen()) return;
  file.Close();
  dark_hook.Install(address::kExpandDarkIcon, DarkHook);
  model_hook.Install(address::kExpandModelIcon, ModelHook);
#endif
}

} // namespace pokemon
