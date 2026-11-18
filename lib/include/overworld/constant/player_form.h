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
 * @file player_form.h
 * @brief The values that the scripts use for the player form.
 */

#pragma once

#include <types.h>

namespace overworld {

/// The player form, as a script writes it.
enum class PlayerForm : u8 {
  kWalking = 0,
  kMachBike = 1,
  kAcroBike = 2,
  kRollerSkates = 3,
  kSurfing = 4,
  kRhyhornOrMamoswineRideStartClimb = 5, ///< Rhyhorn or Mamoswine ride (start of the climb).
  kGogoatRide = 6,
  kRhyhornOrMamoswineRideFast = 7, ///< Rhyhorn or Mamoswine ride (fast).
  kBigGogoatRide = 8,
  kSitting = 9,
  kClimbing = 10,
  kDowsing = 11,
  kStealthySteps = 12,
  kSurfingOnKyogre = 13,
  kSurfingOnSharpedo = 14,
  kSurfingOnWailmer = 15,
  kDiving = 16,
  kDivingWithKyogre = 17,
  kDivingWithSharpedo = 18,
  kDivingWithWailmer = 19,
  kNone = 20,
};

} // namespace overworld
