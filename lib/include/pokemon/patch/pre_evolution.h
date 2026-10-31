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
 * @file pre_evolution.h
 * @brief Gives the species that a species evolves from.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/species.h"

namespace pokemon {

/**
 * @brief The species before a species in its evolution line.
 *
 * The game has the evolutions of a species, but not the species before it.
 * This class has them for all the species.
 *
 * @code
 * SpeciesId before = pokemon::PreEvolution::Get(SpeciesId::kVenusaur);
 * // before is SpeciesId::kIvysaur
 * @endcode
 */
class PreEvolution {
public:
  /// Returns the species that evolves into `species`, or SpeciesId::kNone when
  /// there is none (a species that does not evolve from another species). A
  /// species with several species before it gives the first one.
  static SpeciesId Get(SpeciesId species);
};

} // namespace pokemon
