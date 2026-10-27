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
 * @file texture_editor.h
 * @brief Reads and writes the pixels of the textures of the 3D models.
 */

#pragma once

#include "common.h"

namespace renderer {
struct H3dModel;

/// The texture of a material that the editor changes.
struct TextureTarget {
  const H3dModel* model = nullptr;
  u32 material = 0;
  u32 unit = 0;
  uptr object = 0; ///< The texture object of the game: it tells when the game changes it.
  u8* pixels = nullptr; ///< The pixels, for the CPU.
  u32 width = 0;
  u32 height = 0;
  u8 format = 0; ///< A value of TextureFormat.
  u8 mipmap_count = 1;

  /// The number of bytes of one pixel.
  u32 GetBytesPerPixel() const;
  /// The number of bytes of the first level of the texture.
  u32 GetSize() const { return width * height * GetBytesPerPixel(); }
};

/**
 * @brief Reads and writes the pixels of the textures of the 3D models.
 *
 * The game makes the textures of the Pokémon while it runs, in the memory of
 * the GPU. The editor reads the address, the size and the format of such a
 * texture from the model, and it reads and writes the pixels in place. The
 * pixels have the native format of the GPU: tiles of 8 x 8 pixels, the pixels
 * of a tile in Morton order, and the rows from the bottom of the image. The
 * functions of this class hide it: the pixel (0, 0) is the top-left pixel.
 *
 * Only the first level of the texture changes. The formats with a fixed
 * color count (the ETC1 formats) are not supported. ORAS only.
 *
 * The editor can keep a changed texture on the SD card, in the folder
 * sdmc:/sango/textures. A file is named after the original pixels of the
 * texture (a hash of them), so that any model with the same texture gets the
 * change again when the game makes it.
 */
class TextureEditor {
public:
  /// The biggest side of a texture that the editor accepts, in pixels.
  static constexpr u32 kMaxSide = 256;

  /**
   * @brief Finds the texture of a unit of a material.
   * @param error Receives a short sentence when the function returns false.
   */
  static bool Locate(const H3dModel* model, u32 material, u32 unit,
                     TextureTarget& out, const c8** error);

  /// Returns true when the texture is still the one that Locate() found: the
  /// game can destroy a model or its texture.
  static bool IsValid(const TextureTarget& target);

  /// Returns true for a format that the editor can write.
  static bool IsFormatSupported(u8 format);

  /// Returns true when the format has an alpha channel.
  static bool HasAlpha(u8 format);

  /// The name of a format, for the menu.
  static const c8* GetFormatName(u8 format);

  /// Reads a pixel. `rgba` receives four bytes. y = 0 is the top row.
  static void ReadPixel(const TextureTarget& target, u32 x, u32 y, u8* rgba);

  /// Writes a pixel. The format keeps less bits than 8 for some formats.
  static void WritePixel(const TextureTarget& target, u32 x, u32 y,
                         const u8* rgba);

  /// Writes the cache of the CPU to the memory: the GPU reads the memory.
  static void Flush(const TextureTarget& target);

  /// Reads the list of the saved textures. Call it one time at the start.
  static void Initialize();

  /// Call it one time for each frame: it gives the saved textures to the new
  /// models, when the game has written their pixels.
  static void Update();

  /// Tells the editor that the game put a model in a scene.
  static void OnModelAdded(const H3dModel* model);

  /// Returns the hash of the original pixels of a texture. If the editor gave
  /// a saved texture to the model, the hash is the one of the saved original.
  static u32 GetOriginalHash(const TextureTarget& target);

  /// Writes the pixels of the texture to the SD card. Returns true on success.
  static bool Save(const TextureTarget& target, u32 original_hash);

  /// Deletes the saved texture. Returns true when a file was saved.
  static bool Forget(const TextureTarget& target, u32 original_hash);

  /// Returns true when the original texture has a saved file.
  static bool IsSaved(const TextureTarget& target, u32 original_hash);
};

} // namespace renderer
