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
 * @file type_icon.h
 * @brief The icons of the new types in the layouts of the game.
 */

#pragma once

#include "common.h"

namespace renderer {

/**
 * @brief Gives the layouts of the game the icons of the new types.
 *
 * The layouts (the move list, the summary, the bag...) find the icon of a type
 * by its number in the resource of the common graphic font: the icon of the
 * type Normal, then the next 17 images. A new type has no image there. The
 * plugin changes the function that gives the image of the resource: for the
 * number of a type that has a name (see battle::TypeSpec), it gives the image
 * that it reads from the SD card, sdmc:/sango/types/<name>.bclim. The same is done for the animations of
 * the summary and of the party, which swap the texture of the icon. Without
 * the file, a new type shows the icon of the type Fairy.
 *
 * The image is 50 x 20 pixels, in the format RGBA4, as the icons of the game
 * (see tools/png_to_bclim.py).
 *
 * The buttons of the moves of a battle also have the colour of the type: the
 * game picks it with a frame of an animation, and a new type has none. The
 * plugin gives the frame of the colour_type of the type to its buttons. ORAS
 * only.
 */
class TypeIcon {
public:
  /// Installs the hooks. Call it one time at the start.
  static void Initialize();

  /**
   * @brief Reads the image of a type from the SD card.
   * @param path The BCLIM file.
   * @param buffer Gets the file.
   * @param capacity The size of the buffer.
   * @return The size of the BCLIM file (the footer is at its end), or 0 when
   *         the file does not exist or is not a BCLIM image.
   */
  static u32 ReadFile(const c16* path, u8* buffer, u32 capacity);
};

} // namespace renderer
