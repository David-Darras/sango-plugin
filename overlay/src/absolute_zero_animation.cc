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
 * @file absolute_zero_animation.cc
 * @brief Example: a new animation for the move Absolute Zero (MoveAnimations::Define).
 */

#include "custom_battle.h"
#include "battle/patch/move_animation.h"

namespace battle {

namespace {

constexpr u32 kLightningStrikeCount = 5;
constexpr s32 kLightningStrikeFrames[kLightningStrikeCount] = {
    36, 52, 68, 84, 100};
constexpr f32 kLightningStrikeOffsets[kLightningStrikeCount] = {
    -35.0f, 30.0f, 0.0f, -20.0f, 40.0f};

void AddLightningStrike(MoveAnimation& a, u32 index) {
  const s32 frame = kLightningStrikeFrames[index];
  const f32 offset = kLightningStrikeOffsets[index];
  const bool is_even = index % 2 == 0;
  const u16 bolt_group = is_even ? 5 : 7;
  const u16 impact_group = is_even ? 8 : 9;
  const u16 sound_group = is_even ? 3 : 4;

  a.Sound3dPlay(frame, frame, 852035, -1, 85, 0, -40 + 25 * (s32)(index % 3),
                false, sound_group);
  a.Sound3dGlideToPokemon(frame, frame, AnimationTarget::kDefender,
                          BodyPoint::kOrigin, Vec3(0.0f, 300.0f, 0.0f),
                          PositionAdjust::kNormal, sound_group);

  a.EffectSpawn(frame, frame + 26, EffectId::kThunder0, false,
                  EffectDrawOrder::kStandard, bolt_group);
  a.EffectGlideToPokemon(frame, frame, AnimationTarget::kDefender,
                           BodyPoint::kOrigin, Vec3(offset, 300.0f, 0.0f),
                           PositionAdjust::kNoSizeAdjust, bolt_group);
  a.EffectResize(frame, frame, Vec3(10.0f, 10.0f, 10.0f), bolt_group);

  a.EffectSpawn(frame + 4, frame + 30, EffectId::kThunder4, false,
                  EffectDrawOrder::kStandard, impact_group);
  a.EffectGlideToPokemon(frame + 4, frame + 4, AnimationTarget::kDefender,
                           BodyPoint::kFront, Vec3(offset * 0.5f, 0.0f, 0.0f),
                           PositionAdjust::kNormal, impact_group);
  a.EffectResize(frame + 4, frame + 4, Vec3(4.0f, 4.0f, 4.0f), impact_group);

  a.BackdropColor(frame, frame + 2, Vec3(0.8f, 0.9f, 1.0f), 1.0f);
  a.BackdropColor(frame + 2, frame + 9, Vec3(0.4f, 0.5f, 0.9f), 0.6f);
  a.PokemonTint(frame + 4, frame + 9, AnimationTarget::kDefender,
                Vec3(0.5f, 0.8f, 1.0f), 1.0f, Vec3(0.5f, 0.8f, 1.0f), 0.0f);
  a.CameraRumble(frame + 4, frame + 14, 12.0f, 2.0f, 6.0f, Axis::kY);
}

}

void BuildAbsoluteZeroAnimation(MoveAnimation& a) {
  a.CameraRestore(0, 0, false, MoveCurve::kLinear, false);
  a.Sound3dSetup(0, 0, 0, 1.0f, 11.3f, 200.0f, 200.0f, 300.0f);
  a.HealthBarShowAll(0, 0, false);

  a.BackdropColorToggle(0, 0, true, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColor(0, 25, Vec3(0.4f, 0.5f, 0.9f), 0.9f);

  a.Sound3dPlay(1, 1, 852276, -1, 90, 0, 127, false, 1);
  a.Sound3dGlideToPokemon(1, 1, AnimationTarget::kAttacker, BodyPoint::kOrigin,
                          Vec3(0.0f, 0.0f, 0.0f), PositionAdjust::kNormal, 1);
  a.EffectSpawn(1, 60, EffectId::kSheerColdGlaciate1, false,
                  EffectDrawOrder::kStandard, 1);
  a.EffectGlideToPokemon(1, 1, AnimationTarget::kAttacker, BodyPoint::kOrigin,
                           Vec3(0.0f, 0.0f, 0.0f), PositionAdjust::kNormal, 1);
  a.EffectResize(1, 1, Vec3(2.5f, 2.5f, 2.5f), 1);
  a.PokemonPlayAttackPose(20, 20, AnimationTarget::kAttacker,
                          PokemonPose::kRangedAttack);

  a.CameraRumble(25, 130, 50.0f, 0.0f, 8.0f, Axis::kX);
  a.CameraRumble(25, 130, 50.0f, 0.0f, 8.0f, Axis::kY);
  a.EffectSpawn(25, 135, EffectId::kHail, false, EffectDrawOrder::kStandard,
                  2);
  a.EffectTeleport(25, 25, Vec3(0.0f, 0.0f, 0.0f), true, 2);
  a.EffectResize(25, 25, Vec3(30.0f, 30.0f, 30.0f), 2);

  a.EffectSpawn(45, 140, EffectId::kBlizzardTriAttackPowderSnow, true,
                  EffectDrawOrder::kStandard, 3);
  a.EffectGlideToPokemon(45, 45, AnimationTarget::kDefender,
                           BodyPoint::kOrigin, Vec3(0.0f, 0.0f, 0.0f),
                           PositionAdjust::kNormal, 3);

  a.CameraGlideToPokemon(30, 40, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(100.0f, 25.0f, 340.0f), Vec3(0.0f, 40.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kLinear,
                         false);

  for (u32 i = 0; i < kLightningStrikeCount; i++) AddLightningStrike(a, i);

  a.Sound3dPlay(55, 55, 852295, 589825, 104, 0, 0, false, 2);
  a.Sound3dGlideToPokemon(55, 55, AnimationTarget::kDefender,
                          BodyPoint::kOrigin, Vec3(0.0f, 0.0f, 0.0f),
                          PositionAdjust::kNormal, 2);
  a.EffectSpawn(55, 110, EffectId::kSheerCold1, false,
                  EffectDrawOrder::kStandard, 4);
  a.EffectGlideToPokemon(55, 55, AnimationTarget::kDefender,
                           BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f),
                           PositionAdjust::kNormal, 4);
  a.EffectResize(55, 55, Vec3(10.0f, 10.0f, 10.0f), 4);
  a.PokemonPlayPose(58, 58, AnimationTarget::kDefender, PokemonPose::kHurt);

  a.Sound3dPlay(112, 112, 852035, -1, 100, 0, -20, false, 5);
  a.Sound3dGlideToPokemon(112, 112, AnimationTarget::kDefender,
                          BodyPoint::kOrigin, Vec3(0.0f, 300.0f, 0.0f),
                          PositionAdjust::kNormal, 5);
  a.EffectSpawn(112, 152, EffectId::kThunder6, false,
                  EffectDrawOrder::kStandard, 6);
  a.EffectGlideToPokemon(112, 112, AnimationTarget::kDefender,
                           BodyPoint::kOrigin, Vec3(0.0f, 300.0f, 0.0f),
                           PositionAdjust::kNoSizeAdjust, 6);
  a.EffectResize(112, 112, Vec3(3.0f, 3.0f, 3.0f), 6);

  a.Sound3dPlay(118, 118, 852349, 589824, 100, 0, -60, false, 2);
  a.Sound3dGlideToPokemon(118, 118, AnimationTarget::kDefender,
                          BodyPoint::kOrigin, Vec3(0.0f, 0.0f, 0.0f),
                          PositionAdjust::kNormal, 2);
  a.EffectSpawn(118, 150, EffectId::kThunder5, false,
                  EffectDrawOrder::kStandard);
  a.EffectGlideToPokemon(118, 118, AnimationTarget::kDefender,
                           BodyPoint::kOrigin, Vec3(0.0f, 10.0f, 0.0f),
                           PositionAdjust::kNormal);
  a.EffectResize(118, 118, Vec3(2.0f, 2.0f, 2.0f));
  a.EffectResize(119, 150, Vec3(6.0f, 6.0f, 6.0f));
  a.BackdropColor(118, 121, Vec3(1.0f, 1.0f, 1.0f), 1.0f);
  a.PokemonPlayPose(120, 120, AnimationTarget::kDefender, PokemonPose::kHurt);
  a.PokemonTint(120, 146, AnimationTarget::kDefender, Vec3(0.5f, 0.8f, 1.0f),
                1.0f, Vec3(0.5f, 0.8f, 1.0f), 0.0f);
  a.CameraRumble(118, 138, 25.0f, 5.0f, 5.0f, Axis::kY);
  a.CameraRestore(132, 147, false, MoveCurve::kFastStart, false);

  a.BackdropColor(138, 153, Vec3(1.0f, 1.0f, 1.0f), 0.0f);
  a.BackdropColorToggle(158, 158, false, Vec3(0.0f, 0.0f, 0.0f), 0.0f);

  a.RecolorEffect(EffectId::kSheerCold1, Color8(230, 250, 255, 255),
                    Color8(120, 200, 255, 255), Color8(40, 90, 220, 255));
  a.RecolorEffect(EffectId::kThunder0, Color8(210, 235, 255, 255),
                    Color8(70, 150, 255, 255), Color8(30, 60, 200, 255));
  a.RecolorEffect(EffectId::kThunder4, Color8(210, 235, 255, 255),
                    Color8(70, 150, 255, 255), Color8(30, 60, 200, 255));
  a.RecolorEffect(EffectId::kThunder5, Color8(210, 235, 255, 255),
                    Color8(70, 150, 255, 255), Color8(30, 60, 200, 255));
  a.RecolorEffect(EffectId::kThunder6, Color8(210, 235, 255, 255),
                    Color8(70, 150, 255, 255), Color8(30, 60, 200, 255));
}

void RegisterAbsoluteZeroAnimation() {
  MoveAnimations::Define(kMoveAbsoluteZero, BuildAbsoluteZeroAnimation);
}

}
