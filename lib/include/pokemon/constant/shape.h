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
 * @file shape.h
 * @brief The shape of a species.
 */

#pragma once

#include <types.h>

namespace pokemon {

/// Shape.
enum class Shape : u8 {
  kRound = 0,
  kLegs = 1,
  kFish = 2,
  kInsect = 3,
  kFourLegged = 4,
  kFourWings = 5,
  kGathering = 6,
  kMultiLeg = 7,
  kUprightTorso = 8,
  kTwoLegsAndTail = 9,
  kTwoLegsAndHumanForm = 10,
  kTwoWings = 11,
  kBellyCrawlingNoLegs = 12,
  kArm = 13,
  kCount = 14
};

} // namespace pokemon
