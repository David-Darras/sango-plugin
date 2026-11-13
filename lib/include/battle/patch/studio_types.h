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
 * @file studio_types.h
 * @brief The new types and the type chart that Sango Studio makes.
 *
 * In the page "Type" of Sango Studio, the user adds types (a name in English
 * and an image), changes the icons and changes the type chart. Sango Studio
 * writes these files in `sdmc:/sango-studio`:
 *
 * - `type/types.csv`: the new types. The header is
 *   `id,name,colour_type,ja,ja_kanji,fr,it,de,es,ko`, with one line for each
 *   new type (the id is from 19 to 31). `name` is the English name and
 *   `colour_type` is the type of the game (0 to 17) whose colour the buttons
 *   of the moves use in a battle. A language column that is empty means "the
 *   English name". The older header `id,name,colour_type` is read too.
 * - `type/icon/[id].bclim`: the icon of a type (50 x 20 pixels, RGBA4). It can
 *   be the icon of a new type or the icon of a type of the game (0 to 17).
 * - `type/chart.bin`: the whole type chart. The signature `TCHT`, the version
 *   1 (32 bits), the number of types on a side (32, 32 bits), then 32 x 32
 *   cells, one row for each attacking type, with the values of
 *   battle::TypeMultiplier (0, 2, 4 and 8). The number 18 ("no type") is not
 *   used.
 *
 * This feature registers the types with battle::GameExtension::AddType() and
 * gives the cells to battle::TypeChart::Set(). core::StudioFiles calls it, so
 * it works in the overlay and in every product that calls
 * core::StudioFiles::Load(). ORAS only.
 *
 * @see docs/tutorials/14-add-a-type.md
 */

#pragma once

#include "common.h"

namespace battle {

/// Reads the type files of Sango Studio.
class StudioTypes {
public:
  /// The numbers of the new types.
  static constexpr u32 kFirstId = 19;
  static constexpr u32 kLastId = 31;
  /// The size of the buffer of a name, in 16-bit units, with the zero at the
  /// end. A longer name is cut.
  static constexpr u32 kNameSize = 20;

  /// Reads the list of the types, the icons that exist and the type chart.
  /// Call it one time after the SD card is ready.
  static void LoadAll();

  /// Reads one file again. `path` is the path in reload.bin, for example
  /// "type/types.csv". The icon files are read when the game needs them: a
  /// change of an icon shows after the game restarts.
  static void Apply(const c8* path);

  /// Takes the names of the language of the game again when the player changes
  /// the language. Call it often: it is cheap.
  static void RefreshLanguage();
};

} // namespace battle
