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
 * @file egg_group.h
 * @brief The egg group of a species.
 */

#pragma once

#include <types.h>

namespace pokemon {

/// Egg group.
enum class EggGroup : u8 {
  kNone = 0,
  kMonster = 1,
  kWater1 = 2,
  kBug = 3,
  kFlying = 4,
  kField = 5,
  kFairy = 6,
  kGrass = 7,
  kHumanLike = 8,
  kWater3 = 9,
  kMineral = 10,
  kAmorphous = 11,
  kWater2 = 12,
  kDitto = 13,
  kDragon = 14,
  kUndiscovered = 15,
  kCount = 16
};

} // namespace pokemon
