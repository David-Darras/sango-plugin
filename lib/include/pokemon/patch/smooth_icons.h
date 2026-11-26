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
 * @file smooth_icons.h
 * @brief Pokémon icons with smooth edges: 16 levels of transparency.
 *
 * An icon of the game has a palette of 16-bit colors, with a pixel that is
 * empty or full (RGBA5551). Sango Studio can write an icon with the format
 * RGBA4 instead, so the edges are smooth. The menus of the game draw it as it
 * is: the game copies the colors in a texture of the format that the file
 * gives. Two parts of the game expect the format of the game, and this class
 * corrects them:
 * - the dark icon of a Pokémon in the Pokédex;
 * - the screens that draw an icon on a 3D model (the machine of the Pokémon
 *   Center, the contests).
 */

#pragma once

#include "common.h"

namespace pokemon {

/// Shows the icons with 16 levels of transparency everywhere.
class SmoothIcons {
public:
  /// Corrects the game, when the SD card has icons of Sango Studio. Call it
  /// once at the start. Alpha Sapphire only: the other games do nothing.
  static void Load();
};

} // namespace pokemon
