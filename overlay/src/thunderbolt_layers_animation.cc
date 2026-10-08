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
 * @file thunderbolt_layers_animation.cc
 * @brief Example: an animation that colors each layer of an effect.
 */

#include "battle/patch/move_animation.h"

namespace battle {

namespace {

constexpr s32 kImpactStart = 15;
constexpr s32 kImpactDuration = 240;
constexpr s32 kImpactEnd = kImpactStart + kImpactDuration;

constexpr bool kShowFirstLayer = false;
constexpr bool kShowSecondLayer = false;
constexpr bool kShowThirdLayer = true;

void StyleImpactLayers(MoveAnimation& a) {
  const EffectId impact = EffectId::kThunderboltZapCannonDischarge;

  a.LoopEffect(impact);

  a.ColorEffect(impact, Color8(255, 0, 0, 255), EffectLayer::kFirst);
  a.ColorEffect(impact, Color8(0, 0, 255, 255), EffectLayer::kSecond);
  a.ColorEffect(impact, Color8(0, 255, 0, 255), EffectLayer::kThird);

  a.FadeEffect(impact, 0.0f, 1.0f, 1.0f);
  a.ScaleEffectDensity(impact, 2.0f, EffectLayer::kFirst);
  a.ScaleEffectLife(impact, 1.5f, EffectLayer::kFirst);

  if (!kShowFirstLayer) a.DisableEffect(impact, EffectLayer::kFirst);
  if (!kShowSecondLayer) a.DisableEffect(impact, EffectLayer::kSecond);
  if (!kShowThirdLayer) a.DisableEffect(impact, EffectLayer::kThird);
}

}

void BuildThunderboltLayersAnimation(MoveAnimation& a) {
  a.CameraRestore(0, 0, false, MoveCurve::kLinear, false);
  a.Sound3dSetup(0, 0, 0, 1.0f, 11.3f, 200.0f, 200.0f, 300.0f);
  a.HealthBarShowAll(0, 0, false);
  a.HealthBarShow(0, 0, AnimationTarget::kDefender, true);

  a.CameraGlideToPokemon(2, 14, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(100.0f, 5.0f, 320.0f), Vec3(0.0f, 15.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kLinear,
                         false);

  a.Sound3dPlay(kImpactStart, kImpactStart, 852037, 589827, 90, 0, 40, false,
                1);
  a.Sound3dGlideToPokemon(kImpactStart, kImpactStart,
                          AnimationTarget::kDefender, BodyPoint::kWaist,
                          Vec3(0.0f, 0.0f, 0.0f), PositionAdjust::kNormal, 1);

  a.EffectSpawn(kImpactStart, kImpactEnd,
                  EffectId::kThunderboltZapCannonDischarge, false,
                  EffectDrawOrder::kStandard, 0);
  a.EffectGlideToPokemon(kImpactStart, kImpactStart,
                           AnimationTarget::kDefender, BodyPoint::kCenter,
                           Vec3(0.0f, 0.0f, 0.0f), PositionAdjust::kNormal, 0);
  a.EffectResize(kImpactStart, kImpactStart, Vec3(2.5f, 2.5f, 2.5f), 0);

  a.PokemonPlayPose(kImpactStart + 5, kImpactStart + 5,
                    AnimationTarget::kDefender, PokemonPose::kHurt);
  a.HealthBarApplyDamage(kImpactStart + 20, kImpactStart + 20,
                         AnimationTarget::kDefender, true);

  a.CameraRestore(kImpactEnd - 20, kImpactEnd, false, MoveCurve::kFastStart,
                false);

  StyleImpactLayers(a);
}

void RegisterThunderboltLayersAnimation() {
  MoveAnimations::Define(MoveId::kThunderbolt, BuildThunderboltLayersAnimation);
}

}
