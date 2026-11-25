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
 * @file evolution.h
 * @brief Adds the evolutions of the new species to the evolution table.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/species.h"

namespace pokemon {

/// Changes the evolution data when the game loads it.
class Evolution {
  MAKE_SINGLETON(Evolution)

public:
  static void Initialize();

private:
  static void LoadEvolveTableHook(SpeciesId species, u32 b, u32 c, u32 d);
  /// After an evolution, gives the Pokémon the form that Sango Studio chose
  /// for this evolution (`species/evolution_form/<species>_<form>.bin`: one
  /// byte for each place of the evolution table, 0xFF keeps the form).
  static void EvolveHook(void* pokemon, u32 next_species, u32 place);
  static void PatchEvolve(SpeciesId species);
};

} // namespace pokemon
