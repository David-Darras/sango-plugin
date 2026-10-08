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
 * @file effect_showcase_animation.cc
 * @brief Example: an animation that shows the effects of the game, 10 at a time.
 *
 * Use it to find an effect for your animations.
 */

#include "battle/constant/effect_id.h"
#include "battle/patch/move_animation.h"
#include "ui/log_application.h"

namespace battle {

namespace {

constexpr u32 kMaxEffectsPerUse = kMoveEffectCount;
constexpr u32 kEffectsPerBatch = 10;
constexpr s32 kFramesPerBatch = 30;
constexpr s32 kFramesAlive = 22;
constexpr f32 kSpacing = 30.0f;
constexpr s32 kFirstFrame = 20;
constexpr u32 kFirstEffect = 0;
constexpr bool kAdvanceEachUse = true;
constexpr f32 kEffectScale = 1.5f;

u32 next_effect = kFirstEffect;

}

void BuildEffectShowcaseAnimation(MoveAnimation& a) {
  u32 first = next_effect;
  if (first >= kMoveEffectCount) first = kFirstEffect;
  u32 count = kMoveEffectCount - first;
  if (count > kMaxEffectsPerUse) count = kMaxEffectsPerUse;
  if (kAdvanceEachUse) next_effect = first + count;

  ui::LogApplication::Print(u"Effect showcase: effects %d to %d", (s32)first,
                            (s32)(first + count - 1));

  MoveAnimations::LoadEffectsOnDemand();

  const u32 batches = (count + kEffectsPerBatch - 1) / kEffectsPerBatch;
  const s32 end = kFirstFrame + (s32)batches * kFramesPerBatch;

  a.CameraRestore(0, 0, false, MoveCurve::kLinear, false);
  a.HealthBarShowAll(0, 0, false);
  a.HealthBarShow(0, 0, AnimationTarget::kDefender, true);

  a.CameraGlideToPokemon(2, 14, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(100.0f, 5.0f, 320.0f), Vec3(0.0f, 15.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kLinear,
                         false);

  for (u32 i = 0; i < count; i++) {
    const u32 slot = i % kEffectsPerBatch;
    const s32 frame = kFirstFrame + (s32)(i / kEffectsPerBatch) * kFramesPerBatch;
    const u16 group = (u16)slot;
    const f32 x = ((f32)slot - (f32)(kEffectsPerBatch - 1) * 0.5f) * kSpacing;

    a.EffectSpawn(frame, frame + kFramesAlive, kMoveEffects[first + i],
                  false, EffectDrawOrder::kStandard, group);
    a.EffectGlideToPokemon(frame, frame, AnimationTarget::kDefender,
                           BodyPoint::kCenter, Vec3(x, 0.0f, 0.0f),
                           PositionAdjust::kNormal, group);
    a.EffectResize(frame, frame,
                   Vec3(kEffectScale, kEffectScale, kEffectScale), group);
  }

  a.CameraRestore(end, end + 20, false, MoveCurve::kFastStart, false);

  ui::LogApplication::Print(u"Effect showcase: %d bytes, %d frames",
                            (s32)a.size(), end + 20);
}

void RegisterEffectShowcaseAnimation() {
  MoveAnimations::Define(MoveId::kThunderbolt, BuildEffectShowcaseAnimation);
}

}
