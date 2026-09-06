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

#pragma once

#include <types.h>

enum class MapId : u16 {
  kLittlerootTown = 6,
  kBrendanHouseGroundFloor = 223,
  kBrendanHouseFirstFloor = 224,
  kMayHouseGroundFloor = 225,
  kMayHouseFirstFloor = 226,
  kBirchLaboratory = 227,
  kInsideOfTruck = 399,

  kRoute101 = 23,

  kOldaleTown = 7,
  kOldalePokeCenter = 228,
  kOldalePokeMart = 229,
  kOldaleNorthHouse = 230,
  kOldaleSouthHouse = 231,

  kRoute103 = 25,

  kRoute102 = 24,

  kPetalburgCity = 13,

  kPetalburgPokeCenter = 95,
  kPetalburgPokeMart = 260,
  kPetalburgHouseCenter = 263,
  kPetalburgHouseSouth = 264,
  kPetalburgGymEnter = 431,
  kPetalburgGymRoom1 = 432,
  kPetalburgGymRoom2 = 433,
  kPetalburgGymRoom3 = 434,
  kPetalburgGymRoom4 = 435,
  kPetalburgGymRoom5 = 436,
  kPetalburgGymRoom6 = 437,
  kPetalburgGymRoom7 = 438,
  kPetalburgGymFinalRoom = 439,

  kWallyHouseGroundFloor = 261,
  kWallyHouseFirstFloor = 262,

  kRoute104South = 27,
  kPetalburgWoods = 82,
  kRoute104North = 26,

  kRustboroCity = 16,
  kRoute115 = 42, // NO POKEMON
  kRoute116 = 43,
};
