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
 * @file action_command.h
 * @brief The movements of the characters in the overworld.
 */

#pragma once

#include <3ds/types.h>


namespace overworld {

/// A movement of a character. The number in the name is the duration in frames. See script::Context::Animate().
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
  kSitWait = 134, ///< Sits and waits (the player sits on a chair).
  kSit = 135,
  kSitStand = 136, ///< Stands up from a seat.
  kSitRotate = 137, ///< Turns while sitting (4 frames).
  kHalfSitStart = 148,
  kHalfSitWait = 149,
  kHalfSitEnd = 150,
  kKneelStart = 151,
  kKneelWait = 152,
  kKneelEnd = 153,
  kTurn8Frames = 250,
  kTurn12Frames = 251,
  kFaceSideCameraBehindPlayer = 8, ///< Face the side (camera behind the player).
  kFaceSideStealthySteps = 9, ///< Face the side (stealthy steps).
  kFaceBack = 10, ///< Face the back (camera behind the player).
  kJumpInPlace = 34, ///< Jump in place (hiding cloak).
  kJump3Tiles24Frames = 39, ///< Jump 3 tiles, 24 frames.
  kJump3Tiles24FramesNoLandingSound = 40, ///< Jump 3 tiles, 24 frames, no landing sound.
  kWait16Frames = 46, ///< Wait, 16 frames.
  kWait32Frames = 47, ///< Wait, 32 frames.
  kHide = 48, ///< Hide.
  kShow = 49, ///< Show.
  kLockFacing = 50, ///< Lock the facing.
  kUnlockFacing = 51, ///< Unlock the facing.
  kPauseAnimation = 52, ///< Pause the animation.
  kResumeAnimation = 53, ///< Resume the animation.
  kWarpUp = 54, ///< Warp up.
  kWarpDown = 55, ///< Warp down.
  kExclamationMark = 56, ///< Exclamation mark.
  kExclamationMarkWithSound = 57, ///< Exclamation mark with sound.
  kFreeMovementWeakAcceleration = 58, ///< Free movement: weak acceleration.
  kFreeMovementStrongAcceleration = 59, ///< Free movement: strong acceleration.
  kFreeMovementBraking = 60, ///< Free movement: braking.
  kFreeMovementStrongBraking = 61, ///< Free movement: strong braking.
  kFreeMovementStartTurn = 62, ///< Free movement: start of a turn.
  kFreeMovementTurn = 63, ///< Free movement: turn.
  kFreeMovementStartFastTurn = 64, ///< Free movement: start of a fast turn.
  kFreeMovementFastTurn = 65, ///< Free movement: fast turn.
  kFreeMovementJump = 66, ///< Free movement: jump.
  kFreeMovementConstantSpeed = 67, ///< Free movement: constant speed.
  kFreeMovementEndMove = 68, ///< Free movement: end of the move.
  kFreeMovementStop = 69, ///< Free movement: stop.
  kFreeMovementConstantSpeedSpeedGivenDirectly = 70, ///< Free movement: constant speed (speed given directly).
  kRollerSkatesSpinRight = 71, ///< Roller skates: spin right.
  kRollerSkatesSpinLeft = 72, ///< Roller skates: spin left.
  kRollerSkatesParallel = 73, ///< Roller skates: parallel.
  kRollerSkatesOpenTurn = 74, ///< Roller skates: open turn.
  kRollerSkatesBackFlip = 75, ///< Roller skates: back flip.
  kRollerSkates360Right = 76, ///< Roller skates: 360 right.
  kRollerSkates360Left = 77, ///< Roller skates: 360 left.
  kRollerSkatesBio540 = 78, ///< Roller skates: bio 540.
  kFreeMovementBadGround = 79, ///< Free movement: bad ground.
  kStealthyStepsMove = 80, ///< Stealthy steps: move.
  kMachBikeStop = 81, ///< Mach Bike: stop.
  kMachBikeMove = 82, ///< Mach Bike: move.
  kMachBikeBump = 83, ///< Mach Bike: bump.
  kMachBikeSlowDownWithoutInput = 84, ///< Mach Bike: slow down without input.
  kMachBikeMoveOnSand = 85, ///< Mach Bike: move on sand.
  kMachBikeSlowDownOnSand = 86, ///< Mach Bike: slow down on sand.
  kMachBikeJump = 87, ///< Mach Bike: jump.
  kAcroBikeStop = 88, ///< Acro Bike: stop.
  kAcroBikeMove = 89, ///< Acro Bike: move.
  kAcroBikeBump = 90, ///< Acro Bike: bump.
  kAcroBikeMoveOnSand = 91, ///< Acro Bike: move on sand.
  kAcroBikeJump = 92, ///< Acro Bike: jump.
  kAcroBikeWheelieJump = 93, ///< Acro Bike: wheelie jump (ledge).
  kAcroBikeSideStep = 94, ///< Acro Bike: side step.
  kAcroBikeWheelieJumpInPlace = 95, ///< Acro Bike: wheelie jump in place.
  kAcroBikeWheelieJumpInPlaceAndTurn = 96, ///< Acro Bike: wheelie jump in place and turn.
  kAcroBikeWheelieJumpAndMove = 97, ///< Acro Bike: wheelie jump and move.
  kAcroBikeSteppingStoneJump = 98, ///< Acro Bike: stepping-stone jump.
  kAcroBikeJumpAndTurn = 99, ///< Acro Bike: jump and turn.
  kTurnOnLastFrameValueInput = 100, ///< Turn on the last frame (value of the input).
  kTurnOnLastFrameFacing = 101, ///< Turn on the last frame (facing).
  kSurfingStop = 102, ///< Surfing: stop.
  kSurfingAcceleration = 103, ///< Surfing: acceleration.
  kSurfingConstantSpeed = 104, ///< Surfing: constant speed.
  kSurfingBraking = 105, ///< Surfing: braking.
  kSurfingStrongBraking = 106, ///< Surfing: strong braking.
  kSurfingJump = 107, ///< Surfing: jump.
  kSurfingStartTurn = 108, ///< Surfing: start of a turn.
  kSurfingTurn = 109, ///< Surfing: turn.
  kSurfingEndMove = 110, ///< Surfing: end of the move.
  kSurfingStealthySteps = 111, ///< Surfing: stealthy steps.
  kRhyhornRideStop = 112, ///< Rhyhorn ride: stop.
  kRhyhornRideWalkForward = 113, ///< Rhyhorn ride: walk forward.
  kRhyhornRideWalkBack = 114, ///< Rhyhorn ride: walk back.
  kRhyhornRideTurn = 115, ///< Rhyhorn ride: turn.
  kRhyhornRideWalkForwardFast = 116, ///< Rhyhorn ride: walk forward fast.
  kRhyhornRideWalkBackFast = 117, ///< Rhyhorn ride: walk back fast.
  kRhyhornRideTurnFast = 118, ///< Rhyhorn ride: turn fast.
  kRhyhornRideWalkInPlace = 119, ///< Rhyhorn ride: walk in place.
  kRhyhornRideLieDown = 120, ///< Rhyhorn ride: lie down.
  kRhyhornRideStandUp = 121, ///< Rhyhorn ride: stand up.
  kRhyhornRideWaitLyingDown = 122, ///< Rhyhorn ride: wait lying down.
  kRhyhornRideGetOn = 123, ///< Rhyhorn ride: get on.
  kRhyhornRideGetOff = 124, ///< Rhyhorn ride: get off.
  kRhyhornRideGetOnLeft = 125, ///< Rhyhorn ride: get on (left).
  kRhyhornRideGetOffLeft = 126, ///< Rhyhorn ride: get off (left).
  kRhyhornRideRefuse = 127, ///< Rhyhorn ride: refuse.
  kRhyhornRideAction = 128, ///< Rhyhorn ride: action.
  kGogoatRideGetOn = 129, ///< Gogoat ride: get on.
  kGogoatRideGetOff = 130, ///< Gogoat ride: get off.
  kGogoatRideGetOnLeft = 131, ///< Gogoat ride: get on (left).
  kGogoatRideGetOffLeft = 132, ///< Gogoat ride: get off (left).
  kGogoatRideRefuse = 133, ///< Gogoat ride: refuse.
  kBow = 138, ///< Bow.
  kHoldOutHands = 139, ///< Hold out the hands.
  kPullTowardOneself = 140, ///< Pull toward oneself.
  kPutInBag = 141, ///< Put in the bag.
  kTakeOutBag = 142, ///< Take out of the bag.
  kTalkGesture = 143, ///< Talk gesture.
  kStopOnRollerSkates = 144, ///< Stop on roller skates (not idle).
  kWriteReport = 145, ///< Write the report.
  kWritingReport = 146, ///< Writing the report.
  kFinishedWritingReport = 147, ///< Finished writing the report.
  kGetUpFromBed = 154, ///< Get up from the bed.
  kStopLyingDown = 155, ///< Stop lying down.
  kPutOnDevonScope = 156, ///< Put on the Devon Scope.
  kSlideDownSlope = 157, ///< Slide down a slope.
  kSlideDownSlopeSecretBaseSlide = 158, ///< Slide down a slope (secret base slide).
  kSlideDownSlopeBackward = 159, ///< Slide down a slope backward.
  kSkatePipeMove = 160, ///< Skate pipe: move.
  kSkatePipeStart = 161, ///< Skate pipe: start.
  kSkatePipeEnd = 162, ///< Skate pipe: end.
  kConveyorBeltStepOn = 163, ///< Conveyor belt: step on.
  kConveyorBeltStepOff = 164, ///< Conveyor belt: step off.
  kConveyorBeltPushedBack = 165, ///< Conveyor belt: pushed back.
  kWalkWithRotate4Frames = 166, ///< Walk with a smooth turn, 4 frames.
  kCameraBehindPlayerStop = 167, ///< Camera behind the player: stop.
  kCameraBehindPlayerWalk = 168, ///< Camera behind the player: walk.
  kCameraBehindPlayerRun = 169, ///< Camera behind the player: run.
  kCameraBehindPlayerJump = 170, ///< Camera behind the player: jump.
  kCameraBehindPlayerBump = 171, ///< Camera behind the player: bump.
  kCameraBehindPlayerWalkDebug = 172, ///< Camera behind the player: walk (debug).
  kFreeMovementMoveOnSand = 173, ///< Free movement: move on sand.
  kFishingMaleStyle1Start = 174, ///< Fishing (male, style 1): start.
  kFishingMaleStyle1Cast = 175, ///< Fishing (male, style 1): cast.
  kFishingMaleStyle1WaitForBite = 176, ///< Fishing (male, style 1): wait for a bite.
  kFishingMaleStyle1Bite = 177, ///< Fishing (male, style 1): bite.
  kFishingMaleStyle1ReelIn = 178, ///< Fishing (male, style 1): reel in.
  kFishingMaleStyle1Fail = 179, ///< Fishing (male, style 1): fail.
  kFishingMaleStyle2Start = 180, ///< Fishing (male, style 2): start.
  kFishingMaleStyle2Cast = 181, ///< Fishing (male, style 2): cast.
  kFishingMaleStyle2WaitForBite = 182, ///< Fishing (male, style 2): wait for a bite.
  kFishingMaleStyle2Bite = 183, ///< Fishing (male, style 2): bite.
  kFishingMaleStyle2ReelIn = 184, ///< Fishing (male, style 2): reel in.
  kFishingMaleStyle2Fail = 185, ///< Fishing (male, style 2): fail.
  kFishingMaleStyle3Start = 186, ///< Fishing (male, style 3): start.
  kFishingMaleStyle3Cast = 187, ///< Fishing (male, style 3): cast.
  kFishingMaleStyle3WaitForBite = 188, ///< Fishing (male, style 3): wait for a bite.
  kFishingMaleStyle3Bite = 189, ///< Fishing (male, style 3): bite.
  kFishingMaleStyle3ReelIn = 190, ///< Fishing (male, style 3): reel in.
  kFishingMaleStyle3Fail = 191, ///< Fishing (male, style 3): fail.
  kFishingMaleStyle4Start = 192, ///< Fishing (male, style 4): start.
  kFishingMaleStyle4Cast = 193, ///< Fishing (male, style 4): cast.
  kFishingMaleStyle4WaitForBite = 194, ///< Fishing (male, style 4): wait for a bite.
  kFishingMaleStyle4Bite = 195, ///< Fishing (male, style 4): bite.
  kFishingMaleStyle4ReelIn = 196, ///< Fishing (male, style 4): reel in.
  kFishingMaleStyle4Fail = 197, ///< Fishing (male, style 4): fail.
  kFishingFemaleStyle1Start = 198, ///< Fishing (female, style 1): start.
  kFishingFemaleStyle1Cast = 199, ///< Fishing (female, style 1): cast.
  kFishingFemaleStyle1WaitForBite = 200, ///< Fishing (female, style 1): wait for a bite.
  kFishingFemaleStyle1Bite = 201, ///< Fishing (female, style 1): bite.
  kFishingFemaleStyle1ReelIn = 202, ///< Fishing (female, style 1): reel in.
  kFishingFemaleStyle1Fail = 203, ///< Fishing (female, style 1): fail.
  kFishingFemaleStyle2Start = 204, ///< Fishing (female, style 2): start.
  kFishingFemaleStyle2Cast = 205, ///< Fishing (female, style 2): cast.
  kFishingFemaleStyle2WaitForBite = 206, ///< Fishing (female, style 2): wait for a bite.
  kFishingFemaleStyle2Bite = 207, ///< Fishing (female, style 2): bite.
  kFishingFemaleStyle2ReelIn = 208, ///< Fishing (female, style 2): reel in.
  kFishingFemaleStyle2Fail = 209, ///< Fishing (female, style 2): fail.
  kFishingFemaleStyle3Start = 210, ///< Fishing (female, style 3): start.
  kFishingFemaleStyle3Cast = 211, ///< Fishing (female, style 3): cast.
  kFishingFemaleStyle3WaitForBite = 212, ///< Fishing (female, style 3): wait for a bite.
  kFishingFemaleStyle3Bite = 213, ///< Fishing (female, style 3): bite.
  kFishingFemaleStyle3ReelIn = 214, ///< Fishing (female, style 3): reel in.
  kFishingFemaleStyle3Fail = 215, ///< Fishing (female, style 3): fail.
  kFishingFemaleStyle4Start = 216, ///< Fishing (female, style 4): start.
  kFishingFemaleStyle4Cast = 217, ///< Fishing (female, style 4): cast.
  kFishingFemaleStyle4WaitForBite = 218, ///< Fishing (female, style 4): wait for a bite.
  kFishingFemaleStyle4Bite = 219, ///< Fishing (female, style 4): bite.
  kFishingFemaleStyle4ReelIn = 220, ///< Fishing (female, style 4): reel in.
  kFishingFemaleStyle4Fail = 221, ///< Fishing (female, style 4): fail.
  kWateringCanMaleTakeOut = 222, ///< Watering can (male: take out).
  kWateringCanMaleUse = 223, ///< Watering can (male: use).
  kWateringCanMalePutAway = 224, ///< Watering can (male: put away).
  kWateringCanFemaleTakeOut = 225, ///< Watering can (female: take out).
  kWateringCanFemaleUse = 226, ///< Watering can (female: use).
  kWateringCanFemalePutAway = 227, ///< Watering can (female: put away).
  kHarvestBerries = 228, ///< Harvest Berries.
  kQuestionMark = 229, ///< Question mark.
  kQuestionMarkWithSound = 230, ///< Question mark with sound.
  kGetItem = 231, ///< Get an item.
  kTurnWithLook4Frames = 232, ///< Turn with a look, 4 frames.
  kTurnWithLook8Frames = 233, ///< Turn with a look, 8 frames.
  kAngry = 234, ///< Angry.
  kHappy = 235, ///< Happy.
  kSad = 236, ///< Sad.
  kSurprised = 237, ///< Surprised.
  kPose = 238, ///< Pose.
  kMoveFromFreeMovementCenterTile = 239, ///< Move from free movement to the center of the tile.
  kFastWalk4Frames = 240, ///< Fast walk, 4 frames.
  kSlideDownSlopeOnBike = 241, ///< Slide down a slope (on a bike).
  kSlideDownSlopeBackwardOnBike = 242, ///< Slide down a slope backward (on a bike).
  kFaceWithoutCorrection = 243, ///< Turn to a facing (no correction of the position).
  kFaceRotate6Frames = 244, ///< Turn to a facing, smooth turn, 6 frames.
  kFaceRotate5Frames = 245, ///< Turn to a facing, smooth turn, 5 frames.
  kFaceRotate3Frames = 246, ///< Turn to a facing, smooth turn, 3 frames.
  kFaceRotate2Frames = 247, ///< Turn to a facing, smooth turn, 2 frames.
  kJumpInPlace8FramesHeight0 = 248, ///< Jump in place, 8 frames, height 0.
  kCameraBehindPlayerBumpOnBike = 249, ///< Camera behind the player: bump on a bike.
  kTurnWithMotion12FramesRight = 252, ///< Turn with motion, 12 frames, to the right.
  kTurnWithMotion12FramesLeft = 253, ///< Turn with motion, 12 frames, to the left.
  kFreeMovementWalk = 254, ///< Free movement: walk.
  kFreeMovementDash = 255, ///< Free movement: dash.
  kStealthyStepsStop = 256, ///< Stealthy steps: stop.
  kSitOnChair = 257, ///< Sit on a chair (height control).
  kStandUpFromChair = 258, ///< Stand up from a chair (height control).
  kSlidingMove4Frames = 259, ///< Sliding: move 4 frames.
  kEndList = 65535, ///< End of the list.
};

} // namespace overworld

using overworld::ActionCommand;
