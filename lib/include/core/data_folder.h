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
 * @file data_folder.h
 * @brief The folder of the files of the product on the SD card.
 *
 * Each product has its own data folder: two ROM hacks on the same SD card do
 * not change the files of each other.
 * - The overlay: sdmc:/sango
 * - The other products: sdmc:/sango/<name of the product>
 *
 * The data folder holds the files of the ROM hack: the scripts, the map
 * events, the zones, the texts, the translations, the maps, the icons of the
 * new types and items, the textures, the layouts, the skeletons and the
 * settings of the menu.
 * The packs that all the products share stay in sdmc:/sango: the models of
 * the Pokémon and of the characters, the music, the maps of X and Y and the
 * logs.
 *
 * The Makefile gives the folder (PRODUCT_DATA_FOLDER).
 */

#pragma once

#include "common.h"
#include "system/native/file.h"

#ifndef PRODUCT_DATA_FOLDER
/// The data folder after sdmc:/. The Makefile sets it for each product.
#define PRODUCT_DATA_FOLDER "sango"
#endif

/// The data folder of the product: u"sdmc:/sango/myhack".
#define SANGO_DATA_FOLDER u"sdmc:/" PRODUCT_DATA_FOLDER
/// A file of the data folder: SANGO_DATA_PATH(u"scripts.txt").
#define SANGO_DATA_PATH(name) SANGO_DATA_FOLDER u"/" name
/// A file of the data folder, as a text of the menu:
/// SANGO_DATA_TEXT("scripts.txt").
#define SANGO_DATA_TEXT(name) "sdmc:/" PRODUCT_DATA_FOLDER "/" name

namespace core {

/// Makes the data folder of the product, and sdmc:/sango before it.
INLINE void MakeDataFolder() {
  sys::File::CreateDirectory(u"sdmc:/sango");
  sys::File::CreateDirectory(SANGO_DATA_FOLDER);
}

} // namespace core
