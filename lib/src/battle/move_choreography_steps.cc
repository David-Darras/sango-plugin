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
 * @file move_choreography_steps.cc
 * @brief The typed functions that add the steps of a move choreography.
 *
 * This file is generated from the move choreography data. Do not change it by hand.
 */

#include "battle/patch/move_choreography.h"

namespace battle {

ChoreographyStep MoveChoreography::PokemonTeleport(s32 start_frame, s32 end_frame, ChoreographyTarget who, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonTeleport, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, position);
  step.SetS32(4, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonShift(s32 start_frame, s32 end_frame, ChoreographyTarget who, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonShift, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, offset);
  return step;
}

ChoreographyStep MoveChoreography::PokemonGlideToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget who, ChoreographyTarget destination, BodyPoint point, const Vec3& offset, PositionAdjust adjust, bool rotate, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(destination));
  step.SetS32(2, static_cast<s32>(point));
  step.SetVec3(3, offset);
  step.SetS32(6, static_cast<s32>(adjust));
  step.SetS32(7, rotate ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonResetPosition(s32 start_frame, s32 end_frame, ChoreographyTarget who, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonResetPosition, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

ChoreographyStep MoveChoreography::PokemonGlideToModel(s32 start_frame, s32 end_frame, ChoreographyTarget who, const Vec3& offset, s32 model_group, bool flip, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonGlideToModel, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, offset);
  step.SetS32(4, model_group);
  step.SetS32(5, flip ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonSpin(s32 start_frame, s32 end_frame, ChoreographyTarget who, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonSpin, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, degrees);
  step.SetS32(4, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonFacePokemon(s32 start_frame, s32 end_frame, ChoreographyTarget who, ChoreographyTarget toward, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonFacePokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(toward));
  step.SetS32(2, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::PokemonResize(s32 start_frame, s32 end_frame, ChoreographyTarget who, const Vec3& scale, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonResize, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, scale);
  return step;
}

ChoreographyStep MoveChoreography::PokemonShow(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonShow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonShowNonTargets(s32 start_frame, s32 end_frame, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonShowNonTargets, start_frame, end_frame, group, condition);
  step.SetS32(0, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonShowShadow(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonShowShadow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonPlayPose(s32 start_frame, s32 end_frame, ChoreographyTarget who, PokemonPose pose, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonPlayPose, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(pose));
  return step;
}

ChoreographyStep MoveChoreography::PokemonPlayAttackPose(s32 start_frame, s32 end_frame, ChoreographyTarget who, PokemonPose pose, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonPlayAttackPose, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(pose));
  return step;
}

ChoreographyStep MoveChoreography::PokemonPoseSpeed(s32 start_frame, s32 end_frame, ChoreographyTarget who, f32 speed, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonPoseSpeed, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetF32(1, speed);
  return step;
}

ChoreographyStep MoveChoreography::PokemonOutline(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonOutline, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonTint(s32 start_frame, s32 end_frame, ChoreographyTarget who, const Vec3& start_color, f32 start_strength, const Vec3& end_color, f32 end_strength, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonTint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, start_color);
  step.SetF32(4, start_strength);
  step.SetVec3(5, end_color);
  step.SetF32(8, end_strength);
  return step;
}

ChoreographyStep MoveChoreography::PokemonFollowModel(s32 start_frame, s32 end_frame, ChoreographyTarget who, s32 model_group, const char* bone_name, FollowRotation rotation, const Vec3& position_offset, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonFollowModel, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, model_group);
  step.SetText(2, bone_name, 16);
  step.SetS32(6, static_cast<s32>(rotation));
  step.SetVec3(7, position_offset);
  step.SetVec3(10, rotation_offset);
  return step;
}

ChoreographyStep MoveChoreography::PokemonStopShake(s32 start_frame, s32 end_frame, ChoreographyTarget who, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonStopShake, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

ChoreographyStep MoveChoreography::PokemonShake(s32 start_frame, s32 end_frame, ChoreographyTarget who, f32 start_amount, f32 end_amount, f32 start_decay, f32 end_decay, Axis axis, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonShake, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetF32(1, start_amount);
  step.SetF32(2, end_amount);
  step.SetF32(3, start_decay);
  step.SetF32(4, end_decay);
  step.SetS32(5, static_cast<s32>(axis));
  return step;
}

ChoreographyStep MoveChoreography::CameraJump(s32 start_frame, s32 end_frame, const Vec3& position, const Vec3& look_at, f32 field_of_view, MoveCurve curve, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraJump, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetVec3(3, look_at);
  step.SetF32(6, field_of_view);
  step.SetS32(7, static_cast<s32>(curve));
  step.SetS32(8, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraShift(s32 start_frame, s32 end_frame, const Vec3& position, const Vec3& look_at, f32 field_of_view, MoveCurve curve, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetVec3(3, look_at);
  step.SetF32(6, field_of_view);
  step.SetS32(7, static_cast<s32>(curve));
  step.SetS32(8, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraGlideToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& camera_position, const Vec3& look_at, f32 field_of_view, PositionAdjust adjust, MoveCurve curve, bool auto_rotate, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraGlideToPokemon, start_frame, end_frame, group, condition);
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

ChoreographyStep MoveChoreography::CameraLock(s32 start_frame, s32 end_frame, ChoreographyTarget target, MoveCurve curve, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraLock, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(curve));
  return step;
}

ChoreographyStep MoveChoreography::CameraOrbitPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, ChoreographyTarget direction_target, BodyPoint direction_point, const Vec3& camera_position, const Vec3& look_at, f32 field_of_view, PositionAdjust adjust, bool vertical, MoveCurve curve, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraOrbitPokemon, start_frame, end_frame, group, condition);
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

ChoreographyStep MoveChoreography::CameraRestore(s32 start_frame, s32 end_frame, bool flip, MoveCurve curve, bool skip_in_contest, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraRestore, start_frame, end_frame, group, condition);
  step.SetS32(0, flip ? 1 : 0);
  step.SetS32(1, static_cast<s32>(curve));
  step.SetS32(2, skip_in_contest ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraDepth(s32 start_frame, s32 end_frame, f32 depth, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraDepth, start_frame, end_frame, group, condition);
  step.SetF32(0, depth);
  return step;
}

ChoreographyStep MoveChoreography::CameraTurn(s32 start_frame, s32 end_frame, s32 degrees, s32 move_option, bool flip, ChoreographyTarget flip_target, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraTurn, start_frame, end_frame, group, condition);
  step.SetS32(0, degrees);
  step.SetS32(1, move_option);
  step.SetS32(2, flip ? 1 : 0);
  step.SetS32(3, static_cast<s32>(flip_target));
  return step;
}

ChoreographyStep MoveChoreography::CameraTurnToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget toward, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraTurnToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(toward));
  step.SetS32(1, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::CameraRumble(s32 start_frame, s32 end_frame, f32 start_amount, f32 end_amount, f32 decay, Axis axis, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraRumble, start_frame, end_frame, group, condition);
  step.SetF32(0, start_amount);
  step.SetF32(1, end_amount);
  step.SetF32(2, decay);
  step.SetS32(3, static_cast<s32>(axis));
  return step;
}

ChoreographyStep MoveChoreography::CameraClipOnPokemon(s32 start_frame, s32 end_frame, EffectModelId clip, ChoreographyTarget who, BodyPoint point, bool mirror, PositionAdjust adjust, bool keep_transform, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipOnPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(clip));
  step.SetS32(1, static_cast<s32>(who));
  step.SetS32(2, static_cast<s32>(point));
  step.SetS32(3, mirror ? 1 : 0);
  step.SetS32(4, static_cast<s32>(adjust));
  step.SetS32(5, keep_transform ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipOnPosition(s32 start_frame, s32 end_frame, EffectModelId clip, const Vec3& position, bool force_mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipOnPosition, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(clip));
  step.SetVec3(1, position);
  step.SetS32(4, force_mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipFollow(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& offset, bool mirror, s32 option, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipFollow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, mirror ? 1 : 0);
  step.SetS32(6, option);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipEnd(s32 start_frame, s32 end_frame, bool hold_position, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipEnd, start_frame, end_frame, group, condition);
  step.SetS32(0, hold_position ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipPlanes(s32 start_frame, s32 end_frame, f32 near_plane, f32 far_plane, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipPlanes, start_frame, end_frame, group, condition);
  step.SetF32(0, near_plane);
  step.SetF32(1, far_plane);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipPlanesScreen(s32 start_frame, s32 end_frame, f32 near_plane, f32 far_plane, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipPlanesScreen, start_frame, end_frame, group, condition);
  step.SetF32(0, near_plane);
  step.SetF32(1, far_plane);
  return step;
}

ChoreographyStep MoveChoreography::CameraScreenDepthEffect(s32 start_frame, s32 end_frame, bool enabled, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraScreenDepthEffect, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraFlatView(s32 start_frame, s32 end_frame, bool active, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraFlatView, start_frame, end_frame, group, condition);
  step.SetS32(0, active ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipRotate(s32 start_frame, s32 end_frame, s32 degrees_y, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipRotate, start_frame, end_frame, group, condition);
  step.SetS32(0, degrees_y);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipScale(s32 start_frame, s32 end_frame, const Vec3& scale, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipScale, start_frame, end_frame, group, condition);
  step.SetVec3(0, scale);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipMirror(s32 start_frame, s32 end_frame, bool mirror_x, bool mirror_up_vector, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipMirror, start_frame, end_frame, group, condition);
  step.SetS32(0, mirror_x ? 1 : 0);
  step.SetS32(1, mirror_up_vector ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectSpawn(s32 start_frame, s32 end_frame, EffectId effect, bool per_target, EffectDrawOrder layer, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectSpawn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(effect));
  step.SetS32(1, per_target ? 1 : 0);
  step.SetS32(2, static_cast<s32>(layer));
  return step;
}

ChoreographyStep MoveChoreography::EffectStopEmitting(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectStopEmitting, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::EffectTeleport(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectTeleport, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectShift(s32 start_frame, s32 end_frame, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, offset);
  return step;
}

ChoreographyStep MoveChoreography::EffectGlideToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::EffectGlideToPokemonRotated(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& offset, f32 rotation_offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectGlideToPokemonRotated, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetF32(5, rotation_offset);
  step.SetS32(6, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::EffectGlideToFieldPoint(s32 start_frame, s32 end_frame, FieldPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectGlideToFieldPoint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(point));
  step.SetVec3(1, offset);
  return step;
}

ChoreographyStep MoveChoreography::EffectOrbitPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, ChoreographyTarget direction_target, BodyPoint direction_point, const Vec3& offset, PositionAdjust adjust, bool vertical, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectOrbitPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(direction_target));
  step.SetS32(3, static_cast<s32>(direction_point));
  step.SetVec3(4, offset);
  step.SetS32(7, static_cast<s32>(adjust));
  step.SetS32(8, vertical ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectGlideToModel(s32 start_frame, s32 end_frame, const Vec3& offset, s32 model_group, bool flip, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectGlideToModel, start_frame, end_frame, group, condition);
  step.SetVec3(0, offset);
  step.SetS32(3, model_group);
  step.SetS32(4, flip ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectResize(s32 start_frame, s32 end_frame, const Vec3& scale, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectResize, start_frame, end_frame, group, condition);
  step.SetVec3(0, scale);
  return step;
}

ChoreographyStep MoveChoreography::EffectTurn(s32 start_frame, s32 end_frame, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectTurn, start_frame, end_frame, group, condition);
  step.SetVec3(0, degrees);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectTurnToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, PositionAdjust adjust, bool vertical, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectTurnToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(adjust));
  step.SetS32(3, vertical ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectFollowCamera(s32 start_frame, s32 end_frame, f32 distance, s32 rotation_flags, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectFollowCamera, start_frame, end_frame, group, condition);
  step.SetF32(0, distance);
  step.SetS32(1, rotation_flags);
  step.SetVec3(2, rotation_offset);
  return step;
}

ChoreographyStep MoveChoreography::EffectFollowModel(s32 start_frame, s32 end_frame, s32 model_group, const char* bone_name, FollowRotation rotation, const Vec3& position_offset, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectFollowModel, start_frame, end_frame, group, condition);
  step.SetS32(0, model_group);
  step.SetText(1, bone_name, 16);
  step.SetS32(5, static_cast<s32>(rotation));
  step.SetVec3(6, position_offset);
  step.SetVec3(9, rotation_offset);
  return step;
}

ChoreographyStep MoveChoreography::EffectFollowPokemon(s32 start_frame, s32 end_frame, bool enabled, ChoreographyTarget target, BodyPoint point, const Vec3& position_offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectFollowPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  step.SetS32(1, static_cast<s32>(target));
  step.SetS32(2, static_cast<s32>(point));
  step.SetVec3(3, position_offset);
  return step;
}

ChoreographyStep MoveChoreography::EffectLockDisplay(s32 start_frame, s32 end_frame, f32 distance, bool change_depth, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectLockDisplay, start_frame, end_frame, group, condition);
  step.SetF32(0, distance);
  step.SetS32(1, change_depth ? 1 : 0);
  step.SetS32(2, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectAutoSpin(s32 start_frame, s32 end_frame, bool enabled, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectAutoSpin, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectStopShake(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectStopShake, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::EffectShake(s32 start_frame, s32 end_frame, f32 start_amount, f32 end_amount, f32 start_decay, f32 end_decay, Axis axis, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectShake, start_frame, end_frame, group, condition);
  step.SetF32(0, start_amount);
  step.SetF32(1, end_amount);
  step.SetF32(2, start_decay);
  step.SetF32(3, end_decay);
  step.SetS32(4, static_cast<s32>(axis));
  return step;
}

ChoreographyStep MoveChoreography::ModelSpawn(s32 start_frame, s32 end_frame, EffectModelId model, ModelDrawMode draw_mode, bool per_target, bool effect_lighting, bool outline, bool distortion, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelSpawn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(model));
  step.SetS32(1, static_cast<s32>(draw_mode));
  step.SetS32(2, per_target ? 1 : 0);
  step.SetS32(3, effect_lighting ? 1 : 0);
  step.SetS32(4, outline ? 1 : 0);
  step.SetS32(5, distortion ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelSpawnBall(s32 start_frame, s32 end_frame, s32 ball_index, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelSpawnBall, start_frame, end_frame, group, condition);
  step.SetS32(0, ball_index);
  return step;
}

ChoreographyStep MoveChoreography::ModelTeleport(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelTeleport, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelShift(s32 start_frame, s32 end_frame, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, offset);
  return step;
}

ChoreographyStep MoveChoreography::ModelGlideToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::ModelGlideToPokemonRotated(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& offset, f32 rotation_offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelGlideToPokemonRotated, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetF32(5, rotation_offset);
  step.SetS32(6, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::ModelGlideToFieldPoint(s32 start_frame, s32 end_frame, FieldPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelGlideToFieldPoint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(point));
  step.SetVec3(1, offset);
  return step;
}

ChoreographyStep MoveChoreography::ModelOrbitPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, ChoreographyTarget direction_target, BodyPoint direction_point, const Vec3& offset, PositionAdjust adjust, bool vertical, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelOrbitPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(direction_target));
  step.SetS32(3, static_cast<s32>(direction_point));
  step.SetVec3(4, offset);
  step.SetS32(7, static_cast<s32>(adjust));
  step.SetS32(8, vertical ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelTurn(s32 start_frame, s32 end_frame, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelTurn, start_frame, end_frame, group, condition);
  step.SetVec3(0, degrees);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelTurnToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, PositionAdjust adjust, bool vertical, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelTurnToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetS32(2, static_cast<s32>(adjust));
  step.SetS32(3, vertical ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelResize(s32 start_frame, s32 end_frame, const Vec3& scale, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelResize, start_frame, end_frame, group, condition);
  step.SetVec3(0, scale);
  return step;
}

ChoreographyStep MoveChoreography::ModelShow(s32 start_frame, s32 end_frame, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelShow, start_frame, end_frame, group, condition);
  step.SetS32(0, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelStickToCamera(s32 start_frame, s32 end_frame, f32 distance, s32 rotation_flags, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelStickToCamera, start_frame, end_frame, group, condition);
  step.SetF32(0, distance);
  step.SetS32(1, rotation_flags);
  step.SetVec3(2, rotation_offset);
  return step;
}

ChoreographyStep MoveChoreography::ModelStickToModel(s32 start_frame, s32 end_frame, s32 model_group, const char* bone_name, FollowRotation rotation, const Vec3& position_offset, const Vec3& rotation_offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelStickToModel, start_frame, end_frame, group, condition);
  step.SetS32(0, model_group);
  step.SetText(1, bone_name, 16);
  step.SetS32(5, static_cast<s32>(rotation));
  step.SetVec3(6, position_offset);
  step.SetVec3(9, rotation_offset);
  return step;
}

ChoreographyStep MoveChoreography::ModelFollowPokemon(s32 start_frame, s32 end_frame, bool enabled, ChoreographyTarget target, BodyPoint point, const Vec3& position_offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelFollowPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  step.SetS32(1, static_cast<s32>(target));
  step.SetS32(2, static_cast<s32>(point));
  step.SetVec3(3, position_offset);
  return step;
}

ChoreographyStep MoveChoreography::ModelAutoSpin(s32 start_frame, s32 end_frame, bool enabled, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelAutoSpin, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelTint(s32 start_frame, s32 end_frame, const char* material_name, s32 constant_index, const Vec3& color, f32 alpha, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelTint, start_frame, end_frame, group, condition);
  step.SetText(0, material_name, 32);
  step.SetS32(8, constant_index);
  step.SetVec3(9, color);
  step.SetF32(12, alpha);
  return step;
}

ChoreographyStep MoveChoreography::ModelPlayMotion(s32 start_frame, s32 end_frame, EffectModelId motion, s32 motion_index, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelPlayMotion, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(motion));
  step.SetS32(1, motion_index);
  step.SetS32(2, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelSwitchMotion(s32 start_frame, s32 end_frame, s32 motion_index, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelSwitchMotion, start_frame, end_frame, group, condition);
  step.SetS32(0, motion_index);
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelPlayTextureAnimation(s32 start_frame, s32 end_frame, EffectModelId choreography, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelPlayTextureAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(choreography));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelPlayVisibilityAnimation(s32 start_frame, s32 end_frame, EffectModelId choreography, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelPlayVisibilityAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(choreography));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelPlayShaderAnimation(s32 start_frame, s32 end_frame, EffectModelId choreography, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelPlayShaderAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(choreography));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelMotionSpeed(s32 start_frame, s32 end_frame, f32 speed, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelMotionSpeed, start_frame, end_frame, group, condition);
  step.SetF32(0, speed);
  return step;
}

ChoreographyStep MoveChoreography::ModelLightDirection(s32 start_frame, s32 end_frame, const Vec3& direction, bool flip, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelLightDirection, start_frame, end_frame, group, condition);
  step.SetVec3(0, direction);
  step.SetS32(3, flip ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelBallLightDirection(s32 start_frame, s32 end_frame, bool flip, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelBallLightDirection, start_frame, end_frame, group, condition);
  step.SetS32(0, flip ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelStopShake(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelStopShake, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::ModelShake(s32 start_frame, s32 end_frame, f32 start_amount, f32 end_amount, f32 start_decay, f32 end_decay, Axis axis, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelShake, start_frame, end_frame, group, condition);
  step.SetF32(0, start_amount);
  step.SetF32(1, end_amount);
  step.SetF32(2, start_decay);
  step.SetF32(3, end_decay);
  step.SetS32(4, static_cast<s32>(axis));
  return step;
}

ChoreographyStep MoveChoreography::ModelLockDisplay(s32 start_frame, s32 end_frame, f32 distance, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelLockDisplay, start_frame, end_frame, group, condition);
  step.SetF32(0, distance);
  step.SetS32(1, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ModelReplaceTexture(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelReplaceTexture, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::SoundPlay(s32 start_frame, s32 end_frame, s32 sound_id, s32 player, s32 volume, s32 pan, SoundPanMode pan_mode, s32 pitch, s32 tempo, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSoundPlay, start_frame, end_frame, group, condition);
  step.SetS32(0, sound_id);
  step.SetS32(1, player);
  step.SetS32(2, volume);
  step.SetS32(3, pan);
  step.SetS32(4, static_cast<s32>(pan_mode));
  step.SetS32(5, pitch);
  step.SetS32(6, tempo);
  return step;
}

ChoreographyStep MoveChoreography::SoundVolume(s32 start_frame, s32 end_frame, s32 start_volume, s32 end_volume, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSoundVolume, start_frame, end_frame, group, condition);
  step.SetS32(0, start_volume);
  step.SetS32(1, end_volume);
  return step;
}

ChoreographyStep MoveChoreography::SoundPan(s32 start_frame, s32 end_frame, s32 start_pan, s32 end_pan, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSoundPan, start_frame, end_frame, group, condition);
  step.SetS32(0, start_pan);
  step.SetS32(1, end_pan);
  return step;
}

ChoreographyStep MoveChoreography::SoundPitch(s32 start_frame, s32 end_frame, s32 start_pitch, s32 end_pitch, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSoundPitch, start_frame, end_frame, group, condition);
  step.SetS32(0, start_pitch);
  step.SetS32(1, end_pitch);
  return step;
}

ChoreographyStep MoveChoreography::SoundTempo(s32 start_frame, s32 end_frame, s32 start_tempo, s32 end_tempo, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSoundTempo, start_frame, end_frame, group, condition);
  step.SetS32(0, start_tempo);
  step.SetS32(1, end_tempo);
  return step;
}

ChoreographyStep MoveChoreography::SoundPlayCry(s32 start_frame, s32 end_frame, ChoreographyTarget who, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSoundPlayCry, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

ChoreographyStep MoveChoreography::Sound3dSetup(s32 start_frame, s32 end_frame, s32 priority_reduction, f32 pan_range, f32 speed_of_sound, f32 inertia_size, f32 max_volume_distance, f32 unit_distance, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dSetup, start_frame, end_frame, group, condition);
  step.SetS32(0, priority_reduction);
  step.SetF32(1, pan_range);
  step.SetF32(2, speed_of_sound);
  step.SetF32(3, inertia_size);
  step.SetF32(4, max_volume_distance);
  step.SetF32(5, unit_distance);
  return step;
}

ChoreographyStep MoveChoreography::Sound3dPlay(s32 start_frame, s32 end_frame, s32 sound_id, s32 player, s32 volume, s32 pan, s32 pitch, bool debug, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dPlay, start_frame, end_frame, group, condition);
  step.SetS32(0, sound_id);
  step.SetS32(1, player);
  step.SetS32(2, volume);
  step.SetS32(3, pan);
  step.SetS32(4, pitch);
  step.SetS32(5, debug ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::Sound3dStop(s32 start_frame, s32 end_frame, s32 fade_frames, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dStop, start_frame, end_frame, group, condition);
  step.SetS32(0, fade_frames);
  return step;
}

ChoreographyStep MoveChoreography::Sound3dTeleport(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dTeleport, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::Sound3dShift(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::Sound3dGlideToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::Sound3dFollowPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dFollowPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  return step;
}

ChoreographyStep MoveChoreography::Sound3dFollowObject(s32 start_frame, s32 end_frame, FollowSource source, s32 object_index, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dFollowObject, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(source));
  step.SetS32(1, object_index);
  step.SetVec3(2, offset);
  return step;
}

ChoreographyStep MoveChoreography::Sound3dGlideToFieldPoint(s32 start_frame, s32 end_frame, FieldPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dGlideToFieldPoint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(point));
  step.SetVec3(1, offset);
  return step;
}

ChoreographyStep MoveChoreography::Sound3dPlayCry(s32 start_frame, s32 end_frame, ChoreographyTarget who, s32 volume, s32 pitch, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dPlayCry, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, volume);
  step.SetS32(2, pitch);
  return step;
}

ChoreographyStep MoveChoreography::HealthBarShow(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kHealthBarShow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::HealthBarShowAll(s32 start_frame, s32 end_frame, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kHealthBarShowAll, start_frame, end_frame, group, condition);
  step.SetS32(0, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::HealthBarApplyDamage(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kHealthBarApplyDamage, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::HealthBarApplyDamageAll(s32 start_frame, s32 end_frame, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kHealthBarApplyDamageAll, start_frame, end_frame, group, condition);
  step.SetS32(0, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::KnockbackFlag(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool use_default, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kKnockbackFlag, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, use_default ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::BreakObjectsFlag(s32 start_frame, s32 end_frame, ChoreographyTarget who, BreakKind kind, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kBreakObjectsFlag, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(kind));
  return step;
}

ChoreographyStep MoveChoreography::BackdropColorToggle(s32 start_frame, s32 end_frame, bool enabled, const Vec3& color, f32 alpha, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kBackdropColorToggle, start_frame, end_frame, group, condition);
  step.SetS32(0, enabled ? 1 : 0);
  step.SetVec3(1, color);
  step.SetF32(4, alpha);
  return step;
}

ChoreographyStep MoveChoreography::BackdropColor(s32 start_frame, s32 end_frame, const Vec3& color, f32 alpha, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kBackdropColor, start_frame, end_frame, group, condition);
  step.SetVec3(0, color);
  step.SetF32(3, alpha);
  return step;
}

ChoreographyStep MoveChoreography::BackdropShow(s32 start_frame, s32 end_frame, bool shown, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kBackdropShow, start_frame, end_frame, group, condition);
  step.SetS32(0, shown ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::ScreenFadeIn(s32 start_frame, s32 end_frame, FadeScreen screen, FadeColor color, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kScreenFadeIn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(screen));
  step.SetS32(1, static_cast<s32>(color));
  return step;
}

ChoreographyStep MoveChoreography::ScreenFadeOut(s32 start_frame, s32 end_frame, FadeScreen screen, FadeColor color, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kScreenFadeOut, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(screen));
  step.SetS32(1, static_cast<s32>(color));
  return step;
}

ChoreographyStep MoveChoreography::DebrisSpawn(s32 start_frame, s32 end_frame, EffectModelId model, ModelDrawMode draw_mode, s32 max_count, s32 interval, s32 count_per_emit, s32 rate, s32 life, ClusterShape shape, s32 axis, const Vec3& size, s32 start_degrees, s32 end_degrees, s32 length, bool per_target, bool effect_lighting, bool outline, bool auto_rotate, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisSpawn, start_frame, end_frame, group, condition);
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

ChoreographyStep MoveChoreography::DebrisTeleport(s32 start_frame, s32 end_frame, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisTeleport, start_frame, end_frame, group, condition);
  step.SetVec3(0, position);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::DebrisShift(s32 start_frame, s32 end_frame, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisShift, start_frame, end_frame, group, condition);
  step.SetVec3(0, offset);
  return step;
}

ChoreographyStep MoveChoreography::DebrisGlideToPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisGlideToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::DebrisGlideToFieldPoint(s32 start_frame, s32 end_frame, FieldPoint point, const Vec3& offset, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisGlideToFieldPoint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(point));
  step.SetVec3(1, offset);
  return step;
}

ChoreographyStep MoveChoreography::DebrisTurn(s32 start_frame, s32 end_frame, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisTurn, start_frame, end_frame, group, condition);
  step.SetVec3(0, degrees);
  step.SetS32(3, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::DebrisResize(s32 start_frame, s32 end_frame, const Vec3& scale, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisResize, start_frame, end_frame, group, condition);
  step.SetVec3(0, scale);
  return step;
}

ChoreographyStep MoveChoreography::DebrisSpread(s32 start_frame, s32 end_frame, s32 shape, const Vec3& offset, f32 speed, f32 acceleration, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisSpread, start_frame, end_frame, group, condition);
  step.SetS32(0, shape);
  step.SetVec3(1, offset);
  step.SetF32(4, speed);
  step.SetF32(5, acceleration);
  return step;
}

ChoreographyStep MoveChoreography::DebrisVelocity(s32 start_frame, s32 end_frame, const Vec3& degrees, s32 random_degrees, f32 speed, f32 acceleration, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisVelocity, start_frame, end_frame, group, condition);
  step.SetVec3(0, degrees);
  step.SetS32(3, random_degrees);
  step.SetF32(4, speed);
  step.SetF32(5, acceleration);
  return step;
}

ChoreographyStep MoveChoreography::DebrisGravity(s32 start_frame, s32 end_frame, s32 mode, const Vec3& position, bool absolute, f32 power, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisGravity, start_frame, end_frame, group, condition);
  step.SetS32(0, mode);
  step.SetVec3(1, position);
  step.SetS32(4, absolute ? 1 : 0);
  step.SetF32(5, power);
  return step;
}

ChoreographyStep MoveChoreography::DebrisInitialSize(s32 start_frame, s32 end_frame, const Vec3& minimum, const Vec3& maximum, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisInitialSize, start_frame, end_frame, group, condition);
  step.SetVec3(0, minimum);
  step.SetVec3(3, maximum);
  return step;
}

ChoreographyStep MoveChoreography::DebrisInitialTurn(s32 start_frame, s32 end_frame, const Vec3& minimum, const Vec3& maximum, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisInitialTurn, start_frame, end_frame, group, condition);
  step.SetVec3(0, minimum);
  step.SetVec3(3, maximum);
  return step;
}

ChoreographyStep MoveChoreography::DebrisSizeCurve(s32 start_frame, s32 end_frame, const ChoreographyKey (&keys)[5], u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisSizeCurve, start_frame, end_frame, group, condition);
  for (u32 i = 0; i < 5; i++) {
    step.SetS32(0 + i * 4, keys[i].frame);
    step.SetVec3(0 + i * 4 + 1, keys[i].value);
  }
  return step;
}

ChoreographyStep MoveChoreography::DebrisTurnCurve(s32 start_frame, s32 end_frame, const ChoreographyKey (&keys)[5], u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisTurnCurve, start_frame, end_frame, group, condition);
  for (u32 i = 0; i < 5; i++) {
    step.SetS32(0 + i * 4, keys[i].frame);
    step.SetVec3(0 + i * 4 + 1, keys[i].value);
  }
  return step;
}

ChoreographyStep MoveChoreography::DebrisMaterialAnimation(s32 start_frame, s32 end_frame, EffectModelId choreography, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisMaterialAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(choreography));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::DebrisModelAnimation(s32 start_frame, s32 end_frame, EffectModelId choreography, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisModelAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(choreography));
  step.SetS32(1, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::DebrisReflection(s32 start_frame, s32 end_frame, f32 position, bool absolute, ClusterAnchor plane, ReflectionMode mode, f32 brake, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebrisReflection, start_frame, end_frame, group, condition);
  step.SetF32(0, position);
  step.SetS32(1, absolute ? 1 : 0);
  step.SetS32(2, static_cast<s32>(plane));
  step.SetS32(3, static_cast<s32>(mode));
  step.SetF32(4, brake);
  return step;
}

ChoreographyStep MoveChoreography::ChainAttackSetup(s32 start_frame, s32 end_frame, const ChainWindow (&windows)[6], u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kChainAttackSetup, start_frame, end_frame, group, condition);
  for (u32 i = 0; i < 6; i++) {
    step.SetS32(0 + i * 2, windows[i].start_frame);
    step.SetS32(0 + i * 2 + 1, windows[i].end_frame);
  }
  return step;
}

ChoreographyStep MoveChoreography::MegaEvolution(s32 start_frame, s32 end_frame, ChoreographyTarget who, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kMegaEvolution, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

ChoreographyStep MoveChoreography::ChargeHide(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kChargeHide, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::SubstituteShow(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSubstituteShow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::AegislashFormChange(s32 start_frame, s32 end_frame, ChoreographyTarget who, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kAegislashFormChange, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

ChoreographyStep MoveChoreography::WeatherChange(s32 start_frame, s32 end_frame, WeatherKind weather, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kWeatherChange, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(weather));
  return step;
}

ChoreographyStep MoveChoreography::FieldEffectStart(s32 start_frame, s32 end_frame, FieldEffectStyle kind, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kFieldEffectStart, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(kind));
  return step;
}

ChoreographyStep MoveChoreography::FieldEffectEnd(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kFieldEffectEnd, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::PlatformControl(s32 start_frame, s32 end_frame, s32 mode, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPlatformControl, start_frame, end_frame, group, condition);
  step.SetS32(0, mode);
  return step;
}

ChoreographyStep MoveChoreography::TrainerTeleport(s32 start_frame, s32 end_frame, TrainerSlot trainer, const Vec3& position, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kTrainerTeleport, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetVec3(1, position);
  return step;
}

ChoreographyStep MoveChoreography::TrainerTurn(s32 start_frame, s32 end_frame, TrainerSlot trainer, const Vec3& degrees, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kTrainerTurn, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetVec3(1, degrees);
  return step;
}

ChoreographyStep MoveChoreography::TrainerTurnToPokemon(s32 start_frame, s32 end_frame, TrainerSlot trainer, ChoreographyTarget target, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kTrainerTurnToPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetS32(1, static_cast<s32>(target));
  return step;
}

ChoreographyStep MoveChoreography::TrainerShow(s32 start_frame, s32 end_frame, TrainerSlot trainer, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kTrainerShow, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::TrainerPlayPose(s32 start_frame, s32 end_frame, TrainerSlot trainer, TrainerPose pose, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kTrainerPlayPose, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetS32(1, static_cast<s32>(pose));
  step.SetS32(2, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::TrainerPoseSpeed(s32 start_frame, s32 end_frame, TrainerSlot trainer, f32 speed, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kTrainerPoseSpeed, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetF32(1, speed);
  return step;
}

ChoreographyStep MoveChoreography::TrainerShowBallGauge(s32 start_frame, s32 end_frame, TrainerSlot trainer, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kTrainerShowBallGauge, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  return step;
}

ChoreographyStep MoveChoreography::TrainerTint(s32 start_frame, s32 end_frame, TrainerSlot trainer, f32 start_strength, f32 end_strength, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kTrainerTint, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(trainer));
  step.SetF32(1, start_strength);
  step.SetF32(2, end_strength);
  return step;
}

ChoreographyStep MoveChoreography::OverlayCreate(s32 start_frame, s32 end_frame, OverlayLayoutId layout, s32 choreography_count, OverlayPlane plane, s32 priority, bool mono, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayCreate, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(layout));
  step.SetS32(1, choreography_count);
  step.SetS32(2, static_cast<s32>(plane));
  step.SetS32(3, priority);
  step.SetS32(4, mono ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::OverlayDelete(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayDelete, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::OverlayAnimationStart(s32 start_frame, s32 end_frame, s32 choreography_index, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayAnimationStart, start_frame, end_frame, group, condition);
  step.SetS32(0, choreography_index);
  return step;
}

ChoreographyStep MoveChoreography::OverlayAnimationStop(s32 start_frame, s32 end_frame, s32 choreography_index, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayAnimationStop, start_frame, end_frame, group, condition);
  step.SetS32(0, choreography_index);
  return step;
}

ChoreographyStep MoveChoreography::OverlayAnimationSpeed(s32 start_frame, s32 end_frame, s32 choreography_index, f32 speed, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayAnimationSpeed, start_frame, end_frame, group, condition);
  step.SetS32(0, choreography_index);
  step.SetF32(1, speed);
  return step;
}

ChoreographyStep MoveChoreography::OverlayAnimationFrame(s32 start_frame, s32 end_frame, s32 choreography_index, f32 frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayAnimationFrame, start_frame, end_frame, group, condition);
  step.SetS32(0, choreography_index);
  step.SetF32(1, frame);
  return step;
}

ChoreographyStep MoveChoreography::OverlayDarken(s32 start_frame, s32 end_frame, s32 red, s32 green, s32 blue, const char* pane_name, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayDarken, start_frame, end_frame, group, condition);
  step.SetS32(0, red);
  step.SetS32(1, green);
  step.SetS32(2, blue);
  step.SetText(3, pane_name, 16);
  return step;
}

ChoreographyStep MoveChoreography::OverlayBrighten(s32 start_frame, s32 end_frame, s32 red, s32 green, s32 blue, const char* pane_name, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayBrighten, start_frame, end_frame, group, condition);
  step.SetS32(0, red);
  step.SetS32(1, green);
  step.SetS32(2, blue);
  step.SetText(3, pane_name, 16);
  return step;
}

ChoreographyStep MoveChoreography::OverlayTrainerPicture(s32 start_frame, s32 end_frame, TrainerPicture picture, s32 index, const char* pane_name, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayTrainerPicture, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(picture));
  step.SetS32(1, index);
  step.SetText(2, pane_name, 16);
  return step;
}

ChoreographyStep MoveChoreography::OverlayText(s32 start_frame, s32 end_frame, const char* source_pane, const char* destination_pane, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayText, start_frame, end_frame, group, condition);
  step.SetText(0, source_pane, 24);
  step.SetText(6, destination_pane, 24);
  return step;
}

ChoreographyStep MoveChoreography::OverlayDepth(s32 start_frame, s32 end_frame, f32 depth, f32 parallax, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayDepth, start_frame, end_frame, group, condition);
  step.SetF32(0, depth);
  step.SetF32(1, parallax);
  return step;
}

ChoreographyStep MoveChoreography::EffectSpawnForContest(s32 start_frame, s32 end_frame, EffectId cool, EffectId beauty, EffectId cute, EffectId smart, EffectId tough, bool per_target, EffectDrawOrder layer, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectSpawnForContest, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(cool));
  step.SetS32(1, static_cast<s32>(beauty));
  step.SetS32(2, static_cast<s32>(cute));
  step.SetS32(3, static_cast<s32>(smart));
  step.SetS32(4, static_cast<s32>(tough));
  step.SetS32(5, per_target ? 1 : 0);
  step.SetS32(6, static_cast<s32>(layer));
  return step;
}

ChoreographyStep MoveChoreography::InputWindow(s32 start_frame, s32 end_frame, s32 index, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kInputWindow, start_frame, end_frame, group, condition);
  step.SetS32(0, index);
  return step;
}

ChoreographyStep MoveChoreography::PokemonTeleportQuad(s32 start_frame, s32 end_frame, ContestSlot who, const Vec3& position, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonTeleportQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, position);
  step.SetS32(4, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonTintQuad(s32 start_frame, s32 end_frame, ContestSlot who, const Vec3& start_color, f32 start_strength, const Vec3& end_color, f32 end_strength, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonTintQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, start_color);
  step.SetF32(4, start_strength);
  step.SetVec3(5, end_color);
  step.SetF32(8, end_strength);
  return step;
}

ChoreographyStep MoveChoreography::PokemonSpinQuad(s32 start_frame, s32 end_frame, ContestSlot who, const Vec3& degrees, bool mirror, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonSpinQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, degrees);
  step.SetS32(4, mirror ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipOnPokemonQuad(s32 start_frame, s32 end_frame, EffectModelId clip, ContestSlot who, BodyPoint point, bool mirror, PositionAdjust adjust, bool keep_transform, bool loop, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipOnPokemonQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(clip));
  step.SetS32(1, static_cast<s32>(who));
  step.SetS32(2, static_cast<s32>(point));
  step.SetS32(3, mirror ? 1 : 0);
  step.SetS32(4, static_cast<s32>(adjust));
  step.SetS32(5, keep_transform ? 1 : 0);
  step.SetS32(6, loop ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraGlideToPokemonQuad(s32 start_frame, s32 end_frame, ContestSlot target, BodyPoint point, const Vec3& camera_position, const Vec3& look_at, f32 field_of_view, PositionAdjust adjust, MoveCurve curve, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraGlideToPokemonQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, camera_position);
  step.SetVec3(5, look_at);
  step.SetF32(8, field_of_view);
  step.SetS32(9, static_cast<s32>(adjust));
  step.SetS32(10, static_cast<s32>(curve));
  return step;
}

ChoreographyStep MoveChoreography::EffectGlideToPokemonQuad(s32 start_frame, s32 end_frame, ContestSlot target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectGlideToPokemonQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::ModelGlideToPokemonQuad(s32 start_frame, s32 end_frame, ContestSlot target, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelGlideToPokemonQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, offset);
  step.SetS32(5, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::PokemonHappyPose(s32 start_frame, s32 end_frame, ContestSlot who, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonHappyPose, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  return step;
}

ChoreographyStep MoveChoreography::OverlayTrainerPictureAndText(s32 start_frame, s32 end_frame, TrainerPicture picture, const char* picture_pane, const char* name_pane, const char* entry_pane, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayTrainerPictureAndText, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(picture));
  step.SetText(1, picture_pane, 16);
  step.SetText(5, name_pane, 16);
  step.SetText(9, entry_pane, 16);
  return step;
}

ChoreographyStep MoveChoreography::SoundPlayCryQuad(s32 start_frame, s32 end_frame, ContestSlot who, bool happy, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSoundPlayCryQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, happy ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonShowQuad(s32 start_frame, s32 end_frame, ContestSlot who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonShowQuad, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::SubstituteVisibility(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSubstituteVisibility, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonSkyPose(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonSkyPose, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::OverlayTextContest(s32 start_frame, s32 end_frame, const char* source_pane, const char* destination_pane, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kOverlayTextContest, start_frame, end_frame, group, condition);
  step.SetText(0, source_pane, 24);
  step.SetText(6, destination_pane, 24);
  return step;
}

ChoreographyStep MoveChoreography::PokemonShadeEffect(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonShadeEffect, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::PokemonPointLight(s32 start_frame, s32 end_frame, ChoreographyTarget who, const Vec3& color, const Vec3& position, s32 power, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonPointLight, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetVec3(1, color);
  step.SetVec3(4, position);
  step.SetS32(7, power);
  return step;
}

ChoreographyStep MoveChoreography::PokemonGazeAt(s32 start_frame, s32 end_frame, ChoreographyTarget who, ChoreographyTarget toward, BodyPoint point, const Vec3& offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonGazeAt, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(toward));
  step.SetS32(2, static_cast<s32>(point));
  step.SetVec3(3, offset);
  step.SetS32(6, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::MessageShow(s32 start_frame, s32 end_frame, s32 message_id, s32 option, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kMessageShow, start_frame, end_frame, group, condition);
  step.SetS32(0, message_id);
  step.SetS32(1, option);
  return step;
}

ChoreographyStep MoveChoreography::MessageDismiss(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kMessageDismiss, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::ModelRemove(s32 start_frame, s32 end_frame, bool distortion, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelRemove, start_frame, end_frame, group, condition);
  step.SetS32(0, distortion ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::EffectDelete(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectDelete, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::ModelResetSize(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelResetSize, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::ModelArc(s32 start_frame, s32 end_frame, f32 speed, f32 gravity, f32 end_height, bool relative, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kModelArc, start_frame, end_frame, group, condition);
  step.SetF32(0, speed);
  step.SetF32(1, gravity);
  step.SetF32(2, end_height);
  step.SetS32(3, relative ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::CameraClipScaleValue(s32 start_frame, s32 end_frame, f32 scale, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kCameraClipScaleValue, start_frame, end_frame, group, condition);
  step.SetF32(0, scale);
  return step;
}

ChoreographyStep MoveChoreography::WaitForAnimation(s32 start_frame, s32 end_frame, WaitKind kind, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kWaitForAnimation, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(kind));
  return step;
}

ChoreographyStep MoveChoreography::Sound3dTest(s32 start_frame, s32 end_frame, s32 sound_id, s32 player, f32 horizontal_pan, f32 vertical_pan, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kSound3dTest, start_frame, end_frame, group, condition);
  step.SetS32(0, sound_id);
  step.SetS32(1, player);
  step.SetF32(2, horizontal_pan);
  step.SetF32(3, vertical_pan);
  return step;
}

ChoreographyStep MoveChoreography::PlatformShow(s32 start_frame, s32 end_frame, bool shown, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPlatformShow, start_frame, end_frame, group, condition);
  step.SetS32(0, shown ? 1 : 0);
  return step;
}

ChoreographyStep MoveChoreography::DebugSwapPokemon(s32 start_frame, s32 end_frame, ChoreographyTarget target, s32 species, s32 form, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebugSwapPokemon, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, species);
  step.SetS32(2, form);
  return step;
}

ChoreographyStep MoveChoreography::DebugProbe(s32 start_frame, s32 end_frame, const char* first, const char* second, const char* padding, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kDebugProbe, start_frame, end_frame, group, condition);
  step.SetText(0, first, 32);
  step.SetText(8, second, 32);
  step.SetText(16, padding, 32);
  return step;
}

ChoreographyStep MoveChoreography::EffectGlideToPokemonLegacy(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& temporary_offset, const Vec3& effect_offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectGlideToPokemonLegacy, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, temporary_offset);
  step.SetVec3(5, effect_offset);
  step.SetS32(8, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::EffectGlideToPokemonRotatedLegacy(s32 start_frame, s32 end_frame, ChoreographyTarget target, BodyPoint point, const Vec3& temporary_offset, const Vec3& effect_offset, f32 rotation_offset, PositionAdjust adjust, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kEffectGlideToPokemonRotatedLegacy, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(target));
  step.SetS32(1, static_cast<s32>(point));
  step.SetVec3(2, temporary_offset);
  step.SetVec3(5, effect_offset);
  step.SetF32(8, rotation_offset);
  step.SetS32(9, static_cast<s32>(adjust));
  return step;
}

ChoreographyStep MoveChoreography::PokemonPlayPoseDirect(s32 start_frame, s32 end_frame, ChoreographyTarget who, PokemonPose pose, s32 reset_wait, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonPlayPoseDirect, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, static_cast<s32>(pose));
  step.SetS32(2, reset_wait);
  return step;
}

ChoreographyStep MoveChoreography::PokemonShowSpecial(s32 start_frame, s32 end_frame, ChoreographyTarget who, bool visible, f32 height, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonShowSpecial, start_frame, end_frame, group, condition);
  step.SetS32(0, static_cast<s32>(who));
  step.SetS32(1, visible ? 1 : 0);
  step.SetF32(2, height);
  return step;
}

ChoreographyStep MoveChoreography::PokemonWildCry(s32 start_frame, s32 end_frame, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonWildCry, start_frame, end_frame, group, condition);
  return step;
}

ChoreographyStep MoveChoreography::PokemonResetAll(s32 start_frame, s32 end_frame, bool skip_in_contest, u16 group, StepCondition condition) {
  ChoreographyStep step = Add(ChoreographyStepKind::kPokemonResetAll, start_frame, end_frame, group, condition);
  step.SetS32(0, skip_in_contest ? 1 : 0);
  return step;
}

}
