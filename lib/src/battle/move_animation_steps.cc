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

AnimationStep MoveAnimation::PokemonTeleport(s32 start_frame, s32 end_frame, AnimationTarget who, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonTeleport, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, position);
  step.SetS32(4, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonShift(s32 start_frame, s32 end_frame, AnimationTarget who, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonShift, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, offset);
  return step;
}

AnimationStep MoveAnimation::PokemonGlideToPokemon(s32 start_frame, s32 end_frame, AnimationTarget who, AnimationTarget destination, BodyPoint point, const Vec3& offset, PositionAdjust adjust, bool rotate, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(destination));
  step.SetS32(2, static_cast<s32>(point));
  step.SetVec3(3, offset);
  step.SetS32(6, static_cast<s32>(adjust));
  step.SetS32(7, rotate ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonResetPosition(s32 start_frame, s32 end_frame, AnimationTarget who, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonResetPosition, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

AnimationStep MoveAnimation::PokemonGlideToModel(s32 start_frame, s32 end_frame, AnimationTarget who, const Vec3& offset, s32 model_group, bool flip, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonGlideToModel, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, offset);
  step.SetS32(4, model_group);
  step.SetS32(5, flip ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonSpin(s32 start_frame, s32 end_frame, AnimationTarget who, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonSpin, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, degrees);
  step.SetS32(4, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonFacePokemon(s32 start_frame, s32 end_frame, AnimationTarget who, AnimationTarget toward, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonFacePokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(toward));
  step.SetS32(2, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::PokemonResize(s32 start_frame, s32 end_frame, AnimationTarget who, const Vec3& scale, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonResize, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, scale);
  return step;
}

AnimationStep MoveAnimation::PokemonShow(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonShow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonShowNonTargets(s32 start_frame, s32 end_frame, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonShowNonTargets, start_frame, end_frame, group, condition);
  step.SetS32(0, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonShowShadow(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonShowShadow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonPlayPose(s32 start_frame, s32 end_frame, AnimationTarget who, PokemonPose pose, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonPlayPose, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(pose));
  return step;
}

AnimationStep MoveAnimation::PokemonPlayAttackPose(s32 start_frame, s32 end_frame, AnimationTarget who, PokemonPose pose, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonPlayAttackPose, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(pose));
  return step;
}

AnimationStep MoveAnimation::PokemonPoseSpeed(s32 start_frame, s32 end_frame, AnimationTarget who, f32 speed, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonPoseSpeed, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetF32(1, speed);
  return step;
}

AnimationStep MoveAnimation::PokemonOutline(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonOutline, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonTint(s32 start_frame, s32 end_frame, AnimationTarget who, const Vec3& start_color, f32 start_strength, const Vec3& end_color, f32 end_strength, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonTint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, start_color);
  step.SetF32(4, start_strength);
  step.SetVec3(5, end_color);
  step.SetF32(8, end_strength);
  return step;
}

AnimationStep MoveAnimation::PokemonFollowModel(s32 start_frame, s32 end_frame, AnimationTarget who, s32 model_group, const char* bone_name, FollowRotation rotation, const Vec3& position_offset, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonFollowModel, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, model_group);
  step.SetText(2, bone_name, 16);
  step.SetS32(6, static_cast<s32>(rotation));
  step.SetVec3(7, position_offset);
  step.SetVec3(10, rotation_offset);
  return step;
}

AnimationStep MoveAnimation::PokemonStopShake(s32 start_frame, s32 end_frame, AnimationTarget who, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonStopShake, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

AnimationStep MoveAnimation::PokemonShake(s32 start_frame, s32 end_frame, AnimationTarget who, f32 start_amount, f32 end_amount, f32 start_decay, f32 end_decay, Axis axis, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonShake, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetF32(1, start_amount);
  step.SetF32(2, end_amount);
  step.SetF32(3, start_decay);
  step.SetF32(4, end_decay);
  step.SetS32(5, static_cast<s32>(axis));
  return step;
}

AnimationStep MoveAnimation::CameraJump(s32 start_frame, s32 end_frame, const Vec3& position, const Vec3& look_at, f32 field_of_view, MoveCurve curve, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraJump, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetVec3(3, look_at);
  step.SetF32(6, field_of_view);
  step.SetS32(7, static_cast<s32>(curve));
  step.SetS32(8, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraShift(s32 start_frame, s32 end_frame, const Vec3& position, const Vec3& look_at, f32 field_of_view, MoveCurve curve, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetVec3(3, look_at);
  step.SetF32(6, field_of_view);
  step.SetS32(7, static_cast<s32>(curve));
  step.SetS32(8, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraGlideToPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& camera_position, const Vec3& look_at, f32 field_of_view, PositionAdjust adjust, MoveCurve curve, bool auto_rotate, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, camera_position);
  step.SetVec3(5, look_at);
  step.SetF32(8, field_of_view);
  step.SetS32(9, static_cast<s32>(adjust));
  step.SetS32(10, static_cast<s32>(curve));
  step.SetS32(11, auto_rotate ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraLock(s32 start_frame, s32 end_frame, AnimationTarget target, MoveCurve curve, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraLock, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(curve));
  return step;
}

AnimationStep MoveAnimation::CameraOrbitPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, AnimationTarget direction_target, BodyPoint direction_point, const Vec3& camera_position, const Vec3& look_at, f32 field_of_view, PositionAdjust adjust, bool vertical, MoveCurve curve, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraOrbitPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(direction_target));
  step.SetS32(3, static_cast<s32>(direction_point));
  step.SetVec3(4, camera_position);
  step.SetVec3(7, look_at);
  step.SetF32(10, field_of_view);
  step.SetS32(11, static_cast<s32>(adjust));
  step.SetS32(12, vertical ? 1 : 0);
  step.SetS32(13, static_cast<s32>(curve));
  return step;
}

AnimationStep MoveAnimation::CameraRestore(s32 start_frame, s32 end_frame, bool flip, MoveCurve curve, bool skip_in_contest, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraRestore, start_frame, end_frame, group, condition);
  step.SetS32(0, flip ? 1 : 0);
  step.SetS32(1, static_cast<s32>(curve));
  step.SetS32(2, skip_in_contest ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraDepth(s32 start_frame, s32 end_frame, f32 depth, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraDepth, start_frame, end_frame, group, condition);
  step.SetF32(0, depth);
  return step;
}

AnimationStep MoveAnimation::CameraTurn(s32 start_frame, s32 end_frame, s32 degrees, s32 move_option, bool flip, AnimationTarget flip_target, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraTurn, start_frame, end_frame, group, condition);
  step.SetS32(0, degrees);
  step.SetS32(1, move_option);
  step.SetS32(2, flip ? 1 : 0);
  step.SetS32(3, static_cast<s32>(flip_target));
  return step;
}

AnimationStep MoveAnimation::CameraTurnToPokemon(s32 start_frame, s32 end_frame, AnimationTarget toward, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraTurnToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(toward));
  step.SetS32(1, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::CameraRumble(s32 start_frame, s32 end_frame, f32 start_amount, f32 end_amount, f32 decay, Axis axis, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraRumble, start_frame, end_frame, group, condition);
  step.SetF32(0, start_amount);
  step.SetF32(1, end_amount);
  step.SetF32(2, decay);
  step.SetS32(3, static_cast<s32>(axis));
  return step;
}

AnimationStep MoveAnimation::CameraClipOnPokemon(s32 start_frame, s32 end_frame, EffectModelId clip, AnimationTarget who, BodyPoint point, bool mirror, PositionAdjust adjust, bool keep_transform, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipOnPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(clip));
  step.SetS32(1, static_cast<s32>(who));
  step.SetS32(2, static_cast<s32>(point));
  step.SetS32(3, mirror ? 1 : 0);
  step.SetS32(4, static_cast<s32>(adjust));
  step.SetS32(5, keep_transform ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraClipOnPosition(s32 start_frame, s32 end_frame, EffectModelId clip, const Vec3& position, bool force_mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipOnPosition, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(clip));
  step.SetVec3(1, position);
  step.SetS32(4, force_mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraClipFollow(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& offset, bool mirror, s32 option, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipFollow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, mirror ? 1 : 0);
  step.SetS32(6, option);
  return step;
}

AnimationStep MoveAnimation::CameraClipEnd(s32 start_frame, s32 end_frame, bool hold_position, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipEnd, start_frame, end_frame, group, condition);
  step.SetS32(0, hold_position ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraClipPlanes(s32 start_frame, s32 end_frame, f32 near_plane, f32 far_plane, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipPlanes, start_frame, end_frame, group, condition);
  step.SetF32(0, near_plane);
  step.SetF32(1, far_plane);
  return step;
}

AnimationStep MoveAnimation::CameraClipPlanesScreen(s32 start_frame, s32 end_frame, f32 near_plane, f32 far_plane, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipPlanesScreen, start_frame, end_frame, group, condition);
  step.SetF32(0, near_plane);
  step.SetF32(1, far_plane);
  return step;
}

AnimationStep MoveAnimation::CameraScreenDepthEffect(s32 start_frame, s32 end_frame, bool enabled, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraScreenDepthEffect, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraFlatView(s32 start_frame, s32 end_frame, bool active, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraFlatView, start_frame, end_frame, group, condition);
  step.SetS32(0, active ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraClipRotate(s32 start_frame, s32 end_frame, s32 degrees_y, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipRotate, start_frame, end_frame, group, condition);
  step.SetS32(0, degrees_y);
  return step;
}

AnimationStep MoveAnimation::CameraClipScale(s32 start_frame, s32 end_frame, const Vec3& scale, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipScale, start_frame, end_frame, group, condition);
  step.SetVec3(0, scale);
  return step;
}

AnimationStep MoveAnimation::CameraClipMirror(s32 start_frame, s32 end_frame, bool mirror_x, bool mirror_up_vector, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipMirror, start_frame, end_frame, group, condition);
  step.SetS32(0, mirror_x ? 1 : 0);
  step.SetS32(1, mirror_up_vector ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectSpawn(s32 start_frame, s32 end_frame, EffectId effect, bool per_target, EffectDrawOrder layer, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectSpawn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(effect));
  step.SetS32(1, per_target ? 1 : 0);
  step.SetS32(2, static_cast<s32>(layer));
  return step;
}

AnimationStep MoveAnimation::EffectStopEmitting(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectStopEmitting, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::EffectTeleport(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectTeleport, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectShift(s32 start_frame, s32 end_frame, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, offset);
  return step;
}

AnimationStep MoveAnimation::EffectGlideToPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::EffectGlideToPokemonRotated(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& offset, f32 rotation_offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectGlideToPokemonRotated, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetF32(5, rotation_offset);
  step.SetS32(6, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::EffectGlideToFieldPoint(s32 start_frame, s32 end_frame, FieldPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectGlideToFieldPoint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(point));
  step.SetVec3(1, offset);
  return step;
}

AnimationStep MoveAnimation::EffectOrbitPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, AnimationTarget direction_target, BodyPoint direction_point, const Vec3& offset, PositionAdjust adjust, bool vertical, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectOrbitPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(direction_target));
  step.SetS32(3, static_cast<s32>(direction_point));
  step.SetVec3(4, offset);
  step.SetS32(7, static_cast<s32>(adjust));
  step.SetS32(8, vertical ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectGlideToModel(s32 start_frame, s32 end_frame, const Vec3& offset, s32 model_group, bool flip, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectGlideToModel, start_frame, end_frame, group, condition);
  step.SetVec3(0, offset);
  step.SetS32(3, model_group);
  step.SetS32(4, flip ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectResize(s32 start_frame, s32 end_frame, const Vec3& scale, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectResize, start_frame, end_frame, group, condition);
  step.SetVec3(0, scale);
  return step;
}

AnimationStep MoveAnimation::EffectTurn(s32 start_frame, s32 end_frame, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectTurn, start_frame, end_frame, group, condition);
  step.SetVec3(0, degrees);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectTurnToPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, PositionAdjust adjust, bool vertical, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectTurnToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(adjust));
  step.SetS32(3, vertical ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectFollowCamera(s32 start_frame, s32 end_frame, f32 distance, s32 rotation_flags, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectFollowCamera, start_frame, end_frame, group, condition);
  step.SetF32(0, distance);
  step.SetS32(1, rotation_flags);
  step.SetVec3(2, rotation_offset);
  return step;
}

AnimationStep MoveAnimation::EffectFollowModel(s32 start_frame, s32 end_frame, s32 model_group, const char* bone_name, FollowRotation rotation, const Vec3& position_offset, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectFollowModel, start_frame, end_frame, group, condition);
  step.SetS32(0, model_group);
  step.SetText(1, bone_name, 16);
  step.SetS32(5, static_cast<s32>(rotation));
  step.SetVec3(6, position_offset);
  step.SetVec3(9, rotation_offset);
  return step;
}

AnimationStep MoveAnimation::EffectFollowPokemon(s32 start_frame, s32 end_frame, bool enabled, AnimationTarget target, BodyPoint point, const Vec3& position_offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectFollowPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  step.SetS32(1, static_cast<s32>(target));
  step.SetS32(2, static_cast<s32>(point));
  step.SetVec3(3, position_offset);
  return step;
}

AnimationStep MoveAnimation::EffectLockDisplay(s32 start_frame, s32 end_frame, f32 distance, bool change_depth, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectLockDisplay, start_frame, end_frame, group, condition);
  step.SetF32(0, distance);
  step.SetS32(1, change_depth ? 1 : 0);
  step.SetS32(2, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectAutoSpin(s32 start_frame, s32 end_frame, bool enabled, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectAutoSpin, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectStopShake(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectStopShake, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::EffectShake(s32 start_frame, s32 end_frame, f32 start_amount, f32 end_amount, f32 start_decay, f32 end_decay, Axis axis, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectShake, start_frame, end_frame, group, condition);
  step.SetF32(0, start_amount);
  step.SetF32(1, end_amount);
  step.SetF32(2, start_decay);
  step.SetF32(3, end_decay);
  step.SetS32(4, static_cast<s32>(axis));
  return step;
}

AnimationStep MoveAnimation::ModelSpawn(s32 start_frame, s32 end_frame, EffectModelId model, ModelDrawMode draw_mode, bool per_target, bool effect_lighting, bool outline, bool distortion, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelSpawn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(model));
  step.SetS32(1, static_cast<s32>(draw_mode));
  step.SetS32(2, per_target ? 1 : 0);
  step.SetS32(3, effect_lighting ? 1 : 0);
  step.SetS32(4, outline ? 1 : 0);
  step.SetS32(5, distortion ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelSpawnBall(s32 start_frame, s32 end_frame, s32 ball_index, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelSpawnBall, start_frame, end_frame, group, condition);
  step.SetS32(0, ball_index);
  return step;
}

AnimationStep MoveAnimation::ModelTeleport(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelTeleport, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelShift(s32 start_frame, s32 end_frame, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, offset);
  return step;
}

AnimationStep MoveAnimation::ModelGlideToPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::ModelGlideToPokemonRotated(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& offset, f32 rotation_offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelGlideToPokemonRotated, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetF32(5, rotation_offset);
  step.SetS32(6, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::ModelGlideToFieldPoint(s32 start_frame, s32 end_frame, FieldPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelGlideToFieldPoint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(point));
  step.SetVec3(1, offset);
  return step;
}

AnimationStep MoveAnimation::ModelOrbitPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, AnimationTarget direction_target, BodyPoint direction_point, const Vec3& offset, PositionAdjust adjust, bool vertical, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelOrbitPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(direction_target));
  step.SetS32(3, static_cast<s32>(direction_point));
  step.SetVec3(4, offset);
  step.SetS32(7, static_cast<s32>(adjust));
  step.SetS32(8, vertical ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelTurn(s32 start_frame, s32 end_frame, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelTurn, start_frame, end_frame, group, condition);
  step.SetVec3(0, degrees);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelTurnToPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, PositionAdjust adjust, bool vertical, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelTurnToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(adjust));
  step.SetS32(3, vertical ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelResize(s32 start_frame, s32 end_frame, const Vec3& scale, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelResize, start_frame, end_frame, group, condition);
  step.SetVec3(0, scale);
  return step;
}

AnimationStep MoveAnimation::ModelShow(s32 start_frame, s32 end_frame, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelShow, start_frame, end_frame, group, condition);
  step.SetS32(0, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelStickToCamera(s32 start_frame, s32 end_frame, f32 distance, s32 rotation_flags, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelStickToCamera, start_frame, end_frame, group, condition);
  step.SetF32(0, distance);
  step.SetS32(1, rotation_flags);
  step.SetVec3(2, rotation_offset);
  return step;
}

AnimationStep MoveAnimation::ModelStickToModel(s32 start_frame, s32 end_frame, s32 model_group, const char* bone_name, FollowRotation rotation, const Vec3& position_offset, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelStickToModel, start_frame, end_frame, group, condition);
  step.SetS32(0, model_group);
  step.SetText(1, bone_name, 16);
  step.SetS32(5, static_cast<s32>(rotation));
  step.SetVec3(6, position_offset);
  step.SetVec3(9, rotation_offset);
  return step;
}

AnimationStep MoveAnimation::ModelFollowPokemon(s32 start_frame, s32 end_frame, bool enabled, AnimationTarget target, BodyPoint point, const Vec3& position_offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelFollowPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  step.SetS32(1, static_cast<s32>(target));
  step.SetS32(2, static_cast<s32>(point));
  step.SetVec3(3, position_offset);
  return step;
}

AnimationStep MoveAnimation::ModelAutoSpin(s32 start_frame, s32 end_frame, bool enabled, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelAutoSpin, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelTint(s32 start_frame, s32 end_frame, const char* material_name, s32 constant_index, const Vec3& color, f32 alpha, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelTint, start_frame, end_frame, group, condition);
  step.SetText(0, material_name, 32);
  step.SetS32(8, constant_index);
  step.SetVec3(9, color);
  step.SetF32(12, alpha);
  return step;
}

AnimationStep MoveAnimation::ModelPlayMotion(s32 start_frame, s32 end_frame, EffectModelId motion, s32 motion_index, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelPlayMotion, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(motion));
  step.SetS32(1, motion_index);
  step.SetS32(2, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelSwitchMotion(s32 start_frame, s32 end_frame, s32 motion_index, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelSwitchMotion, start_frame, end_frame, group, condition);
  step.SetS32(0, motion_index);
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelPlayTextureAnimation(s32 start_frame, s32 end_frame, EffectModelId animation, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelPlayTextureAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(animation));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelPlayVisibilityAnimation(s32 start_frame, s32 end_frame, EffectModelId animation, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelPlayVisibilityAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(animation));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelPlayShaderAnimation(s32 start_frame, s32 end_frame, EffectModelId animation, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelPlayShaderAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(animation));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelMotionSpeed(s32 start_frame, s32 end_frame, f32 speed, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelMotionSpeed, start_frame, end_frame, group, condition);
  step.SetF32(0, speed);
  return step;
}

AnimationStep MoveAnimation::ModelLightDirection(s32 start_frame, s32 end_frame, const Vec3& direction, bool flip, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelLightDirection, start_frame, end_frame, group, condition);
  step.SetVec3(0, direction);
  step.SetS32(3, flip ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelBallLightDirection(s32 start_frame, s32 end_frame, bool flip, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelBallLightDirection, start_frame, end_frame, group, condition);
  step.SetS32(0, flip ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelStopShake(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelStopShake, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::ModelShake(s32 start_frame, s32 end_frame, f32 start_amount, f32 end_amount, f32 start_decay, f32 end_decay, Axis axis, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelShake, start_frame, end_frame, group, condition);
  step.SetF32(0, start_amount);
  step.SetF32(1, end_amount);
  step.SetF32(2, start_decay);
  step.SetF32(3, end_decay);
  step.SetS32(4, static_cast<s32>(axis));
  return step;
}

AnimationStep MoveAnimation::ModelLockDisplay(s32 start_frame, s32 end_frame, f32 distance, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelLockDisplay, start_frame, end_frame, group, condition);
  step.SetF32(0, distance);
  step.SetS32(1, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ModelReplaceTexture(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelReplaceTexture, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::SoundPlay(s32 start_frame, s32 end_frame, s32 sound_id, s32 player, s32 volume, s32 pan, SoundPanMode pan_mode, s32 pitch, s32 tempo, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSoundPlay, start_frame, end_frame, group, condition);
  step.SetS32(0, sound_id);
  step.SetS32(1, player);
  step.SetS32(2, volume);
  step.SetS32(3, pan);
  step.SetS32(4, static_cast<s32>(pan_mode));
  step.SetS32(5, pitch);
  step.SetS32(6, tempo);
  return step;
}

AnimationStep MoveAnimation::SoundVolume(s32 start_frame, s32 end_frame, s32 start_volume, s32 end_volume, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSoundVolume, start_frame, end_frame, group, condition);
  step.SetS32(0, start_volume);
  step.SetS32(1, end_volume);
  return step;
}

AnimationStep MoveAnimation::SoundPan(s32 start_frame, s32 end_frame, s32 start_pan, s32 end_pan, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSoundPan, start_frame, end_frame, group, condition);
  step.SetS32(0, start_pan);
  step.SetS32(1, end_pan);
  return step;
}

AnimationStep MoveAnimation::SoundPitch(s32 start_frame, s32 end_frame, s32 start_pitch, s32 end_pitch, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSoundPitch, start_frame, end_frame, group, condition);
  step.SetS32(0, start_pitch);
  step.SetS32(1, end_pitch);
  return step;
}

AnimationStep MoveAnimation::SoundTempo(s32 start_frame, s32 end_frame, s32 start_tempo, s32 end_tempo, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSoundTempo, start_frame, end_frame, group, condition);
  step.SetS32(0, start_tempo);
  step.SetS32(1, end_tempo);
  return step;
}

AnimationStep MoveAnimation::SoundPlayCry(s32 start_frame, s32 end_frame, AnimationTarget who, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSoundPlayCry, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

AnimationStep MoveAnimation::Sound3dSetup(s32 start_frame, s32 end_frame, s32 priority_reduction, f32 pan_range, f32 speed_of_sound, f32 inertia_size, f32 max_volume_distance, f32 unit_distance, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dSetup, start_frame, end_frame, group, condition);
  step.SetS32(0, priority_reduction);
  step.SetF32(1, pan_range);
  step.SetF32(2, speed_of_sound);
  step.SetF32(3, inertia_size);
  step.SetF32(4, max_volume_distance);
  step.SetF32(5, unit_distance);
  return step;
}

AnimationStep MoveAnimation::Sound3dPlay(s32 start_frame, s32 end_frame, s32 sound_id, s32 player, s32 volume, s32 pan, s32 pitch, bool debug, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dPlay, start_frame, end_frame, group, condition);
  step.SetS32(0, sound_id);
  step.SetS32(1, player);
  step.SetS32(2, volume);
  step.SetS32(3, pan);
  step.SetS32(4, pitch);
  step.SetS32(5, debug ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::Sound3dStop(s32 start_frame, s32 end_frame, s32 fade_frames, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dStop, start_frame, end_frame, group, condition);
  step.SetS32(0, fade_frames);
  return step;
}

AnimationStep MoveAnimation::Sound3dTeleport(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dTeleport, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::Sound3dShift(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::Sound3dGlideToPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::Sound3dFollowPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dFollowPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  return step;
}

AnimationStep MoveAnimation::Sound3dFollowObject(s32 start_frame, s32 end_frame, FollowSource source, s32 object_index, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dFollowObject, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(source));
  step.SetS32(1, object_index);
  step.SetVec3(2, offset);
  return step;
}

AnimationStep MoveAnimation::Sound3dGlideToFieldPoint(s32 start_frame, s32 end_frame, FieldPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dGlideToFieldPoint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(point));
  step.SetVec3(1, offset);
  return step;
}

AnimationStep MoveAnimation::Sound3dPlayCry(s32 start_frame, s32 end_frame, AnimationTarget who, s32 volume, s32 pitch, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dPlayCry, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, volume);
  step.SetS32(2, pitch);
  return step;
}

AnimationStep MoveAnimation::HealthBarShow(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kHealthBarShow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::HealthBarShowAll(s32 start_frame, s32 end_frame, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kHealthBarShowAll, start_frame, end_frame, group, condition);
  step.SetS32(0, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::HealthBarApplyDamage(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kHealthBarApplyDamage, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::HealthBarApplyDamageAll(s32 start_frame, s32 end_frame, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kHealthBarApplyDamageAll, start_frame, end_frame, group, condition);
  step.SetS32(0, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::KnockbackFlag(s32 start_frame, s32 end_frame, AnimationTarget who, bool use_default, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kKnockbackFlag, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, use_default ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::BreakObjectsFlag(s32 start_frame, s32 end_frame, AnimationTarget who, BreakKind kind, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kBreakObjectsFlag, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(kind));
  return step;
}

AnimationStep MoveAnimation::BackdropColorToggle(s32 start_frame, s32 end_frame, bool enabled, const Vec3& color, f32 alpha, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kBackdropColorToggle, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  step.SetVec3(1, color);
  step.SetF32(4, alpha);
  return step;
}

AnimationStep MoveAnimation::BackdropColor(s32 start_frame, s32 end_frame, const Vec3& color, f32 alpha, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kBackdropColor, start_frame, end_frame, group, condition);
  step.SetVec3(0, color);
  step.SetF32(3, alpha);
  return step;
}

AnimationStep MoveAnimation::BackdropShow(s32 start_frame, s32 end_frame, bool shown, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kBackdropShow, start_frame, end_frame, group, condition);
  step.SetS32(0, shown ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::ScreenFadeIn(s32 start_frame, s32 end_frame, FadeScreen screen, FadeColor color, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kScreenFadeIn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(screen));
  step.SetS32(1, static_cast<s32>(color));
  return step;
}

AnimationStep MoveAnimation::ScreenFadeOut(s32 start_frame, s32 end_frame, FadeScreen screen, FadeColor color, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kScreenFadeOut, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(screen));
  step.SetS32(1, static_cast<s32>(color));
  return step;
}

AnimationStep MoveAnimation::DebrisSpawn(s32 start_frame, s32 end_frame, EffectModelId model, ModelDrawMode draw_mode, s32 max_count, s32 interval, s32 count_per_emit, s32 rate, s32 life, ClusterShape shape, s32 axis, const Vec3& size, s32 start_degrees, s32 end_degrees, s32 length, bool per_target, bool effect_lighting, bool outline, bool auto_rotate, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisSpawn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(model));
  step.SetS32(1, static_cast<s32>(draw_mode));
  step.SetS32(2, max_count);
  step.SetS32(3, interval);
  step.SetS32(4, count_per_emit);
  step.SetS32(5, rate);
  step.SetS32(6, life);
  step.SetS32(7, static_cast<s32>(shape));
  step.SetS32(8, axis);
  step.SetVec3(9, size);
  step.SetS32(12, start_degrees);
  step.SetS32(13, end_degrees);
  step.SetS32(14, length);
  step.SetS32(15, per_target ? 1 : 0);
  step.SetS32(16, effect_lighting ? 1 : 0);
  step.SetS32(17, outline ? 1 : 0);
  step.SetS32(18, auto_rotate ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::DebrisTeleport(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisTeleport, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::DebrisShift(s32 start_frame, s32 end_frame, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, offset);
  return step;
}

AnimationStep MoveAnimation::DebrisGlideToPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::DebrisGlideToFieldPoint(s32 start_frame, s32 end_frame, FieldPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisGlideToFieldPoint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(point));
  step.SetVec3(1, offset);
  return step;
}

AnimationStep MoveAnimation::DebrisTurn(s32 start_frame, s32 end_frame, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisTurn, start_frame, end_frame, group, condition);
  step.SetVec3(0, degrees);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::DebrisResize(s32 start_frame, s32 end_frame, const Vec3& scale, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisResize, start_frame, end_frame, group, condition);
  step.SetVec3(0, scale);
  return step;
}

AnimationStep MoveAnimation::DebrisSpread(s32 start_frame, s32 end_frame, s32 shape, const Vec3& offset, f32 speed, f32 acceleration, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisSpread, start_frame, end_frame, group, condition);
  step.SetS32(0, shape);
  step.SetVec3(1, offset);
  step.SetF32(4, speed);
  step.SetF32(5, acceleration);
  return step;
}

AnimationStep MoveAnimation::DebrisVelocity(s32 start_frame, s32 end_frame, const Vec3& degrees, s32 random_degrees, f32 speed, f32 acceleration, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisVelocity, start_frame, end_frame, group, condition);
  step.SetVec3(0, degrees);
  step.SetS32(3, random_degrees);
  step.SetF32(4, speed);
  step.SetF32(5, acceleration);
  return step;
}

AnimationStep MoveAnimation::DebrisGravity(s32 start_frame, s32 end_frame, s32 mode, const Vec3& position, bool absolute, f32 power, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisGravity, start_frame, end_frame, group, condition);
  step.SetS32(0, mode);
  step.SetVec3(1, position);
  step.SetS32(4, absolute ? 1 : 0);
  step.SetF32(5, power);
  return step;
}

AnimationStep MoveAnimation::DebrisInitialSize(s32 start_frame, s32 end_frame, const Vec3& minimum, const Vec3& maximum, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisInitialSize, start_frame, end_frame, group, condition);
  step.SetVec3(0, minimum);
  step.SetVec3(3, maximum);
  return step;
}

AnimationStep MoveAnimation::DebrisInitialTurn(s32 start_frame, s32 end_frame, const Vec3& minimum, const Vec3& maximum, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisInitialTurn, start_frame, end_frame, group, condition);
  step.SetVec3(0, minimum);
  step.SetVec3(3, maximum);
  return step;
}

AnimationStep MoveAnimation::DebrisSizeCurve(s32 start_frame, s32 end_frame, const AnimationKey (&keys)[5], u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisSizeCurve, start_frame, end_frame, group, condition);
  for (u32 i = 0; i < 5; i++) {
    step.SetS32(0 + i * 4, keys[i].frame);
    step.SetVec3(0 + i * 4 + 1, keys[i].value);
  }
  return step;
}

AnimationStep MoveAnimation::DebrisTurnCurve(s32 start_frame, s32 end_frame, const AnimationKey (&keys)[5], u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisTurnCurve, start_frame, end_frame, group, condition);
  for (u32 i = 0; i < 5; i++) {
    step.SetS32(0 + i * 4, keys[i].frame);
    step.SetVec3(0 + i * 4 + 1, keys[i].value);
  }
  return step;
}

AnimationStep MoveAnimation::DebrisMaterialAnimation(s32 start_frame, s32 end_frame, EffectModelId animation, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisMaterialAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(animation));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::DebrisModelAnimation(s32 start_frame, s32 end_frame, EffectModelId animation, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisModelAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(animation));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::DebrisReflection(s32 start_frame, s32 end_frame, f32 position, bool absolute, ClusterAnchor plane, ReflectionMode mode, f32 brake, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebrisReflection, start_frame, end_frame, group, condition);
  step.SetF32(0, position);
  step.SetS32(1, absolute ? 1 : 0);
  step.SetS32(2, static_cast<s32>(plane));
  step.SetS32(3, static_cast<s32>(mode));
  step.SetF32(4, brake);
  return step;
}

AnimationStep MoveAnimation::ChainAttackSetup(s32 start_frame, s32 end_frame, const ChainWindow (&windows)[6], u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kChainAttackSetup, start_frame, end_frame, group, condition);
  for (u32 i = 0; i < 6; i++) {
    step.SetS32(0 + i * 2, windows[i].start_frame);
    step.SetS32(0 + i * 2 + 1, windows[i].end_frame);
  }
  return step;
}

AnimationStep MoveAnimation::MegaEvolution(s32 start_frame, s32 end_frame, AnimationTarget who, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kMegaEvolution, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

AnimationStep MoveAnimation::ChargeHide(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kChargeHide, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::SubstituteShow(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSubstituteShow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::AegislashFormChange(s32 start_frame, s32 end_frame, AnimationTarget who, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kAegislashFormChange, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

AnimationStep MoveAnimation::WeatherChange(s32 start_frame, s32 end_frame, WeatherKind weather, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kWeatherChange, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(weather));
  return step;
}

AnimationStep MoveAnimation::FieldEffectStart(s32 start_frame, s32 end_frame, FieldEffectStyle kind, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kFieldEffectStart, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(kind));
  return step;
}

AnimationStep MoveAnimation::FieldEffectEnd(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kFieldEffectEnd, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::PlatformControl(s32 start_frame, s32 end_frame, s32 mode, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPlatformControl, start_frame, end_frame, group, condition);
  step.SetS32(0, mode);
  return step;
}

AnimationStep MoveAnimation::TrainerTeleport(s32 start_frame, s32 end_frame, TrainerSlot trainer, const Vec3& position, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kTrainerTeleport, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetVec3(1, position);
  return step;
}

AnimationStep MoveAnimation::TrainerTurn(s32 start_frame, s32 end_frame, TrainerSlot trainer, const Vec3& degrees, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kTrainerTurn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetVec3(1, degrees);
  return step;
}

AnimationStep MoveAnimation::TrainerTurnToPokemon(s32 start_frame, s32 end_frame, TrainerSlot trainer, AnimationTarget target, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kTrainerTurnToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetS32(1, static_cast<s32>(target));
  return step;
}

AnimationStep MoveAnimation::TrainerShow(s32 start_frame, s32 end_frame, TrainerSlot trainer, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kTrainerShow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::TrainerPlayPose(s32 start_frame, s32 end_frame, TrainerSlot trainer, TrainerPose pose, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kTrainerPlayPose, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetS32(1, static_cast<s32>(pose));
  step.SetS32(2, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::TrainerPoseSpeed(s32 start_frame, s32 end_frame, TrainerSlot trainer, f32 speed, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kTrainerPoseSpeed, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetF32(1, speed);
  return step;
}

AnimationStep MoveAnimation::TrainerShowBallGauge(s32 start_frame, s32 end_frame, TrainerSlot trainer, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kTrainerShowBallGauge, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  return step;
}

AnimationStep MoveAnimation::TrainerTint(s32 start_frame, s32 end_frame, TrainerSlot trainer, f32 start_strength, f32 end_strength, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kTrainerTint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetF32(1, start_strength);
  step.SetF32(2, end_strength);
  return step;
}

AnimationStep MoveAnimation::OverlayCreate(s32 start_frame, s32 end_frame, OverlayLayoutId layout, s32 animation_count, OverlayPlane plane, s32 priority, bool mono, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayCreate, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(layout));
  step.SetS32(1, animation_count);
  step.SetS32(2, static_cast<s32>(plane));
  step.SetS32(3, priority);
  step.SetS32(4, mono ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::OverlayDelete(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayDelete, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::OverlayAnimationStart(s32 start_frame, s32 end_frame, s32 animation_index, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayAnimationStart, start_frame, end_frame, group, condition);
  step.SetS32(0, animation_index);
  return step;
}

AnimationStep MoveAnimation::OverlayAnimationStop(s32 start_frame, s32 end_frame, s32 animation_index, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayAnimationStop, start_frame, end_frame, group, condition);
  step.SetS32(0, animation_index);
  return step;
}

AnimationStep MoveAnimation::OverlayAnimationSpeed(s32 start_frame, s32 end_frame, s32 animation_index, f32 speed, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayAnimationSpeed, start_frame, end_frame, group, condition);
  step.SetS32(0, animation_index);
  step.SetF32(1, speed);
  return step;
}

AnimationStep MoveAnimation::OverlayAnimationFrame(s32 start_frame, s32 end_frame, s32 animation_index, f32 frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayAnimationFrame, start_frame, end_frame, group, condition);
  step.SetS32(0, animation_index);
  step.SetF32(1, frame);
  return step;
}

AnimationStep MoveAnimation::OverlayDarken(s32 start_frame, s32 end_frame, s32 red, s32 green, s32 blue, const char* pane_name, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayDarken, start_frame, end_frame, group, condition);
  step.SetS32(0, red);
  step.SetS32(1, green);
  step.SetS32(2, blue);
  step.SetText(3, pane_name, 16);
  return step;
}

AnimationStep MoveAnimation::OverlayBrighten(s32 start_frame, s32 end_frame, s32 red, s32 green, s32 blue, const char* pane_name, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayBrighten, start_frame, end_frame, group, condition);
  step.SetS32(0, red);
  step.SetS32(1, green);
  step.SetS32(2, blue);
  step.SetText(3, pane_name, 16);
  return step;
}

AnimationStep MoveAnimation::OverlayTrainerPicture(s32 start_frame, s32 end_frame, TrainerPicture picture, s32 index, const char* pane_name, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayTrainerPicture, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(picture));
  step.SetS32(1, index);
  step.SetText(2, pane_name, 16);
  return step;
}

AnimationStep MoveAnimation::OverlayText(s32 start_frame, s32 end_frame, const char* source_pane, const char* destination_pane, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayText, start_frame, end_frame, group, condition);
  step.SetText(0, source_pane, 24);
  step.SetText(6, destination_pane, 24);
  return step;
}

AnimationStep MoveAnimation::OverlayDepth(s32 start_frame, s32 end_frame, f32 depth, f32 parallax, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayDepth, start_frame, end_frame, group, condition);
  step.SetF32(0, depth);
  step.SetF32(1, parallax);
  return step;
}

AnimationStep MoveAnimation::EffectSpawnForContest(s32 start_frame, s32 end_frame, EffectId cool, EffectId beauty, EffectId cute, EffectId smart, EffectId tough, bool per_target, EffectDrawOrder layer, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectSpawnForContest, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(cool));
  step.SetS32(1, static_cast<s32>(beauty));
  step.SetS32(2, static_cast<s32>(cute));
  step.SetS32(3, static_cast<s32>(smart));
  step.SetS32(4, static_cast<s32>(tough));
  step.SetS32(5, per_target ? 1 : 0);
  step.SetS32(6, static_cast<s32>(layer));
  return step;
}

AnimationStep MoveAnimation::InputWindow(s32 start_frame, s32 end_frame, s32 index, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kInputWindow, start_frame, end_frame, group, condition);
  step.SetS32(0, index);
  return step;
}

AnimationStep MoveAnimation::PokemonTeleportQuad(s32 start_frame, s32 end_frame, ContestSlot who, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonTeleportQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, position);
  step.SetS32(4, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonTintQuad(s32 start_frame, s32 end_frame, ContestSlot who, const Vec3& start_color, f32 start_strength, const Vec3& end_color, f32 end_strength, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonTintQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, start_color);
  step.SetF32(4, start_strength);
  step.SetVec3(5, end_color);
  step.SetF32(8, end_strength);
  return step;
}

AnimationStep MoveAnimation::PokemonSpinQuad(s32 start_frame, s32 end_frame, ContestSlot who, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonSpinQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, degrees);
  step.SetS32(4, mirror ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraClipOnPokemonQuad(s32 start_frame, s32 end_frame, EffectModelId clip, ContestSlot who, BodyPoint point, bool mirror, PositionAdjust adjust, bool keep_transform, bool loop, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipOnPokemonQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(clip));
  step.SetS32(1, static_cast<s32>(who));
  step.SetS32(2, static_cast<s32>(point));
  step.SetS32(3, mirror ? 1 : 0);
  step.SetS32(4, static_cast<s32>(adjust));
  step.SetS32(5, keep_transform ? 1 : 0);
  step.SetS32(6, loop ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraGlideToPokemonQuad(s32 start_frame, s32 end_frame, ContestSlot target, BodyPoint point, const Vec3& camera_position, const Vec3& look_at, f32 field_of_view, PositionAdjust adjust, MoveCurve curve, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraGlideToPokemonQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, camera_position);
  step.SetVec3(5, look_at);
  step.SetF32(8, field_of_view);
  step.SetS32(9, static_cast<s32>(adjust));
  step.SetS32(10, static_cast<s32>(curve));
  return step;
}

AnimationStep MoveAnimation::EffectGlideToPokemonQuad(s32 start_frame, s32 end_frame, ContestSlot target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectGlideToPokemonQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::ModelGlideToPokemonQuad(s32 start_frame, s32 end_frame, ContestSlot target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelGlideToPokemonQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::PokemonHappyPose(s32 start_frame, s32 end_frame, ContestSlot who, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonHappyPose, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

AnimationStep MoveAnimation::OverlayTrainerPictureAndText(s32 start_frame, s32 end_frame, TrainerPicture picture, const char* picture_pane, const char* name_pane, const char* entry_pane, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayTrainerPictureAndText, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(picture));
  step.SetText(1, picture_pane, 16);
  step.SetText(5, name_pane, 16);
  step.SetText(9, entry_pane, 16);
  return step;
}

AnimationStep MoveAnimation::SoundPlayCryQuad(s32 start_frame, s32 end_frame, ContestSlot who, bool happy, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSoundPlayCryQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, happy ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonShowQuad(s32 start_frame, s32 end_frame, ContestSlot who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonShowQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::SubstituteVisibility(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSubstituteVisibility, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonSkyPose(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonSkyPose, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::OverlayTextContest(s32 start_frame, s32 end_frame, const char* source_pane, const char* destination_pane, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kOverlayTextContest, start_frame, end_frame, group, condition);
  step.SetText(0, source_pane, 24);
  step.SetText(6, destination_pane, 24);
  return step;
}

AnimationStep MoveAnimation::PokemonShadeEffect(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonShadeEffect, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::PokemonPointLight(s32 start_frame, s32 end_frame, AnimationTarget who, const Vec3& color, const Vec3& position, s32 power, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonPointLight, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, color);
  step.SetVec3(4, position);
  step.SetS32(7, power);
  return step;
}

AnimationStep MoveAnimation::PokemonGazeAt(s32 start_frame, s32 end_frame, AnimationTarget who, AnimationTarget toward, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonGazeAt, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(toward));
  step.SetS32(2, static_cast<s32>(point));
  step.SetVec3(3, offset);
  step.SetS32(6, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::MessageShow(s32 start_frame, s32 end_frame, s32 message_id, s32 option, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kMessageShow, start_frame, end_frame, group, condition);
  step.SetS32(0, message_id);
  step.SetS32(1, option);
  return step;
}

AnimationStep MoveAnimation::MessageDismiss(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kMessageDismiss, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::ModelRemove(s32 start_frame, s32 end_frame, bool distortion, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelRemove, start_frame, end_frame, group, condition);
  step.SetS32(0, distortion ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::EffectDelete(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectDelete, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::ModelResetSize(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelResetSize, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::ModelArc(s32 start_frame, s32 end_frame, f32 speed, f32 gravity, f32 end_height, bool relative, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kModelArc, start_frame, end_frame, group, condition);
  step.SetF32(0, speed);
  step.SetF32(1, gravity);
  step.SetF32(2, end_height);
  step.SetS32(3, relative ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::CameraClipScaleValue(s32 start_frame, s32 end_frame, f32 scale, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kCameraClipScaleValue, start_frame, end_frame, group, condition);
  step.SetF32(0, scale);
  return step;
}

AnimationStep MoveAnimation::WaitForAnimation(s32 start_frame, s32 end_frame, WaitKind kind, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kWaitForAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(kind));
  return step;
}

AnimationStep MoveAnimation::Sound3dTest(s32 start_frame, s32 end_frame, s32 sound_id, s32 player, f32 horizontal_pan, f32 vertical_pan, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kSound3dTest, start_frame, end_frame, group, condition);
  step.SetS32(0, sound_id);
  step.SetS32(1, player);
  step.SetF32(2, horizontal_pan);
  step.SetF32(3, vertical_pan);
  return step;
}

AnimationStep MoveAnimation::PlatformShow(s32 start_frame, s32 end_frame, bool shown, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPlatformShow, start_frame, end_frame, group, condition);
  step.SetS32(0, shown ? 1 : 0);
  return step;
}

AnimationStep MoveAnimation::DebugSwapPokemon(s32 start_frame, s32 end_frame, AnimationTarget target, s32 species, s32 form, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebugSwapPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, species);
  step.SetS32(2, form);
  return step;
}

AnimationStep MoveAnimation::DebugProbe(s32 start_frame, s32 end_frame, const char* first, const char* second, const char* padding, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kDebugProbe, start_frame, end_frame, group, condition);
  step.SetText(0, first, 32);
  step.SetText(8, second, 32);
  step.SetText(16, padding, 32);
  return step;
}

AnimationStep MoveAnimation::EffectGlideToPokemonLegacy(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& temporary_offset, const Vec3& effect_offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectGlideToPokemonLegacy, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, temporary_offset);
  step.SetVec3(5, effect_offset);
  step.SetS32(8, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::EffectGlideToPokemonRotatedLegacy(s32 start_frame, s32 end_frame, AnimationTarget target, BodyPoint point, const Vec3& temporary_offset, const Vec3& effect_offset, f32 rotation_offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kEffectGlideToPokemonRotatedLegacy, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, temporary_offset);
  step.SetVec3(5, effect_offset);
  step.SetF32(8, rotation_offset);
  step.SetS32(9, static_cast<s32>(adjust));
  return step;
}

AnimationStep MoveAnimation::PokemonPlayPoseDirect(s32 start_frame, s32 end_frame, AnimationTarget who, PokemonPose pose, s32 reset_wait, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonPlayPoseDirect, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(pose));
  step.SetS32(2, reset_wait);
  return step;
}

AnimationStep MoveAnimation::PokemonShowSpecial(s32 start_frame, s32 end_frame, AnimationTarget who, bool visible, f32 height, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonShowSpecial, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  step.SetF32(2, height);
  return step;
}

AnimationStep MoveAnimation::PokemonWildCry(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonWildCry, start_frame, end_frame, group, condition);
  return step;
}

AnimationStep MoveAnimation::PokemonResetAll(s32 start_frame, s32 end_frame, bool skip_in_contest, u16 group, StepCondition condition) {
  AnimationStep step = Add(AnimationStepKind::kPokemonResetAll, start_frame, end_frame, group, condition);
  step.SetS32(0, skip_in_contest ? 1 : 0);
  return step;
}

}
