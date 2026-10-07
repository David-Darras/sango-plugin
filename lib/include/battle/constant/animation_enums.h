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

#include "core/types.h"

namespace battle {

enum class AnimationTarget : s32 {
  kAttacker = 0,
  kDefender = 1,
  kDefenderCenter = 2,
};

enum class ContestSlot : s32 {
  kFirst = 0,
  kSecond = 1,
  kThird = 2,
  kFourth = 3,
};

enum class TrainerSlot : s32 {
  kFirst = 0,
  kSecond = 1,
};

enum class BodyPoint : s32 {
  kOrigin = 0,
  kContactPoint = 1,
  kShootPoint = 2,
  kWaist = 3,
  kHeadTop = 4,
  kFront = 5,
  kFace = 6,
  kEyes = 7,
  kMouth = 8,
  kHorn = 9,
  kTail = 10,
  kFirstHand = 11,
  kSecondHand = 12,
  kFirstFoot = 13,
  kSecondFoot = 14,
  kCenter = 15,
  kBase = 16,
  kLowerJaw = 20,
};

enum class PokemonPose : s32 {
  kIdle = 0,
  kContactAttack = 1,
  kRangedAttack = 2,
  kAppeal = 3,
  kHurt = 4,
  kSpecial = 5,
  kHurtLight = 6,
  kHurtHeavy = 7,
  kFaint = 8,
  kEnterJump = 9,
  kEnterFall = 10,
  kEnterLanding = 11,
  kEnter = 12,
  kPhysicalAttack1 = 13,
  kPhysicalAttack2 = 14,
  kPhysicalAttack3 = 15,
  kPhysicalAttack4 = 16,
  kSpecialAttack1 = 17,
  kSpecialAttack2 = 18,
  kSpecialAttack3 = 19,
  kSpecialAttack4 = 20,
  kIdleB = 21,
  kIdleC = 22,
  kAttackPhysical = 30,
  kAttackSpecial = 31,
  kAttackBodyBlow = 32,
  kAttackPunch = 33,
  kAttackKick = 34,
  kAttackTail = 35,
  kAttackBite = 36,
  kAttackPeck = 37,
  kAttackRadial = 38,
  kAttackCry = 39,
  kAttackPowder = 40,
  kAttackShoot = 41,
  kAttackGuard = 42,
};

enum class PositionAdjust : s32 {
  kUnset = 0,
  kNormal = 1,
  kNoSizeAdjust = 2,
  kNoAutoFlip = 4,
  kNoAutoFlipNormal = 5,
  kNone = 8,
};

enum class MoveCurve : s32 {
  kLinear = 0,
  kFastStart = 1,
  kSlowStart = 2,
  kEaseInOut = 3,
};

enum class Axis : s32 {
  kX = 0,
  kY = 1,
  kZ = 2,
};

enum class FieldPoint : s32 {
  kAttackerSide = 0,
  kDefenderSide = 1,
  kDefenderCenter = 2,
};

enum class FollowRotation : s32 {
  kNone = 0,
  kX = 1,
  kY = 2,
  kXY = 3,
  kZ = 4,
  kXZ = 5,
  kYZ = 6,
  kXYZ = 7,
};

enum class EffectDrawOrder : s32 {
  kStandard = 0,
  kBeforePokemon = 1,
};

enum class ModelDrawMode : s32 {
  kAfterPokemon = 0,
  kWithEffects = 1,
  kBeforePokemon = 2,
};

enum class FollowSource : s32 {
  kEffect = 0,
  kModel = 1,
};

enum class WeatherKind : s32 {
  kReset = 0,
  kSun = 1,
  kRain = 2,
  kHail = 3,
  kSandstorm = 4,
  kHeavyRain = 5,
  kHarshSun = 6,
  kStrongWinds = 7,
};

enum class FadeScreen : s32 {
  kUpper = 0,
  kLower = 1,
  kBoth = 2,
};

enum class FadeColor : s32 {
  kWhite = 0,
  kBlack = 1,
  kLayout = 2,
};

enum class WaitKind : s32 {
  kKey = 0,
  kMessage = 1,
  kLoad = 2,
  kEnemyMotion = 3,
  kEnemyMotionSecond = 4,
  kTrainerMotion = 5,
  kCryFinished = 6,
  kPokemonLoad = 7,
  kSlowMessage = 8,
  kFunctionFinished = 9,
  kFade = 10,
};

enum class SoundPanMode : s32 {
  kNone = 0,
  kTowardAttacker = 1,
  kTowardDefender = 2,
};

enum class TrainerPose : s32 {
  kEnter = 0,
  kThrowBall = 1,
  kThrowBallCapture = 2,
  kLose = 3,
  kIdle = 4,
  kEnterSecond = 5,
  kThrowBallSecond = 6,
  kLoseSecond = 7,
  kWin = 8,
  kWinSpecial1 = 9,
  kWinSpecial2 = 10,
  kEnterMulti = 11,
  kThrowBallMulti1 = 12,
  kThrowBallMulti2 = 13,
  kMegaStart = 14,
  kMegaChange = 15,
};

enum class OverlayPlane : s32 {
  kFront = 0,
  kBack = 1,
};

enum class TrainerPicture : s32 {
  kFullBody = 0,
  kUpperBody = 1,
  kUpperBodyShadow = 2,
  kFullBodyByNumber = 3,
  kUpperBodyByNumber = 4,
  kUpperBodyShadowByNumber = 5,
};

enum class FieldEffectStyle : s32 {
  kGrassy = 1,
  kMisty = 2,
  kAqua = 3,
  kIcy = 4,
  kElectric = 5,
};

enum class ClusterShape : s32 {
  kPoint = 0,
  kSphere = 1,
  kCylinderFromBase = 2,
  kCylinderFromCenter = 3,
  kBox = 4,
};

enum class ClusterAnchor : s32 {
  kOrigin = 0,
  kAxisX = 1,
  kAxisY = 2,
  kAxisZ = 3,
  kPlaneX = 4,
  kPlaneY = 5,
  kPlaneZ = 6,
};

enum class ReflectionMode : s32 {
  kNone = 0,
  kBounce = 1,
  kStop = 2,
  kVanish = 3,
};

enum class BreakKind : s32 {
  kWind = 0,
  kFire = 1,
};

enum class StepCondition : s32 {
  kAlways = 0,
  kDoubleBattle = 10,
  kTripleOrRotation = 11,
  kSingleBattle = 12,
  kHorde = 13,
  kNotSingleBattle = 14,
  kTripleRotationOrHorde = 15,
  kSingleOrDouble = 16,
  kAttackerNear = 20,
  kAttackerFar = 21,
  kDefenderNear = 22,
  kDefenderFar = 23,
  kAttackerNearDefenderNear = 24,
  kAttackerNearDefenderFar = 25,
  kAttackerFarDefenderNear = 26,
  kAttackerFarDefenderFar = 27,
  kSingleBattleNearCamera = 30,
  kDefenderVisible = 40,
  kAttackerCanFly = 41,
  kAttackerCannotFly = 42,
  kDefenderCanFly = 43,
  kDefenderCannotFly = 44,
  kContestCool = 50,
  kContestBeauty = 51,
  kContestCute = 52,
  kContestSmart = 53,
  kContestTough = 54,
  kContestNotCool = 55,
  kContestNotBeauty = 56,
  kContestNotCute = 57,
  kContestNotSmart = 58,
  kContestNotTough = 59,
  kContestCrowdTiny = 60,
  kContestCrowdSmall = 61,
  kContestCrowdLarge = 62,
  kContestCrowdHuge = 63,
  kContestRankNormal = 64,
  kContestRankSuper = 65,
  kContestRankHyper = 66,
  kContestRankMaster = 67,
  kRareOnly = 70,
  kContestChampionOnly = 71,
  kNotDeoxysEvent = 80,
};

}
