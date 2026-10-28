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
 * @file shiny_entry.cc
 * @brief A bigger star effect when a shiny Pokémon comes out of its Poke Ball.
 *
 * The declarations are in battle/patch/shiny_entry.h.
 */

#include "battle/patch/shiny_entry.h"

#include "battle/patch/move_animation.h"

namespace battle {

namespace {
// The animation file of the game that plays when a shiny Pokémon comes out.
constexpr u32 kShinyEntryFile = 3;
// The steps that restore the backdrop start at this frame. They move to the
// end of the longer animation.
constexpr s32 kClosingFrame = 43;
constexpr s32 kExtraFrames = 20;
// The bursts of the plugin use the effect groups 3 to 9. The game uses 1 and 2.
constexpr u16 kFirstGroup = 3;
constexpr u32 kBurstCount = 5;
constexpr s32 kBurstDelay = 4;
constexpr s32 kBurstLife = 37;
constexpr f32 kSpread = 10.0f;
// The flash goes through all the hues, one color for each 2 frames.
constexpr s32 kFlashFrame = 8;
constexpr s32 kFlashStep = 2;
constexpr u32 kFlashColorCount = 18;

// Converts a hue (0 to 1) into a bright color.
Vec3 HueToColor(f32 hue) {
  const f32 h = hue * 6.0f;
  const f32 rise = h - (f32)(s32)h;
  const f32 fall = 1.0f - rise;
  switch ((s32)h % 6) {
    case 0: return Vec3(1.0f, rise, 0.0f);
    case 1: return Vec3(fall, 1.0f, 0.0f);
    case 2: return Vec3(0.0f, 1.0f, rise);
    case 3: return Vec3(0.0f, fall, 1.0f);
    case 4: return Vec3(rise, 0.0f, 1.0f);
    default: return Vec3(1.0f, 0.0f, fall);
  }
}

void EditAnimation(MoveAnimation& animation) {
  if (!ShinyEntry::GetInstance().is_enabled) return;

  // The animation lasts longer, so that the last bursts can end.
  auto shift = [](AnimationStep& step) {
    if (step.start_frame() < kClosingFrame) return;
    step.SetStartFrame(step.start_frame() + kExtraFrames);
    step.SetEndFrame(step.end_frame() + kExtraFrames);
  };
  animation.ForEach(AnimationStepKind::kBackdropColor, shift);
  animation.ForEach(AnimationStepKind::kBackdropColorToggle, shift);
  animation.SetDuration(animation.last_frame() + kExtraFrames);
  const s32 last = animation.last_frame();

  // The bursts: one in each corner around the Pokémon, then above it.
  const f32 places[kBurstCount][2] = {
      {-1.0f, 1.0f}, {1.0f, 1.0f}, {-1.0f, -1.0f}, {1.0f, -1.0f}, {0.0f, 2.0f}};
  for (u32 i = 0; i < kBurstCount; i++) {
    const s32 start = 6 + (s32)i * kBurstDelay;
    const s32 end = start + kBurstLife < last ? start + kBurstLife : last;
    const u16 group = kFirstGroup + (u16)i;
    animation.EffectSpawn(start, end, EffectId::kBattleScene3, true,
                          EffectDrawOrder::kStandard, group,
                          StepCondition::kRareOnly);
    animation.EffectGlideToPokemonRotated(
        start, start, AnimationTarget::kDefender, BodyPoint::kCenter,
        Vec3(places[i][0] * kSpread, places[i][1] * kSpread, 0.0f), 0.0f,
        PositionAdjust::kNormal, group, StepCondition::kRareOnly);
    animation.EffectResize(start, start, Vec3(2.0f, 2.0f, 2.0f), group,
                           StepCondition::kRareOnly);
  }

  for (u32 i = 0; i < kFlashColorCount; i++) {
    const s32 start = kFlashFrame + (s32)i * kFlashStep;
    animation.BackdropColor(start, start + kFlashStep - 1,
                            HueToColor((f32)i / (f32)kFlashColorCount), 0.55f,
                            0, StepCondition::kRareOnly);
  }

  // Bigger stars, and fewer of them.
  animation.ScaleEffectSize(EffectId::kBattleScene3, 5.0f);
  animation.ScaleEffectDensity(EffectId::kBattleScene3, 0.6f);
}
} // namespace

void ShinyEntry::Initialize() {
  MoveAnimations::EditFile(kShinyEntryFile, EditAnimation);
}

void ShinyEntry::Refresh() {
  MoveAnimations::Invalidate();
}
} // namespace battle
