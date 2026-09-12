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

enum class Video : u8 {
  kTitleOmegaRuby = 2,
  kTitleAlphaSapphire = 3,
  kTruckLookAtPokedex = 4,
  kTruckShowPokemon = 5,
  kTruckOpenDoors = 6,
  kTruckJumpOutside = 7,

  kBoat = 9,

  kSkyUp = 11,
  kSkyDown = 12,
  kFlyingWithLatiosAndSteven = 13,
  kFlyingWithLatiasAndSteven = 14,
  kGreenLight = 15,

  kGroudonWalking = 20,
  kKyogreDiving = 21,

  kBrownPixels = 28,
  kRayquazaFlyOmegaRuby = 29,
  kRayquazaFlyAlphaSapphire = 30,
  kSkyPillar = 31,

  kDivingGoDown = 32,
  kDivingGoUp = 33,

  kEggHatching = 34,

  kWaterfallDown = 35,
  kWaterfallUp = 36,

  kExecuteFieldMove = 37,
  kExecuteFieldMove2 = 38,
  kBulbasaurAndWindow = 39,
  kBulbasaurAndWindow2 = 43,
  kWindow = 46,
  kFly = 47,

  kBadge1 = 48,
  kBadge2 = 49,
  kBadge3 = 50,
  kBadge4 = 51,
  kBadge5 = 52,
  kBadge6 = 53,
  kBadge7 = 54,
  kBadge8 = 55,

  kUseHmBugged = 56,

  kMegaLatiosFly = 57,
  kFlash1 = 58,
  kMegaLatiosLand = 59,
  kFlash2 = 60,

  kEvolution = 61,
  kEvolution2 = 62,
  kEvolution3 = 63,

  kReceivePokemon = 64,
  kReceivePokemon2 = 65,
  kSendPokemon = 66,

  kPokemonExchangeRed = 67,
  kPokemonExchangeBlue = 68,

  kBlackLines = 69,
  kBlackLines2 = 70,
  kSmallWindow = 84,
  kPrimoGroudon = 85,
  kPrimoKyogre = 86,
};
