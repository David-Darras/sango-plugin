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

#include "battle/patch/move_animation.h"

namespace battle {

namespace {

constexpr s32 kChargeStart = 10;
constexpr s32 kWebStart = 25;
constexpr s32 kImpactStart = 30;
constexpr s32 kHitStart = 40;
constexpr s32 kDamageFrame = 45;
constexpr s32 kWebEnd = 70;
constexpr s32 kImpactEnd = 95;
constexpr s32 kHitEnd = 85;
constexpr s32 kBackdropEnd = 100;

constexpr u16 kChargeGroup = 0;
constexpr u16 kImpactGroup = 1;
constexpr u16 kHitGroup = 2;
constexpr u16 kWebGroup = 0;

constexpr u16 kChargeSoundGroup = 1;
constexpr u16 kBoltSoundGroup = 2;
constexpr u16 kHitSoundGroup = 3;

void AddSetup(MoveAnimation& a) {
  a.CameraRestore(0, 0, false, MoveCurve::kLinear, false);
  a.Sound3dSetup(0, 0, 0, 1.0f, 11.3f, 200.0f, 200.0f, 300.0f);
  a.HealthBarShowAll(0, 0, false);
  a.HealthBarShow(0, 0, AnimationTarget::kDefender, true);
  a.BackdropColorToggle(0, 0, true, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColor(5, 15, Vec3(0.0f, 0.0f, 0.15f), 0.65f);
}

void AddCharge(MoveAnimation& a) {
  a.PokemonPlayAttackPose(kChargeStart, kChargeStart, AnimationTarget::kAttacker,
                          PokemonPose::kAttackShoot);

  a.Sound3dPlay(kChargeStart, kChargeStart, 852197, 589824, 64, 0, -96, false,
                kChargeSoundGroup);
  a.Sound3dGlideToPokemon(kChargeStart, kChargeStart, AnimationTarget::kAttacker,
                          BodyPoint::kFront, Vec3(0.0f, 0.0f, 20.0f),
                          PositionAdjust::kNormal, kChargeSoundGroup);
  a.Sound3dGlideToPokemon(kChargeStart, kWebStart, AnimationTarget::kDefender,
                          BodyPoint::kFront, Vec3(0.0f, 0.0f, 50.0f),
                          PositionAdjust::kNormal, kChargeSoundGroup);

  a.EffectSpawn(kChargeStart, kImpactStart + 10,
                EffectId::kVoltTackleElectroBallElectrowebWildCharge, true,
                EffectDrawOrder::kStandard, kChargeGroup);
  a.EffectResize(kChargeStart, kChargeStart, Vec3(0.1f, 0.1f, 0.1f), kChargeGroup);
  a.EffectResize(kChargeStart, kChargeStart + 6, Vec3(1.5f, 1.5f, 1.5f),
                 kChargeGroup);
  a.EffectGlideToPokemon(kChargeStart, kChargeStart, AnimationTarget::kAttacker,
                         BodyPoint::kShootPoint, Vec3(0.0f, 0.0f, 0.0f),
                         PositionAdjust::kNormal, kChargeGroup);
  a.EffectGlideToPokemon(kChargeStart, kWebStart, AnimationTarget::kDefender,
                         BodyPoint::kFront, Vec3(0.0f, 0.0f, 0.0f),
                         PositionAdjust::kNormal, kChargeGroup);
}

void AddWeb(MoveAnimation& a) {
  a.ModelSpawn(kWebStart, kWebEnd, EffectModelId::kSurfModel,
               ModelDrawMode::kWithEffects, true, false, false, false, kWebGroup);
  a.ModelGlideToPokemon(kWebStart, kWebStart, AnimationTarget::kDefender,
                        BodyPoint::kFront, Vec3(0.0f, 0.0f, 0.0f),
                        PositionAdjust::kNormal, kWebGroup);
  a.ModelPlayTextureAnimation(kWebStart, kWebStart,
                    EffectModelId::kSurfModelAn, false, kWebGroup);
  a.ModelTint(kWebStart, kWebStart, "Comb1", 0, Vec3(0.3f, 0.7f, 1.0f), 1.0f,
              kWebGroup);
  a.ModelResize(kWebStart, kWebStart, Vec3(0.1f, 0.1f, 0.1f), kWebGroup);
  a.ModelResize(kWebStart, kWebStart + 5, Vec3(6.0f, 6.0f, 6.0f), kWebGroup);
  a.ModelTurn(kWebStart, kWebEnd, Vec3(0.0f, 0.0f, 360.0f), false, kWebGroup);
  a.ModelGlideToPokemon(kWebStart, kWebStart + 5, AnimationTarget::kDefender,
                        BodyPoint::kFront, Vec3(0.0f, 0.0f, 35.0f),
                        PositionAdjust::kNormal, kWebGroup);
  a.ModelResize(kWebEnd - 6, kWebEnd, Vec3(0.01f, 0.01f, 0.01f), kWebGroup);
}

void AddImpact(MoveAnimation& a) {
  a.Sound3dPlay(kImpactStart, kImpactStart, 852037, 589827, 100, 0, 0, false,
                kBoltSoundGroup);
  a.Sound3dGlideToPokemon(kImpactStart, kImpactStart, AnimationTarget::kDefender,
                          BodyPoint::kWaist, Vec3(0.0f, 0.0f, 0.0f),
                          PositionAdjust::kNormal, kBoltSoundGroup);

  a.EffectSpawn(kImpactStart, kImpactEnd,
                EffectId::kThunderboltZapCannonDischarge, true,
                EffectDrawOrder::kStandard, kImpactGroup);
  a.EffectGlideToPokemon(kImpactStart, kImpactStart, AnimationTarget::kDefender,
                         BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f),
                         PositionAdjust::kNormal, kImpactGroup);
  a.EffectResize(kImpactStart, kImpactStart, Vec3(2.5f, 2.5f, 2.5f), kImpactGroup);

  a.BackdropColor(kImpactStart, kImpactStart + 3, Vec3(1.0f, 1.0f, 0.4f), 0.8f);
  a.BackdropColor(kImpactStart + 3, kHitStart, Vec3(0.0f, 0.0f, 0.15f), 0.65f);
}

void AddHit(MoveAnimation& a) {
  a.Sound3dPlay(kHitStart, kHitStart, 852194, -1, 120, 0, 40, false,
                kHitSoundGroup);
  a.Sound3dGlideToPokemon(kHitStart, kHitStart, AnimationTarget::kDefender,
                          BodyPoint::kFront, Vec3(0.0f, 0.0f, 20.0f),
                          PositionAdjust::kNormal, kHitSoundGroup);

  a.EffectSpawn(kHitStart, kHitEnd, EffectId::kCommon10, true,
                EffectDrawOrder::kStandard, kHitGroup);
  a.EffectGlideToPokemon(kHitStart, kHitStart, AnimationTarget::kDefender,
                         BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f),
                         PositionAdjust::kNormal, kHitGroup);
  a.EffectResize(kHitStart, kHitStart, Vec3(2.5f, 2.5f, 2.5f), kHitGroup);

  a.PokemonPlayPose(kHitStart, kHitStart, AnimationTarget::kDefender,
                    PokemonPose::kHurt);
  a.KnockbackFlag(kHitStart, kHitStart, AnimationTarget::kDefender, true);
  a.HealthBarApplyDamage(kDamageFrame, kDamageFrame, AnimationTarget::kDefender,
                         true);
}

void AddEnd(MoveAnimation& a) {
  a.BackdropColor(kHitEnd - 10, kBackdropEnd, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColorToggle(kBackdropEnd, kBackdropEnd, true, Vec3(0.0f, 0.0f, 0.0f),
                        0.0f);
}

}

void BuildThunderboltWebAnimation(MoveAnimation& a) {
  AddSetup(a);
  AddCharge(a);
  AddWeb(a);
  AddImpact(a);
  AddHit(a);
  AddEnd(a);
}

void RegisterThunderboltWebAnimation() {
  MoveAnimations::Define(MoveId::kThunderbolt, BuildThunderboltWebAnimation);
}

}
