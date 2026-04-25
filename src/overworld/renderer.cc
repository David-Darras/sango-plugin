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

#include "overworld/renderer.h"

#include <cmath>

#include "battle/manager.h"
#include "hack/hook_manager.h"
#include "menu/plugin_menu.h"
#include "overworld/model_manager.h"

namespace overworld {

enum class CameraState { kIdle, kFree, kRotate, kFpv };

static struct {
  u8 state;
  u8 old_state;
  Vec3 rot;
  Vec3 pos;
  Vec3 up;
  Vec3 target;
  f32 radius;
  f32 height;
  f32 theta;
  f32 theta_speed;
  bool is_updating_camera;
} ctx;

void UpdateMatrices(StereoCamera* stereo_camera, bool update) {
  if (&Renderer::GetInstance().GetStereoCamera() == stereo_camera ||
      (battle::Process::IsInBattle() &&
       &battle::Graphics::GetInstance().GetStereoCamera() == stereo_camera)) {
    ctx.is_updating_camera = true;
  }
  HookManager::GetInstance()
      .Get(HookID::kUpdateMatrices)
      ->CallOriginal<u32>(stereo_camera, update);
}

Mtx34* UpdateLookAt(Mtx34* output, Vec3* pos, Vec3* up, Vec3* target) {
  if (!ctx.is_updating_camera) {
    return HookManager::GetInstance()
        .Get(HookID::kUpdateLookAt)
        ->CallOriginal<Mtx34*>(output, pos, up, target);
  }
  ctx.is_updating_camera = false;

  if (ctx.state != ctx.old_state) {
    if (static_cast<CameraState>(ctx.state) == CameraState::kFree) {
      ctx.pos = *pos;
      f32 dx = target->x - pos->x;
      f32 dy = target->y - pos->y;
      f32 dz = target->z - pos->z;
      f32 dist = std::sqrt(dx * dx + dy * dy + dz * dz);

      ctx.rot.y = std::atan2(dz, dx);
      ctx.rot.x = (dist > 0.0001f) ? std::asin(dy / dist) : 0.0f;
    }
    ctx.old_state = ctx.state;
  }

  Model& player = ModelManager::GetInstance().GetPlayer();

  switch (static_cast<CameraState>(ctx.state)) {
    case CameraState::kRotate:
      if (battle::Process::IsInBattle()) {
        *target = Vec3{0, 0, 0};
      } else {
        *target = player.draw_pos;
      }
      *up = {0.0f, 1.0f, 0.0f};
      pos->x = target->x + ctx.radius * std::cos(ctx.theta);
      pos->z = target->z + ctx.radius * std::sin(ctx.theta);
      pos->y = target->y + ctx.height;
      ctx.theta += ctx.theta_speed;
      break;

    case CameraState::kFree: {
      Vec3 forward = {std::cos(ctx.rot.y) * std::cos(ctx.rot.x),
                      std::sin(ctx.rot.x),
                      std::sin(ctx.rot.y) * std::cos(ctx.rot.x)};

      *pos = ctx.pos;
      *target = {ctx.pos.x + forward.x, ctx.pos.y + forward.y,
                 ctx.pos.z + forward.z};
      *up = {0.0f, 1.0f, 0.0f};
      break;
    }

    case CameraState::kFpv:
      pos->x = player.draw_pos.x;
      pos->y = player.draw_pos.y + 30.0f;
      pos->z = player.draw_pos.z;

      *target = {pos->x + player.facing_direction.x,
                 pos->y + player.facing_direction.y,
                 pos->z + player.facing_direction.z};
      *up = {0.0f, 1.0f, 0.0f};
      break;

    case CameraState::kIdle:
    default:
      break;
  }

  return HookManager::GetInstance()
      .Get(HookID::kUpdateLookAt)
      ->CallOriginal<Mtx34*>(output, pos, up, target);
}

void StereoCamera::LoadMenu(menu::PluginMenu& menu, void* args) {
  static const c8* STATES[] = {"Idle", "Free", "Rotate", "Fpv"};
  bool& skybox = *(bool*)((uptr)&Renderer::GetInstance() + 0xB74);

  menu.WithNoBackground()
      .Add("Skybox", skybox)
      .Add("State", ctx.state)
      .WithArray(STATES, SIZE(STATES))
      .Add("Free Pos X (Left/Right)", ctx.pos.x)
      .WithFactor(5.0f)
      .Add("Free Pos Y (Up/Down)", ctx.pos.y)
      .WithFactor(5.0f)
      .Add("Free Pos Z (Forward/Back)", ctx.pos.z)
      .WithFactor(5.0f)
      .Add("Free Yaw (Turn)", ctx.rot.y)
      .WithFactor(0.05f)
      .Add("Free Pitch (Look)", ctx.rot.x)
      .WithFactor(0.05f)
      .Add("Radius", ctx.radius)
      .WithFactor(3.0f)
      .Add("Height", ctx.height)
      .WithFactor(3.0f)
      .Add("Orbit Rot Speed", ctx.theta_speed)
      .WithFactor(0.01f);
}

void StereoCamera::SetupHooks() {
  HookManager& man = HookManager::GetInstance();
  man.Add(HookID::kUpdateMatrices, ADDRESS_STEREO_CAMERA_UPDATE_MATRICES,
          (uptr)UpdateMatrices);
  man.Add(HookID::kUpdateLookAt, ADDRESS_STEREO_CAMERA_UPDATE_LOOK_AT,
          (uptr)UpdateLookAt);
}

}  // namespace overworld