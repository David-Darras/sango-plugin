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
 * @file exit_type.h
 * @brief The values that the scripts use for the exit type.
 */

#pragma once

#include <types.h>

namespace overworld {

/// The exit type, as a script writes it.
enum class ExitType : u8 {
  kNone = 0,
  kFadeOnly = 1,
  kMat = 2,
  kDoor = 3,
  kScriptOnly = 4,
  kStairs = 5,
  kWarp = 6,
  kSecretBaseEntrance = 7,
  kMatWithoutFadeIn = 8,
  kStairsWithoutFadeIn = 9,
  kSpecial1 = 15,
  kSpecial2 = 16,
  kSpecial3 = 17,
  kSpecial4 = 18,
  kSpecial5 = 19,
  kSpecial6 = 20,
  kSpecial7 = 21,
  kSpecial8 = 22,
  kSpecial9 = 23,
  kSpecial10 = 24,
  kSpecial11 = 25,
  kSpecial12 = 26,
  kSpecial13 = 27,
  kSpecial14 = 28,
  kSpecial15 = 29,
  kSpecial16 = 30,
  kSpecial17 = 31,
  kSpecial18 = 32,
  kSpecial19 = 33,
  kSpecial20 = 34,
  kSpecial21 = 35,
  kSpecial22 = 36,
  kSpecial23 = 37,
  kSpecial24 = 38,
  kSpecial25 = 39,
  kSpecial26 = 40,
  kSpecial27 = 41,
  kSpecial28 = 42,
  kSpecial29 = 43,
  kSpecial30 = 44,
  kSpecial31 = 45,
  kSpecial32 = 46,
  kSpecial33 = 47,
  kSpecial34 = 48,
  kSpecial35 = 49,
  kSpecial36 = 50,
  kSpecial37 = 51,
  kSpecial38 = 52,
  kSpecial39 = 53,
  kSpecial40 = 54,
  kSpecial41 = 55,
  kSpecial42 = 56,
  kSpecial43 = 57,
  kSpecial44 = 58,
  kSpecial45 = 59,
  kSpecial46 = 60,
  kSpecial47 = 61,
  kSpecial48 = 62,
  kSpecial49 = 63,
  kSpecial50 = 64,
};

} // namespace overworld
