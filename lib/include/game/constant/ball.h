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

enum class Ball : u8 {
  kNone = 0,
  kMasterBall = 1,
  kUltraBall = 2,
  kGreatBall = 3,
  kPokeBall = 4,
  kSafariBall = 5,
  kNetBall = 6,
  kDiveBall = 7,
  kNestBall = 8,
  kRepeatBall = 9,
  kTimerBall = 10,
  kLuxuryBall = 11,
  kPremierBall = 12,
  kDuskBall = 13,
  kHealBall = 14,
  kQuickBall = 15,
  kCherishBall = 16,
  kSpeedBall = 17,
  kLevelBall = 18,
  kLureBall = 19,
  kHeavyBall = 20,
  kLoveBall = 21,
  kFriendBall = 22,
  kMoonBall = 23,
  kSportBall = 24,
  kDreamBall = 25,
  kCount = 26,
};
