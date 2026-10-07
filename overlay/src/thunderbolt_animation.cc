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

constexpr u16 kNoGroup = 0xFFFF;
constexpr s32 kGroupGap = 2;
constexpr s32 kSoundLength = 25;

class GroupPool {
public:
  explicit GroupPool(u32 count) : count_(count) {
    for (u32 i = 0; i < kMaxGroups; i++) busy_until_[i] = -100;
  }

  u16 Acquire(s32 start, s32 end) {
    for (u32 i = 0; i < count_; i++) {
      if (busy_until_[i] + kGroupGap <= start) {
        busy_until_[i] = end;
        return (u16)i;
      }
    }
    return kNoGroup;
  }

private:
  static constexpr u32 kMaxGroups = 10;

  s32 busy_until_[kMaxGroups];
  u32 count_;
};

struct Palette {
  EffectId effect;
  u8 start[3];
  u8 middle[3];
  u8 end[3];
};

constexpr Palette kPalettes[] = {
    {EffectId::kElectroBall, {255, 255, 255}, {120, 255, 255}, {0, 160, 255}},
    {EffectId::kSpark0, {200, 255, 255}, {0, 230, 255}, {0, 90, 200}},
    {EffectId::kParabolicCharge1, {255, 255, 200}, {255, 210, 40}, {255, 120, 0}},
    {EffectId::kThunderbolt, {255, 210, 210}, {255, 30, 30}, {150, 0, 50}},
    {EffectId::kThunderShock, {255, 255, 190}, {255, 235, 0}, {255, 130, 0}},
    {EffectId::kChargeBeam0, {255, 200, 240}, {255, 50, 180}, {170, 0, 110}},
    {EffectId::kBoltStrike1, {215, 225, 255}, {70, 100, 255}, {20, 20, 170}},
    {EffectId::kSpark1, {255, 235, 200}, {255, 140, 0}, {200, 60, 0}},
    {EffectId::kThunderWave0, {255, 255, 255}, {60, 255, 200}, {0, 140, 120}},
    {EffectId::kThunderboltZapCannonDischarge, {230, 255, 230}, {60, 255, 90}, {0, 140, 60}},
    {EffectId::kThunderShockTriAttack, {255, 220, 255}, {220, 60, 255}, {100, 0, 180}},
    {EffectId::kThunderWave1, {255, 255, 210}, {190, 255, 40}, {90, 170, 0}},
    {EffectId::kThunder1, {255, 240, 200}, {255, 150, 30}, {220, 60, 0}},
    {EffectId::kThunder2, {255, 255, 255}, {255, 245, 130}, {255, 150, 0}},
};

constexpr EffectId kBoltEffects[6] = {
    EffectId::kThunderbolt,   EffectId::kThunderShock,
    EffectId::kChargeBeam0,   EffectId::kBoltStrike1,
    EffectId::kSpark1,        EffectId::kThunderWave0};

constexpr EffectId kImpactEffects[5] = {
    EffectId::kThunderboltZapCannonDischarge,
    EffectId::kThunderShockTriAttack, EffectId::kThunderWave1,
    EffectId::kThunder1, EffectId::kSpark0};

constexpr f32 kFlashColors[6][3] = {
    {1.0f, 0.3f, 0.3f}, {1.0f, 0.9f, 0.2f}, {1.0f, 0.4f, 0.9f},
    {0.3f, 0.5f, 1.0f}, {1.0f, 0.6f, 0.1f}, {0.4f, 1.0f, 0.9f}};

constexpr u32 kVolleyCount = 9;
constexpr s32 kVolleyFirstFrame = 45;
constexpr s32 kVolleySpacing = 10;
constexpr f32 kVolleyTilts[kVolleyCount] = {
    0.0f, -22.0f, 22.0f, -40.0f, 40.0f, -12.0f, 12.0f, -30.0f, 30.0f};
constexpr f32 kVolleyRolls[kVolleyCount] = {
    0.0f, 15.0f, -15.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f};

constexpr u32 kImpactCount = 10;
constexpr s32 kImpactFirstFrame = 62;
constexpr s32 kImpactSpacing = 9;

Vec3 FlashColor(u32 index) {
  const u32 slot = index % 6;
  return Vec3(kFlashColors[slot][0], kFlashColors[slot][1],
              kFlashColors[slot][2]);
}

void PlaySound(MoveAnimation& a, GroupPool& sounds, s32 frame, s32 sound_id,
               s32 volume, s32 pitch, AnimationTarget target,
               const Vec3& offset) {
  const u16 group = sounds.Acquire(frame, frame + kSoundLength);
  if (group == kNoGroup) return;
  a.Sound3dPlay(frame, frame, sound_id, -1, volume, 0, pitch, false, group);
  a.Sound3dGlideToPokemon(frame, frame, target, BodyPoint::kOrigin, offset,
                          PositionAdjust::kNormal, group);
}

void LaunchBolt(MoveAnimation& a, GroupPool& effects, EffectId effect,
                s32 start, s32 life, const Vec3& tilt, f32 first_scale,
                f32 last_scale, const Vec3& spin) {
  const u16 group = effects.Acquire(start, start + life);
  if (group == kNoGroup) return;
  a.EffectSpawn(start, start + life, effect, false,
                  EffectDrawOrder::kStandard, group);
  a.EffectGlideToPokemon(start, start, AnimationTarget::kAttacker,
                           BodyPoint::kCenter, Vec3(0.0f, -20.0f, 0.0f),
                           PositionAdjust::kNormal, group);
  a.EffectTurn(start, start, tilt, false, group);
  a.EffectResize(start, start,
                   Vec3(first_scale, first_scale, first_scale), group);
  a.EffectResize(start, start + life,
                   Vec3(last_scale, last_scale, last_scale), group);
  a.EffectTurn(start, start + life, spin, false, group);
}

void LaunchImpact(MoveAnimation& a, GroupPool& effects, EffectId effect,
                  s32 start, s32 life, const Vec3& offset, f32 first_scale,
                  f32 last_scale, const Vec3& spin) {
  const u16 group = effects.Acquire(start, start + life);
  if (group == kNoGroup) return;
  a.EffectSpawn(start, start + life, effect, false,
                  EffectDrawOrder::kStandard, group);
  a.EffectGlideToPokemon(start, start, AnimationTarget::kDefender,
                           BodyPoint::kCenter, offset, PositionAdjust::kNormal,
                           group);
  a.EffectResize(start, start,
                   Vec3(first_scale, first_scale, first_scale), group);
  a.EffectResize(start, start + life,
                   Vec3(last_scale, last_scale, last_scale), group);
  a.EffectTurn(start, start + life, spin, false, group);
}

void AddCharge(MoveAnimation& a, GroupPool& effects, GroupPool& sounds) {
  PlaySound(a, sounds, 1, 852276, 100, 127, AnimationTarget::kAttacker,
            Vec3(0.0f, 0.0f, 0.0f));

  const u16 orb = effects.Acquire(2, 52);
  a.EffectSpawn(2, 52, EffectId::kElectroBall, false,
                  EffectDrawOrder::kStandard, orb);
  a.EffectGlideToPokemon(2, 2, AnimationTarget::kAttacker,
                           BodyPoint::kHeadTop, Vec3(0.0f, 35.0f, 0.0f),
                           PositionAdjust::kNormal, orb);
  a.EffectResize(2, 2, Vec3(0.3f, 0.3f, 0.3f), orb);
  a.EffectResize(2, 45, Vec3(3.0f, 3.0f, 3.0f), orb);
  a.EffectTurn(2, 52, Vec3(0.0f, 720.0f, 360.0f), false, orb);

  const u16 aura = effects.Acquire(3, 60);
  a.EffectSpawn(3, 60, EffectId::kSpark0, false, EffectDrawOrder::kStandard,
                  aura);
  a.EffectGlideToPokemon(3, 3, AnimationTarget::kAttacker,
                           BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f),
                           PositionAdjust::kNormal, aura);
  a.EffectResize(3, 3, Vec3(1.0f, 1.0f, 1.0f), aura);
  a.EffectResize(3, 40, Vec3(4.0f, 4.0f, 4.0f), aura);
  a.EffectTurn(3, 60, Vec3(0.0f, 0.0f, -540.0f), false, aura);

  const u16 halo = effects.Acquire(14, 60);
  a.EffectSpawn(14, 60, EffectId::kParabolicCharge1, false,
                  EffectDrawOrder::kStandard, halo);
  a.EffectGlideToPokemon(14, 14, AnimationTarget::kAttacker,
                           BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f),
                           PositionAdjust::kNormal, halo);
  a.EffectResize(14, 14, Vec3(2.0f, 2.0f, 2.0f), halo);
  a.EffectResize(14, 45, Vec3(10.0f, 10.0f, 10.0f), halo);

  a.PokemonShake(5, 38, AnimationTarget::kAttacker, 0.0f, 20.0f, 8.0f, 0.0f,
                 Axis::kY);
  a.PokemonPlayAttackPose(36, 36, AnimationTarget::kAttacker,
                          PokemonPose::kRangedAttack);
  a.CameraGlideToPokemon(2, 14, AnimationTarget::kAttacker, BodyPoint::kCenter,
                         Vec3(-80.0f, 25.0f, 250.0f), Vec3(0.0f, 15.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kEaseInOut,
                         false);
}

void AddBarrage(MoveAnimation& a, GroupPool& effects, GroupPool& sounds) {
  for (u32 i = 0; i < kVolleyCount; i++) {
    const s32 frame = kVolleyFirstFrame + kVolleySpacing * (s32)i;
    const EffectId effect = kBoltEffects[i % 6];
    const Vec3 tilt(0.0f, kVolleyTilts[i], kVolleyRolls[i]);
    const f32 spin_direction = i % 2 == 0 ? 1.0f : -1.0f;

    LaunchBolt(a, effects, effect, frame, 32, tilt, 1.0f,
               4.0f + 0.4f * (f32)(i % 4),
               Vec3(0.0f, 0.0f, 120.0f * spin_direction));
    PlaySound(a, sounds, frame, 852195, 90, -60 + 20 * (s32)(i % 5),
              AnimationTarget::kAttacker, Vec3(0.0f, 0.0f, 0.0f));

    a.BackdropColor(frame, frame + 2, FlashColor(i), 0.9f);
    a.BackdropColor(frame + 2, frame + 9, Vec3(0.05f, 0.05f, 0.25f), 0.6f);
  }

  a.CameraGlideToPokemon(40, 56, AnimationTarget::kDefender,
                         BodyPoint::kCenter, Vec3(120.0f, 10.0f, 330.0f),
                         Vec3(0.0f, 15.0f, 0.0f), 0.0f,
                         PositionAdjust::kNormal, MoveCurve::kLinear, false);
}

void AddImpactStorm(MoveAnimation& a, GroupPool& effects,
                    GroupPool& sounds) {
  for (u32 i = 0; i < kImpactCount; i++) {
    const s32 frame = kImpactFirstFrame + kImpactSpacing * (s32)i;
    const EffectId effect = kImpactEffects[i % 5];
    const Vec3 offset(25.0f * (f32)((s32)(i % 3) - 1), 10.0f * (f32)(i % 4),
                      15.0f * (i % 2 == 0 ? 1.0f : -1.0f));

    LaunchImpact(a, effects, effect, frame, 28, offset, 1.0f,
                 4.5f + 0.5f * (f32)(i % 3),
                 Vec3(0.0f, 360.0f + 90.0f * (f32)(i % 3), 180.0f));
    PlaySound(a, sounds, frame, 852037, 95, -40 + 25 * (s32)(i % 4),
              AnimationTarget::kDefender, Vec3(0.0f, 0.0f, 0.0f));

    const Vec3 color = FlashColor(i + 2);
    a.PokemonTint(frame, frame + 6, AnimationTarget::kDefender, color, 1.0f,
                  color, 0.0f);
    a.BackdropColor(frame, frame + 2, color, 0.8f);
    a.BackdropColor(frame + 2, frame + 8, Vec3(0.05f, 0.05f, 0.25f), 0.6f);
  }

  a.PokemonPlayPose(64, 64, AnimationTarget::kDefender, PokemonPose::kHurt);
  a.PokemonPlayPose(102, 102, AnimationTarget::kDefender, PokemonPose::kHurt);
  a.PokemonPlayPose(140, 140, AnimationTarget::kDefender, PokemonPose::kHurt);
  a.PokemonShake(62, 78, AnimationTarget::kDefender, 0.0f, 20.0f, 8.0f, 0.0f,
                 Axis::kY);
  a.PokemonShake(100, 116, AnimationTarget::kDefender, 0.0f, 20.0f, 8.0f, 0.0f,
                 Axis::kY);
  a.PokemonShake(138, 154, AnimationTarget::kDefender, 0.0f, 20.0f, 8.0f, 0.0f,
                 Axis::kY);

  a.CameraRumble(62, 100, 8.0f, 2.0f, 12.0f, Axis::kY);
  a.CameraRumble(100, 138, 12.0f, 4.0f, 10.0f, Axis::kX);
  a.CameraGlideToPokemon(70, 90, AnimationTarget::kDefender,
                         BodyPoint::kCenter, Vec3(-140.0f, 60.0f, 260.0f),
                         Vec3(0.0f, 20.0f, 0.0f), 0.0f,
                         PositionAdjust::kNormal, MoveCurve::kEaseInOut,
                         false);
  a.CameraGlideToPokemon(95, 118, AnimationTarget::kDefender,
                         BodyPoint::kCenter, Vec3(0.0f, 220.0f, 240.0f),
                         Vec3(0.0f, 10.0f, 0.0f), 0.0f,
                         PositionAdjust::kNormal, MoveCurve::kEaseInOut,
                         false);
  a.CameraGlideToPokemon(120, 142, AnimationTarget::kDefender,
                         BodyPoint::kCenter, Vec3(160.0f, 30.0f, 180.0f),
                         Vec3(0.0f, 20.0f, 0.0f), 0.0f,
                         PositionAdjust::kNormal, MoveCurve::kFastStart,
                         false);

  a.HealthBarApplyDamage(70, 70, AnimationTarget::kDefender, true);
}

void AddFinale(MoveAnimation& a, GroupPool& effects, GroupPool& sounds) {
  PlaySound(a, sounds, 150, 852349, 110, -60, AnimationTarget::kDefender,
            Vec3(0.0f, 0.0f, 0.0f));
  PlaySound(a, sounds, 152, 852035, 100, -30, AnimationTarget::kDefender,
            Vec3(0.0f, 300.0f, 0.0f));

  const u16 flash = effects.Acquire(150, 195);
  a.EffectSpawn(150, 195, EffectId::kThunder2, false,
                  EffectDrawOrder::kStandard, flash);
  a.EffectGlideToPokemon(150, 150, AnimationTarget::kDefender,
                           BodyPoint::kOrigin, Vec3(0.0f, 1.0f, 0.0f),
                           PositionAdjust::kNormal, flash);
  a.EffectResize(150, 150, Vec3(4.0f, 4.0f, 4.0f), flash);
  a.EffectResize(150, 185, Vec3(18.0f, 18.0f, 18.0f), flash);

  const u16 burst = effects.Acquire(152, 195);
  a.EffectSpawn(152, 195, EffectId::kThunder1, false,
                  EffectDrawOrder::kStandard, burst);
  a.EffectGlideToPokemon(152, 152, AnimationTarget::kDefender,
                           BodyPoint::kOrigin, Vec3(0.0f, 1.0f, 0.0f),
                           PositionAdjust::kNormal, burst);
  a.EffectResize(152, 152, Vec3(2.0f, 2.0f, 2.0f), burst);
  a.EffectResize(152, 190, Vec3(10.0f, 10.0f, 10.0f), burst);
  a.EffectTurn(152, 195, Vec3(0.0f, 720.0f, 0.0f), false, burst);

  for (u32 k = 0; k < 8; k++) {
    const s32 frame = 150 + 2 * (s32)k;
    a.BackdropColor(frame, frame + 1, FlashColor(k), 1.0f);
  }
  a.CameraRumble(150, 175, 30.0f, 5.0f, 5.0f, Axis::kY);
  a.PokemonPlayPose(152, 152, AnimationTarget::kDefender, PokemonPose::kHurt);
  a.PokemonTint(152, 185, AnimationTarget::kDefender, Vec3(1.0f, 0.9f, 0.3f),
                1.0f, Vec3(1.0f, 0.9f, 0.3f), 0.0f);
  a.CameraRestore(168, 190, false, MoveCurve::kFastStart, false);
  a.BackdropColor(172, 192, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColorToggle(196, 196, false, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
}

void ApplyPalettes(MoveAnimation& a) {
  for (u32 i = 0; i < sizeof(kPalettes) / sizeof(kPalettes[0]); i++) {
    const Palette& palette = kPalettes[i];
    a.RecolorEffect(palette.effect,
                      Color8(palette.start[0], palette.start[1],
                             palette.start[2], 255),
                      Color8(palette.middle[0], palette.middle[1],
                             palette.middle[2], 255),
                      Color8(palette.end[0], palette.end[1], palette.end[2],
                             255));
  }
}

}

void BuildThunderboltAnimation(MoveAnimation& a) {
  GroupPool effects(10);
  GroupPool sounds(6);

  a.CameraRestore(0, 0, false, MoveCurve::kLinear, false);
  a.Sound3dSetup(0, 0, 0, 1.0f, 11.3f, 200.0f, 200.0f, 300.0f);
  a.HealthBarShowAll(0, 0, false);
  a.HealthBarShow(0, 0, AnimationTarget::kDefender, true);
  a.BackdropColorToggle(0, 0, true, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColor(0, 20, Vec3(0.05f, 0.05f, 0.25f), 0.6f);

  AddCharge(a, effects, sounds);
  AddBarrage(a, effects, sounds);
  AddImpactStorm(a, effects, sounds);
  AddFinale(a, effects, sounds);
  ApplyPalettes(a);
}

void RegisterThunderboltAnimation() {
  MoveAnimations::Define(MoveId::kThunderbolt, BuildThunderboltAnimation);
}

}
