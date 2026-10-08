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
 * @file encounter_method.h
 * @brief The methods of wild encounters.
 */

#pragma once

#include <types.h>

namespace overworld {

/**
 * @brief The method of a wild encounter: the tables of overworld::EncounterData.
 *
 * kXXX and kYYY are two tables of the game with an unknown use.
 */
enum class EncounterMethod : u8 {
  kWalk, kXXX, kYYY, kSurf, kRockSmash,
  kOldRod, kGoodRod, kSuperRod, kHorde,
  kMax
};

} // namespace overworld
