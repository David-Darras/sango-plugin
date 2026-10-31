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
 * @file type_moves_animation.cc
 * @brief The animations of the four moves of the new types: Prism Lance
 * (Light), Sonic Requiem (Sound), Ion Storm (Plasma) and Dreamfall (Dream).
 *
 * Each animation re-colours the effects of the game, so the four moves do not
 * look like the moves they borrow their effects from.
 */

#include "battle/patch/move_animation.h"
#include "custom_battle.h"

namespace battle {
namespace {

// The sounds of the game that the animations use.
constexpr s32 kSoundBolt = 852035;
constexpr s32 kSoundCharge = 852276;
constexpr s32 kSoundImpact = 852295;
constexpr s32 kSoundBlast = 852349;

// The library gives 10 groups to the effects and 6 groups to the sounds. The
// particles of an effect live after its last frame, so a group must never get a
// new effect while the old one can still be alive: the game frees the old
// object and the draw list keeps it. Each effect gets a group that no effect
// used yet. The animations have fewer than 10 effects.
constexpr u32 kEffectGroupCount = 10;
constexpr u32 kSoundGroupCount = 6;
u32 next_effect_group;
u32 next_sound_group;

u16 AcquireEffectGroup() {
  const u32 group = next_effect_group;
  if (next_effect_group + 1 < kEffectGroupCount) next_effect_group++;
  return static_cast<u16>(group);
}

// The steps that all the animations start with: no camera, the sound space,
// and the health bars of both sides.
void Begin(MoveAnimation& a) {
  next_effect_group = 0;
  next_sound_group = 0;
  a.CameraRestore(0, 0, false, MoveCurve::kLinear, false);
  a.Sound3dSetup(0, 0, 0, 1.0f, 11.3f, 200.0f, 200.0f, 300.0f);
  a.HealthBarShowAll(0, 0, false);
}

// Plays a sound at a Pokémon.
void Sound(MoveAnimation& a, s32 frame, s32 sound, s32 volume, s32 pitch,
           AnimationTarget who) {
  const u16 group = static_cast<u16>(next_sound_group++ % kSoundGroupCount);
  a.Sound3dPlay(frame, frame, sound, -1, volume, 0, pitch, false, group);
  a.Sound3dGlideToPokemon(frame, frame, who, BodyPoint::kOrigin,
                          Vec3(0.0f, 0.0f, 0.0f), PositionAdjust::kNormal,
                          group);
}

// Puts an effect on a Pokémon for some frames, with a size. Returns its group.
u16 Effect(MoveAnimation& a, s32 start, s32 end, EffectId id,
           AnimationTarget who, BodyPoint point, const Vec3& offset,
           f32 size) {
  const u16 group = AcquireEffectGroup();
  a.EffectSpawn(start, end, id, false, EffectDrawOrder::kStandard, group);
  a.EffectGlideToPokemon(start, start, who, point, offset,
                         PositionAdjust::kNormal, group);
  a.EffectResize(start, start, Vec3(size, size, size), group);
  return group;
}

void Recolour(MoveAnimation& a, EffectId id, Color8 light, Color8 middle,
              Color8 dark) {
  a.RecolorEffect(id, light, middle, dark);
}

// Each animation is small, like the animations of the game: two effects of the
// game, four or five spawns, and a few camera, backdrop and sound steps.

// --- Prism Lance (Light) --------------------------------------------------
// The user gathers white light, then two flashes of light hit the target.
void BuildPrismLance(MoveAnimation& a) {
  Begin(a);
  a.BackdropColorToggle(0, 0, true, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColor(0, 20, Vec3(1.0f, 0.97f, 0.8f), 0.85f);
  a.CameraGlideToPokemon(0, 24, AnimationTarget::kAttacker, BodyPoint::kCenter,
                         Vec3(60.0f, 30.0f, 260.0f), Vec3(0.0f, 20.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kEaseInOut,
                         false);

  Sound(a, 1, kSoundCharge, 100, 40, AnimationTarget::kAttacker);
  const u16 charge = Effect(a, 1, 30, EffectId::kLusterPurge1,
                            AnimationTarget::kAttacker, BodyPoint::kCenter,
                            Vec3(0.0f, 0.0f, 0.0f), 2.0f);
  a.EffectResize(2, 24, Vec3(5.0f, 5.0f, 5.0f), charge);
  a.PokemonPlayAttackPose(18, 18, AnimationTarget::kAttacker,
                          PokemonPose::kRangedAttack);
  a.CameraRumble(4, 24, 6.0f, 1.0f, 2.0f, Axis::kY);

  a.CameraGlideToPokemon(26, 36, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(-90.0f, 25.0f, 320.0f), Vec3(0.0f, 30.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kFastStart,
                         false);
  Sound(a, 30, kSoundBolt, 100, 60, AnimationTarget::kDefender);
  Effect(a, 30, 56, EffectId::kThunder4, AnimationTarget::kDefender,
         BodyPoint::kCenter, Vec3(0.0f, 20.0f, 0.0f), 5.0f);
  Sound(a, 40, kSoundBlast, 110, -20, AnimationTarget::kDefender);
  Effect(a, 40, 66, EffectId::kThunder4, AnimationTarget::kDefender,
         BodyPoint::kCenter, Vec3(0.0f, -20.0f, 0.0f), 8.0f);
  a.PokemonPlayPose(32, 32, AnimationTarget::kDefender, PokemonPose::kHurt);
  a.PokemonTint(32, 46, AnimationTarget::kDefender, Vec3(1.0f, 1.0f, 0.85f),
                1.0f, Vec3(1.0f, 1.0f, 0.85f), 0.0f);
  a.BackdropColor(30, 33, Vec3(1.0f, 1.0f, 1.0f), 1.0f);
  a.BackdropColor(33, 60, Vec3(1.0f, 0.9f, 0.5f), 0.0f);
  a.CameraRumble(30, 60, 30.0f, 4.0f, 6.0f, Axis::kX);
  a.HealthBarApplyDamage(42, 42, AnimationTarget::kDefender, true);
  a.CameraRestore(56, 72, false, MoveCurve::kFastStart, false);
  a.BackdropColorToggle(76, 76, false, Vec3(0.0f, 0.0f, 0.0f), 0.0f);

  Recolour(a, EffectId::kLusterPurge1, Color8(255, 255, 230, 255),
           Color8(255, 235, 120, 255), Color8(255, 190, 60, 255));
  Recolour(a, EffectId::kThunder4, Color8(255, 255, 255, 255),
           Color8(255, 240, 150, 255), Color8(255, 200, 80, 255));
}

// --- Sonic Requiem (Sound) ------------------------------------------------
// Two rings of sound grow around the user, then a wave hits the target.
void BuildSonicRequiem(MoveAnimation& a) {
  Begin(a);
  a.BackdropColorToggle(0, 0, true, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColor(0, 16, Vec3(0.1f, 0.35f, 0.55f), 0.8f);
  a.CameraGlideToPokemon(0, 20, AnimationTarget::kAttacker, BodyPoint::kFront,
                         Vec3(-40.0f, 20.0f, 240.0f), Vec3(0.0f, 20.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kEaseInOut,
                         false);
  a.PokemonPlayAttackPose(6, 6, AnimationTarget::kAttacker,
                          PokemonPose::kAppeal);

  for (u32 i = 0; i < 2; i++) {
    const s32 frame = 4 + 16 * static_cast<s32>(i);
    Sound(a, frame, kSoundCharge, 100, -60 + 40 * static_cast<s32>(i),
          AnimationTarget::kAttacker);
    const u16 ring = Effect(a, frame, frame + 26, EffectId::kUproar1,
                            AnimationTarget::kAttacker, BodyPoint::kFront,
                            Vec3(0.0f, 0.0f, 0.0f), 1.5f);
    a.EffectResize(frame + 1, frame + 24, Vec3(8.0f, 8.0f, 8.0f), ring);
    a.CameraRumble(frame, frame + 12, 10.0f + 6.0f * i, 2.0f, 5.0f, Axis::kX);
  }

  a.CameraGlideToPokemon(36, 48, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(120.0f, 20.0f, 300.0f), Vec3(0.0f, 30.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kFastStart,
                         false);
  Sound(a, 44, kSoundBlast, 110, -80, AnimationTarget::kDefender);
  Effect(a, 44, 76, EffectId::kMirrorShot1, AnimationTarget::kDefender,
         BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f), 8.0f);
  a.PokemonPlayPose(46, 46, AnimationTarget::kDefender, PokemonPose::kHurtHeavy);
  a.PokemonTint(46, 60, AnimationTarget::kDefender, Vec3(0.4f, 0.8f, 1.0f),
                0.8f, Vec3(0.4f, 0.8f, 1.0f), 0.0f);
  a.CameraRumble(44, 76, 40.0f, 0.0f, 8.0f, Axis::kX);
  a.BackdropColor(44, 47, Vec3(1.0f, 1.0f, 1.0f), 1.0f);
  a.BackdropColor(47, 76, Vec3(0.1f, 0.35f, 0.55f), 0.0f);
  a.HealthBarApplyDamage(50, 50, AnimationTarget::kDefender, true);
  a.CameraRestore(70, 86, false, MoveCurve::kEaseInOut, false);
  a.BackdropColorToggle(90, 90, false, Vec3(0.0f, 0.0f, 0.0f), 0.0f);

  Recolour(a, EffectId::kUproar1, Color8(220, 255, 255, 255),
           Color8(80, 210, 240, 255), Color8(20, 100, 200, 255));
  Recolour(a, EffectId::kMirrorShot1, Color8(255, 255, 255, 255),
           Color8(130, 230, 255, 255), Color8(30, 110, 220, 255));
}

// --- Ion Storm (Plasma) ---------------------------------------------------
// The user makes a ball of plasma. The ball hits the target and three arcs of
// plasma follow.
void BuildIonStorm(MoveAnimation& a) {
  Begin(a);
  a.BackdropColorToggle(0, 0, true, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColor(0, 20, Vec3(0.25f, 0.0f, 0.35f), 0.9f);
  a.CameraGlideToPokemon(0, 22, AnimationTarget::kAttacker, BodyPoint::kCenter,
                         Vec3(80.0f, 30.0f, 280.0f), Vec3(0.0f, 20.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kEaseInOut,
                         false);

  Sound(a, 1, kSoundCharge, 100, 20, AnimationTarget::kAttacker);
  const u16 ball = Effect(a, 1, 30, EffectId::kThunder6,
                          AnimationTarget::kAttacker, BodyPoint::kShootPoint,
                          Vec3(0.0f, 20.0f, 0.0f), 1.0f);
  a.EffectResize(2, 28, Vec3(5.0f, 5.0f, 5.0f), ball);
  a.PokemonPlayAttackPose(20, 20, AnimationTarget::kAttacker,
                          PokemonPose::kRangedAttack);
  a.CameraRumble(2, 30, 8.0f, 2.0f, 3.0f, Axis::kY);

  a.CameraGlideToPokemon(30, 42, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(-110.0f, 30.0f, 330.0f), Vec3(0.0f, 30.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kLinear,
                         false);
  Sound(a, 32, kSoundBolt, 105, 0, AnimationTarget::kAttacker);
  Sound(a, 44, kSoundBlast, 110, -40, AnimationTarget::kDefender);
  Effect(a, 44, 76, EffectId::kThunder5, AnimationTarget::kDefender,
         BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f), 9.0f);
  a.PokemonPlayPose(46, 46, AnimationTarget::kDefender, PokemonPose::kHurtHeavy);
  a.BackdropColor(44, 47, Vec3(1.0f, 0.8f, 1.0f), 1.0f);
  a.BackdropColor(47, 90, Vec3(0.25f, 0.0f, 0.35f), 0.9f);
  a.CameraRumble(44, 90, 40.0f, 0.0f, 8.0f, Axis::kY);
  a.HealthBarApplyDamage(50, 50, AnimationTarget::kDefender, true);

  constexpr f32 kOffsets[3] = {-40.0f, 30.0f, -10.0f};
  for (u32 i = 0; i < 3; i++) {
    const s32 frame = 52 + 10 * static_cast<s32>(i);
    Sound(a, frame, kSoundBolt, 90, -30 + 20 * static_cast<s32>(i),
          AnimationTarget::kDefender);
    Effect(a, frame, frame + 18, EffectId::kThunder5,
           AnimationTarget::kDefender, BodyPoint::kFront,
           Vec3(kOffsets[i], 0.0f, 0.0f), 5.0f);
    a.PokemonTint(frame, frame + 6, AnimationTarget::kDefender,
                  Vec3(1.0f, 0.4f, 1.0f), 1.0f, Vec3(1.0f, 0.4f, 1.0f), 0.0f);
  }

  a.CameraRestore(88, 104, false, MoveCurve::kFastStart, false);
  a.BackdropColor(92, 106, Vec3(0.25f, 0.0f, 0.35f), 0.0f);
  a.BackdropColorToggle(110, 110, false, Vec3(0.0f, 0.0f, 0.0f), 0.0f);

  Recolour(a, EffectId::kThunder6, Color8(255, 220, 255, 255),
           Color8(240, 80, 240, 255), Color8(120, 20, 200, 255));
  Recolour(a, EffectId::kThunder5, Color8(255, 240, 255, 255),
           Color8(255, 140, 230, 255), Color8(170, 40, 220, 255));
}

// --- Dreamfall (Dream) ----------------------------------------------------
// The world fades to a dusk of pink and violet. A spiral of dream dust wraps
// the target, and a glimmer goes back to the user.
void BuildDreamfall(MoveAnimation& a) {
  Begin(a);
  a.BackdropColorToggle(0, 0, true, Vec3(0.0f, 0.0f, 0.0f), 0.0f);
  a.BackdropColor(0, 30, Vec3(0.12f, 0.05f, 0.3f), 0.92f);
  a.CameraGlideToPokemon(0, 30, AnimationTarget::kDefender, BodyPoint::kCenter,
                         Vec3(-40.0f, 40.0f, 360.0f), Vec3(0.0f, 30.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kEaseInOut,
                         false);

  Sound(a, 1, kSoundCharge, 90, -100, AnimationTarget::kAttacker);
  a.PokemonPlayAttackPose(10, 10, AnimationTarget::kAttacker,
                          PokemonPose::kAppeal);
  Effect(a, 4, 40, EffectId::kSheerColdGlaciate1, AnimationTarget::kAttacker,
         BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f), 2.0f);

  Sound(a, 26, kSoundImpact, 80, -60, AnimationTarget::kDefender);
  Effect(a, 26, 66, EffectId::kSheerColdGlaciate1, AnimationTarget::kDefender,
         BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f), 3.0f);
  a.BackdropColor(26, 30, Vec3(0.9f, 0.5f, 1.0f), 0.95f);
  a.BackdropColor(30, 40, Vec3(0.12f, 0.05f, 0.3f), 0.92f);
  // The target gets violet, then its colour comes back.
  a.PokemonTint(26, 44, AnimationTarget::kDefender, Vec3(0.7f, 0.4f, 1.0f),
                0.0f, Vec3(0.7f, 0.4f, 1.0f), 0.7f);
  a.PokemonTint(56, 70, AnimationTarget::kDefender, Vec3(0.7f, 0.4f, 1.0f),
                0.7f, Vec3(0.7f, 0.4f, 1.0f), 0.0f);
  a.PokemonPlayPose(36, 36, AnimationTarget::kDefender, PokemonPose::kHurtLight);
  a.CameraRumble(26, 66, 6.0f, 6.0f, 2.0f, Axis::kY);
  a.HealthBarApplyDamage(50, 50, AnimationTarget::kDefender, true);

  a.CameraGlideToPokemon(66, 84, AnimationTarget::kAttacker, BodyPoint::kCenter,
                         Vec3(70.0f, 30.0f, 280.0f), Vec3(0.0f, 20.0f, 0.0f),
                         0.0f, PositionAdjust::kNormal, MoveCurve::kEaseInOut,
                         false);
  Sound(a, 68, kSoundCharge, 90, 60, AnimationTarget::kAttacker);
  Effect(a, 68, 90, EffectId::kSilverWind1, AnimationTarget::kAttacker,
         BodyPoint::kCenter, Vec3(0.0f, 0.0f, 0.0f), 3.0f);
  a.PokemonTint(70, 86, AnimationTarget::kAttacker, Vec3(1.0f, 0.7f, 1.0f),
                0.8f, Vec3(1.0f, 0.7f, 1.0f), 0.0f);

  a.CameraRestore(88, 104, false, MoveCurve::kEaseInOut, false);
  a.BackdropColor(90, 104, Vec3(0.12f, 0.05f, 0.3f), 0.0f);
  a.BackdropColorToggle(108, 108, false, Vec3(0.0f, 0.0f, 0.0f), 0.0f);

  Recolour(a, EffectId::kSheerColdGlaciate1, Color8(255, 220, 255, 255),
           Color8(220, 120, 255, 255), Color8(110, 40, 210, 255));
  Recolour(a, EffectId::kSilverWind1, Color8(255, 245, 255, 255),
           Color8(255, 170, 240, 255), Color8(180, 100, 255, 255));
}

} // namespace

void RegisterTypeMovesAnimations() {
  MoveAnimations::Define(kMovePrismLance, BuildPrismLance);
  MoveAnimations::Define(kMoveSonicRequiem, BuildSonicRequiem);
  MoveAnimations::Define(kMoveIonStorm, BuildIonStorm);
  MoveAnimations::Define(kMoveDreamfall, BuildDreamfall);
}

} // namespace battle
