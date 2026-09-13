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

#include <3ds/types.h>


namespace overworld {

enum class ActionCommand : u16 {
  kNone = 0,
  kFace = 1,
  kFaceRotate32Frames = 2,
  kFaceRotate16Frames = 3,
  kFaceRotate14Frames = 4,
  kFaceRotate12Frames = 5,
  kFaceRotate8Frames = 6,
  kFaceRotate4Frames = 7,
  kWalk32Frames = 11,
  kWalk16Frames = 12,
  kWalk8Frames = 13,
  kWalk4Frames = 14,
  kWalk2Frames = 15,
  kWalk1Frame = 16,
  kWalk12Frames = 17,
  kWalk14Frames = 18,
  kWalk7Frames = 19,
  kWalk6Frames = 20,
  kWalk3Frames = 21,
  kWalk5Frames = 22,
  kDash6Frames = 23,
  kSlowWalk12Frames = 24,
  kStealthyWalk16Frames = 25,
  kTurn2Frames = 26,
  kWalkInPlace32Frames = 27,
  kWalkInPlace16Frames = 28,
  kWalkInPlace8Frames = 29,
  kWalkInPlace4Frames = 30,
  kWalkInPlace2Frames = 31,
  kJumpInPlace16Frames = 32,
  kJumpInPlace8Frames = 33,
  kJump1Tile8Frames = 35,
  kJump1Tile16Frames = 36,
  kJump2Tiles16Frames = 37,
  kJump3Tiles16Frames = 38,
  kWait1Frame = 41,
  kWait2Frames = 42,
  kWait4Frames = 43,
  kWait8Frames = 44,
  kWait15Frames = 45,
  kHalfSitStart = 148,
  kHalfSitWait = 149,
  kHalfSitEnd = 150,
  kKneelStart = 151,
  kKneelWait = 152,
  kKneelEnd = 153,
  kTurn8Frames = 250,
  kTurn12Frames = 251,
};

} // namespace overworld

using overworld::ActionCommand;
