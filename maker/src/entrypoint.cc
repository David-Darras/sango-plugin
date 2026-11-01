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
 * @file entrypoint.cc
 * @brief Sango Maker: a ROM hack without code.
 *
 * The files of the data folder (sdmc:/sango/maker) change the game:
 * encounters.txt, trainers.txt, species.txt, story.txt, characters.txt and
 * rules.txt (see core::DataFiles), and the files of the editors of the menu
 * (scripts.txt, map_events.txt, zones.txt, texts.txt, maps/, translations/).
 * The menu has all the pages of the library: the editors make the files.
 */

#include "sango.h"
#include "core/data_files.h"

void SetUpHack() {
  core::DataFiles::Load();
}
