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

enum class BattleBackground : u8 {
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

  kNone = 0,
  kGrass = 1,
  kFactory = 2,
  kCave = 3,
  kForest = 4,
  kShore = 5,
  kMountain = 6,
  kIndoors = 7,
  kCity = 8,
  kTown = 9,
  kPalace = 10,
  kLinkBattle = 11,
  kWater = 12,
  kSea = 13,
  kSnowCave = 14,
  kMirrorDungeon = 15,
  kRockGymLeader = 16,
  kFightingGymLeader = 17,
  kElectricGymLeader = 18,
  kFireGymLeader = 19,
  kNormalGymLeader = 20,
  kFlyingGymLeader = 21,
  kPsychicGymLeader = 22,
  kWaterGymLeader = 23,
  kEliteFourDark = 24,
  kEliteFourIce = 25,
  kEliteFourGhost = 26,
  kEliteFourDragon = 27,
  kChampionBattle = 28,
  kDarkRoom = 29,
  kWcs = 30,
  kWcs2 = 31,
  kRockGymLeader2 = 32,
  kFightingGymLeader2 = 33,
  kElectricGymLeader2 = 34,
  kFireGymLeader2 = 35,
  kNormalGymLeader2 = 36,
  kFlyingGymLeader2 = 37,
  kPsychicGymLeader2 = 38,
  kWaterGymLeader2 = 39,
  kSkyBattle = 40,
  kWaterCave = 41,
  kGrey = 42,
  kDesert = 43,
  kVolcanicArea = 44,
  kCityShore = 45,
  kFord = 46,
  kCityFord = 47,
  kSeabed = 48,
  kStoneCave = 49,
  kWaterfallCave = 50,
  kSeabedCave = 51,
  kWaterfall = 52,
  kUnderseaAquaHideout = 53,
  kCycling = 54,
  kShip = 55,
  kCemetery = 56,
  kSkyPillar = 57,
  kPokemonRuby = 58,
  kPokemonSapphire = 59,
  kPokemonEmerald = 60,
  kPokemonLegendary = 61,
  kContestACool = 62,
  kContestABeautiful = 63,
  kContestACute = 64,
  kContestAClever = 65,
  kContestAStrong = 66,
  kContestBCool = 67,
  kContestBBeautiful = 68,
  kContestBCute = 69,
  kContestBClever = 70,
  kContestBStrong = 71,
  kContestCCool = 72,
  kContestCBeautiful = 73,
  kContestCCute = 74,
  kContestCClever = 75,
  kContestCStrong = 76,
  kContestDCool = 77,
  kContestDBeautiful = 78,
  kContestDCute = 79,
  kContestDClever = 80,
  kContestDStrong = 81,
  kContestOpening = 82,
  kContestEnding = 83,
  kAquaBoss = 84,
  kMagmaBoss = 85,
  kSkyPillarTop = 86,
  kDeoxys = 87,
  kVolcano = 88,
  kVictoryRoad = 89,
  kLastBattleEvent = 90,
  kAquaHideoutCave = 91,
  kAbandonedShip = 92,
};
