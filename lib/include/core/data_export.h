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
 * @file data_export.h
 * @brief Writes data of the game to the data folder, in the format of the
 * files of Sango Maker.
 *
 * A ROM hacker does not start from an empty file: the Sango Maker page of the
 * menu writes the wild Pokémon of the current map, a species or the team of
 * the player. Copy the text into encounters.txt, species.txt or trainers.txt,
 * then change it.
 *
 * The names are in the form of the files: lowercase, without spaces
 * ("thundershock"). core::DataFiles reads them.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/species.h"

namespace core {

/// Writes data of the game in the format of the files of Sango Maker.
class DataExport {
public:
  /// Writes the wild Pokémon of the current map to export_encounters.txt
  /// (Alpha Sapphire). Returns false when the player is not on a map.
  static bool WriteEncounters();

  /// Writes the stats, the types, the abilities and the catch rate of a
  /// species to export_species.txt.
  static bool WriteSpecies(SpeciesId species);

  /// Writes the team of the player to export_team.txt, as the Pokémon of a
  /// trainer of trainers.txt. Returns false when the team is empty.
  static bool WriteTeam();
};

} // namespace core
