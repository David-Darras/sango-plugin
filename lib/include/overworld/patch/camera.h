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

#include <type_traits>

#include "common.h"
#include "overworld/constant/camera_context.h"
#include "overworld/constant/camera_state.h"
#include "overworld/constant/facing.h"

namespace overworld {
struct StereoCamera;

struct CameraSettings {
  CameraState overworld_state = CameraState::kIdle;
  CameraState overworld_old_state = CameraState::kIdle;
  CameraState battle_state = CameraState::kIdle;
  CameraState battle_old_state = CameraState::kIdle;

  u8 battle_target_pokemon_slot = 0;

  Vec3 rot;
  Vec3 pos;
  Vec3 up;
  Vec3 target;
  f32 radius = 2000.0f;
  f32 height = 800.0f;
  f32 theta = 0.0f;
  f32 theta_speed = 0.005f;
  bool is_updating_camera = false;
  f32 fpv_height = 30.0f;
  f32 fpv_offset = 24.0f;

  f32 tps_dist = 152;
  f32 tps_height = 32;
  f32 tps_offset = 30;
};
static_assert(std::is_standard_layout<CameraSettings>::value,
              "CameraSettings must have standard layout");

struct Camera : public CameraSettings {
  MAKE_SINGLETON(Camera)

  CameraContext active_context = CameraContext::kNone;

  void SetCameraIdle(bool is_battle);
  void SetCameraFree(bool is_battle, f32 x, f32 y, f32 z, f32 yaw, f32 pitch);
  void SetCameraRotate(bool is_battle, f32 r, f32 h, f32 w = 0.0f);
  void SetCameraTPS(bool is_battle, f32 dist, f32 height, f32 offset);

  static void Initialize();

private:
  static u32 GetPlayerMovement(uptr self, Vec3* dir_vec, f32* speed,
                               Facing* dir, u8* x);
  static u32 UpdateMatricesHook(StereoCamera* stereo_camera, bool update);
  static Mtx34* UpdateLookAtHook(Mtx34* output, Vec3* pos, Vec3* up,
                                 Vec3* target);
};
} // namespace overworld
