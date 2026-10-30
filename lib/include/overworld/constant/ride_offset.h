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
 * @file ride_offset.h
 * @brief The seat of the player on the Pokémon that he rides, for each species.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/species.h"

namespace overworld {
/// A value of the table that means: use the value of the menu.
constexpr f32 kRideMenu = 1000.0f;

/// The seat of the player on one species.
struct RideOffset {
  SpeciesId species;
  f32 height;
  ///< The height of the seat above the back bone (the same unit as "Ride Height").
  f32 back;
  ///< How far the seat goes back along the Pokémon (the same unit as "Ride Back").
};

/**
 * The species that need their own seat. A species that is not in the list uses
 * the values of the menu (Cheats, "Ride Height" and "Ride Back"). A number
 * that is kRideMenu also uses the value of the menu. To fix a species, change
 * its numbers (the unit is the one of the menu), or add a line.
 *
 * To add a species, add a line with its two numbers (a number that is
 * kRideMenu keeps the value of the menu).
 */
constexpr RideOffset kRideOffsets[] = {
    {SpeciesId::kArcanine, -16, 46},
    {SpeciesId::kRaikou, -16, 46},
    {SpeciesId::kPersian, -32, 2},
    {SpeciesId::kTentacruel, -2, 68},
    {SpeciesId::kDodrio, -26, 22},
    {SpeciesId::kTauros, -18, 30},
    {SpeciesId::kGyarados, -14, 30},
    {SpeciesId::kLapras, 38, -12},
    {SpeciesId::kAerodactyl, -28, 36},
    {SpeciesId::kZapdos, -28, 36},
    {SpeciesId::kArticuno, -28, 36},
    {SpeciesId::kMoltres, -28, 36},
    {SpeciesId::kDragonite, -26, 58},
    {SpeciesId::kGroudon, 30, 54},
};

/// Returns the seat of a species, or null when the menu values are for it.
inline const RideOffset* FindRideOffset(SpeciesId species) {
  for (const RideOffset& entry : kRideOffsets) {
    if (entry.species == species) return &entry;
  }
  return nullptr;
}
} // namespace overworld