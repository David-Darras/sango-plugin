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

enum class StatusCondition : u16 {
  kNone = 0,
  kParalysis = 1,
  kSleep = 2,
  kFreeze = 3,
  kBurn = 4,
  kPoison = 5,
  kConfusion = 6,
  kInfatuation = 7,
  kBind = 8,
  kNightmare = 9,
  kCurse = 10,
  kTaunt = 11,
  kTorment = 12,
  kDisable = 13,
  kYawn = 14,
  kHealBlock = 15,
  kGastroAcid = 16,
  kOdorSleuth = 17,
  kLeechSeed = 18,
  kEmbargo = 19,
  kPerishSong = 20,
  kIngrain = 21,
  kBlock = 22,
  kEncore = 23,
  kRoost = 24,
  kMoveLock = 25,
  kChargeLock = 26,
  kChoice = 27,
  kMustHit = 28,
  kMustHitTarget = 29,
  kMagnetRise = 30,
  kMagnetRiseCancel = 31,
  kTelekinesis = 32,
  kFreeFall = 33,
  kAccuracyUp = 34,
  kAquaRing = 35,
  kForceMoveType = 36,
  kPowder = 37,
};
