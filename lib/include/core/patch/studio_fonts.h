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
 * @file studio_fonts.h
 * @brief Puts the fonts of Sango Studio in the fonts that the game already loaded.
 *
 * The page "Font" of Sango Studio writes `sdmc:/sango-studio/font/[n].bin`.
 * The game loads the fonts of the archive of the fonts (core::Archive replaces
 * the file when the game reads it). The two system fonts (files 0 and 1) are
 * loaded when the game starts, before the plugin starts, so the replacement
 * of the archive comes too late for them.
 *
 * This feature finds the font in memory and writes the new pictures and the
 * new widths in it. It works only when the file of the studio has the size and
 * the layout of the font of the game: the page of the studio always makes such
 * a file. The pointers that the game wrote in the font are not changed.
 *
 * ORAS and XY: the feature looks at the memory only, it has no address.
 */

#pragma once

#include "common.h"

namespace core {

/// Writes the fonts of the studio in the fonts that are in memory.
class StudioFonts {
public:
  /// Does the work for the files 0 to 4 that exist on the SD card.
  static void LoadAll();

  /// Does the work again for a file of reload.bin, for example "font/0.bin".
  static void Apply(const c8* path);

  /// Returns true when the path is a font of the studio.
  static bool Owns(const c8* path);
};

} // namespace core
