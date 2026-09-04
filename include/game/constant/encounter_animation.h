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

enum class EncounterAnimation : u8 {
  kNone = 0,
  kWild = 1,
  kWildGrass = 2,
  kWildAshes = 3,
  kWildWater = 4,
  kWildDive = 5,
  kWildCave = 6,
  kWildSnow = 7,
  kWildSand = 8,
  kWildIce = 9,
  kWildTallGrass = 10,
  kWildLand = 11,
  kWildFlying = 12,
  kPokemonAmieGrass = 13,
  kPokemonAmieWater = 14,
  kPokemonX = 15,
  kPokemonY = 16,
  kPokemonZ = 17,
  kSpecial01 = 18,
  kSpecial02 = 19,
  kWildKyogre = 20,
  kWildGroudon = 21,
  kWildRayquaza = 22,
  kWildHoopaRing = 23,
  kWildRegirock = 24,
  kWildRegice = 25,
  kWildRegisteel = 26,
  kWildRegigigas = 27,
  kWildDeoxys = 28,
  kTrainer2d = 29,
  kTrainer3d = 30,
  kSupportFemale = 31,
  kSupportMale = 32,
  kRivalEarly = 33,
  kRivalLate = 34,
  kZinnia = 35,
  kChampion = 36,
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
  kKorrina = 54,
  kMockBattle3d = 55,
  kTeamMagmaGrunt = 56,
  kTeamMagmaMulti = 57,
  kTeamMagmaHorde = 58,
  kTeamMagmaBoss = 59,
  kTeamAquaGrunt = 60,
  kTeamAquaMulti = 61,
  kTeamAquaHorde = 62,
  kTeamAquaBoss = 63,
  kBattleHouse = 64,
  kBattleHouseMulti = 65,
};
