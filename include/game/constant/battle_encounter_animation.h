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

enum class BattleEncounterAnimation : u8 {
  kNone = 0,
  kWildGeneric = 1,
  kWildGrass = 2,
  kWildAshes = 3,
  kWildWater = 4,
  kWildDive = 5,
  kWildCave = 6,
  kWildSnow = 7,
  kWildSand = 8,
  kWildIce = 9,
  kWildLongGrass = 10,
  kWildLand = 11,
  kWildFlight = 12,
  kContactGrass = 13,
  kContactWater = 14,
  kPokeX = 15,
  kPokeY = 16,
  kPokeZ = 17,
  kWildSpecial01 = 18,
  kWildSpecial02 = 19,
  kKyogre = 20,
  kGroudon = 21,
  kRayquaza = 22,
  kHoopa = 23,
  kRegirock = 24,
  kRegice = 25,
  kRegisteel = 26,
  kRegigigas = 27,
  kDeoxys = 28,
  kTrainerNormal2d = 29,
  kTrainerNormal3d = 30,
  kSupportFemale3d = 31,
  kSupportMale3d = 32,
  kRival3dFirstHalf = 33,
  kRival3dSecondHalf = 34,
  kDeltaTrainer3d = 35,
  kChampion3d = 36,
  kTrainerMulti2d = 37,
  kTrainerMulti3d = 38,
  kGymLeaderRock = 39,
  kGymLeaderFighting = 40,
  kGymLeaderElectric = 41,
  kGymLeaderFire = 42,
  kGymLeaderNormal = 43,
  kGymLeaderFlying = 44,
  kGymLeaderPsychic = 45,
  kGymLeaderWater = 46,
  kEliteFourDark = 47,
  kEliteFourIce = 48,
  kEliteFourGhost = 49,
  kEliteFourDragon = 50,
  kLinkBattle = 51,
  kWcsQualifier = 52,
  kWcsFinal = 53,
  kSpecialTrainerCorni = 54,
  kMock3dTrainer = 55,
  kMagmaTeam = 56,
  kMagmaTeamMulti = 57,
  kMagmaTeamBand = 58,
  kMagmaTeamBoss = 59,
  kAquaTeam = 60,
  kAquaTeamMulti = 61,
  kAquaTeamBand = 62,
  kAquaTeamBoss = 63,
  kBattleHouseXy = 64,
  kBattleHouseXyMulti = 65,
};
