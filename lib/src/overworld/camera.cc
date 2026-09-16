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

#include "overworld/patch/camera.h"
#include "battle/native/graphics.h"

#include <cmath>
#include "battle/native/manager.h"
#include "core/hook_manager.h"
#include "core/native/process_manager.h"
#include "overworld/constant/facing.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/renderer.h"
#include "system/native/controller.h"

namespace overworld {

void Camera::SetCameraIdle(bool is_battle) {
  (is_battle ? battle_state : overworld_state) = CameraState::kIdle;
}

void Camera::SetCameraFree(bool is_battle, f32 x, f32 y, f32 z, f32 yaw,
                           f32 pitch) {
  CameraState& state_ref = is_battle ? battle_state : overworld_state;
  CameraState& old_state_ref =
      is_battle ? battle_old_state : overworld_old_state;
  state_ref = CameraState::kFree;
  pos.x = x;
  pos.y = y;
  pos.z = z;
  rot.y = yaw;
  rot.x = pitch;
  old_state_ref = state_ref;
}

void Camera::SetCameraRotate(bool is_battle, f32 r, f32 h, f32 w) {
  (is_battle ? battle_state : overworld_state) = CameraState::kRotate;
  radius = r;
  height = h;
  theta = w;
}

void Camera::SetCameraTPS(bool is_battle, f32 dist, f32 height, f32 offset) {
  (is_battle ? battle_state : overworld_state) = CameraState::kTps;
  tps_dist = dist;
  tps_height = height;
  tps_offset = offset;
}

void Camera::Initialize() {
  core::HookManager::Initialize(HookId::kUpdateMatrices,
                          renderer::address::kStereoCameraUpdateMatrices,
                          (uptr)UpdateMatricesHook);
  core::HookManager::Initialize(HookId::kUpdateLookAt,
                          renderer::address::kStereoCameraUpdateLookAt,
                          (uptr)UpdateLookAtHook);
  core::HookManager::Initialize(HookId::kGetPlayerMovement,
                          address::kGetPlayerMovement,
                          (uptr)GetPlayerMovement, false);
}

u32 Camera::GetPlayerMovement(uptr self, Vec3* dir_vec, f32* speed,
                              Facing* dir, u8* x) {
  u32 result = core::HookManager::Call<u32>(HookId::kGetPlayerMovement,
                                      self, dir_vec, speed, dir, x);

  auto& ctx = GetInstance();
  if (ctx.overworld_state != CameraState::kFpv) return result;

  auto& player = ModelManager::GetInstance().GetPlayer();
  auto& controller = sys::Controller::GetInstance();

  if (controller.IsKeyPressed(Key::kL)) {
    if (player.facing_direction.x == 1.0f && player.facing_direction.z == 0.0f) {
      player.facing_direction.x = 0.0f;
      player.facing_direction.z = 1.0f;
    } else if (player.facing_direction.x == -1.0f && player.facing_direction.z == 0.0f) {
      player.facing_direction.x = 0.0f;
      player.facing_direction.z = -1.0f;
    } else if (player.facing_direction.x == 0.0f && player.facing_direction.z == 1.0f) {
      player.facing_direction.x = -1.0f;
      player.facing_direction.z = 0.0f;
    } else if (player.facing_direction.x == 0.0f && player.facing_direction.z == -1.0f) {
      player.facing_direction.x = 1.0f;
      player.facing_direction.z = 0.0f;
    }
  }
  if (controller.IsKeyPressed(Key::kR)) {
    if (player.facing_direction.x == 1.0f && player.facing_direction.z == 0.0f) {
      player.facing_direction.x = 0.0f;
      player.facing_direction.z = -1.0f;
    } else if (player.facing_direction.x == -1.0f && player.facing_direction.z == 0.0f) {
      player.facing_direction.x = 0.0f;
      player.facing_direction.z = 1.0f;
    } else if (player.facing_direction.x == 0.0f && player.facing_direction.z == 1.0f) {
      player.facing_direction.x = 1.0f;
      player.facing_direction.z = 0.0f;
    } else if (player.facing_direction.x == 0.0f && player.facing_direction.z == -1.0f) {
      player.facing_direction.x = -1.0f;
      player.facing_direction.z = 0.0f;
    }
  }

  if (*dir == Facing::kUp) {
    if (player.facing_direction.x > 0.0f) {
      *dir = Facing::kRight;
      dir_vec->x = 1.0f;
      dir_vec->z = 0.0f;
    } else if (player.facing_direction.x < 0.0f) {
      *dir = Facing::kLeft;
      dir_vec->x = -1.0f;
      dir_vec->z = 0.0f;
    } else if (player.facing_direction.z > 0.0f) {
      *dir = Facing::kDown;
      dir_vec->x = 0.0f;
      dir_vec->z = -1.0f;
    } else if (player.facing_direction.z < 0.0f) {
      *dir = Facing::kUp;
      dir_vec->x = 0.0f;
      dir_vec->z = 1.0f;
    }
  } else {
    *dir = Facing::kInvalid;
    dir_vec->x = 0.0f;
    dir_vec->z = 0.0f;
  }

  return result;
}

u32 Camera::UpdateMatricesHook(StereoCamera* stereo_camera,
                              bool update) {
  auto& ctx = GetInstance();
  auto& process_manager = core::ProcessManager::GetInstance();

  if (process_manager.IsCurrentProcess(address::kVtable)) {
    auto* renderer = Renderer::GetInstance();
    if (renderer != nullptr && renderer->GetStereoCamera() == stereo_camera) {
      ctx.is_updating_camera = true;
      ctx.active_context = CameraContext::kOverworld;
    }
  } else if (process_manager.IsCurrentProcess(battle::address::kVtable)) {
    ctx.is_updating_camera = true;
    ctx.active_context = CameraContext::kBattle;
  }

  return core::HookManager::Call<u32>(HookId::kUpdateMatrices, stereo_camera, update);
}

Mtx34* Camera::UpdateLookAtHook(Mtx34* output, Vec3* pos, Vec3* up,
                               Vec3* target) {
  auto& ctx = GetInstance();
  if (!ctx.is_updating_camera) {
    return core::HookManager::Call<Mtx34*>(HookId::kUpdateLookAt, output, pos, up,
                                     target);
  }
  ctx.is_updating_camera = false;

  bool is_battle = (ctx.active_context == CameraContext::kBattle);
  CameraState& state = is_battle ? ctx.battle_state : ctx.overworld_state;
  CameraState& old_state =
      is_battle ? ctx.battle_old_state : ctx.overworld_old_state;

  if (state != old_state) {
    if (state == CameraState::kFree) {
      ctx.pos = *pos;
      f32 dx = target->x - pos->x;
      f32 dy = target->y - pos->y;
      f32 dz = target->z - pos->z;
      f32 dist = std::sqrt(dx * dx + dy * dy + dz * dz);

      ctx.rot.y = std::atan2(dz, dx);
      ctx.rot.x = (dist > 0.0001f) ? std::asin(dy / dist) : 0.0f;
    }
    old_state = state;
  }

  Vec3 target_pos;
  Vec3 facing;
  if (is_battle) {
    auto& model = battle::Manager::GetInstance().GetGraphics().
        GetPokemonModel(ctx.battle_target_pokemon_slot);
    target_pos = model.position;
    facing = {std::sin(model.rotation.y), 0.0f, std::cos(model.rotation.y)};
  } else {
    auto& player = ModelManager::GetInstance().GetPlayer();
    target_pos = player.draw_pos;
    facing = player.facing_direction;
  }

  switch (state) {
    case CameraState::kRotate:
      *target = target_pos;
      *up = {0.0f, 1.0f, 0.0f};
      pos->x = target->x + ctx.radius * std::cos(ctx.theta);
      pos->z = target->z + ctx.radius * std::sin(ctx.theta);
      pos->y = target->y + ctx.height;
      ctx.theta += ctx.theta_speed;
      break;

    case CameraState::kTop:
      *target = target_pos;
      *up = {0.0f, 0.0f, -1.0f};
      pos->x = target->x;
      pos->z = target->z;
      pos->y = target->y + 300.0f;
      break;

    case CameraState::kFree: {
      Vec3 forward = {
          std::cos(ctx.rot.y) * std::cos(ctx.rot.x),
          std::sin(ctx.rot.x),
          std::sin(ctx.rot.y) * std::cos(ctx.rot.x)
      };

      *pos = ctx.pos;
      *target = {
          ctx.pos.x + forward.x, ctx.pos.y + forward.y,
          ctx.pos.z + forward.z
      };
      *up = {0.0f, 1.0f, 0.0f};
      break;
    }

    case CameraState::kFpv: {
      Vec3 dir = facing;
      f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
      if (len > 0.0001f) {
        dir.x /= len;
        dir.z /= len;
      }

      pos->x = target_pos.x - (dir.x * ctx.fpv_offset);
      pos->y = target_pos.y + ctx.fpv_height;
      pos->z = target_pos.z - (dir.z * ctx.fpv_offset);

      *target = {
          pos->x + dir.x,
          pos->y,
          pos->z + dir.z
      };
      *up = {0.0f, 1.0f, 0.0f};
      break;
    }

    case CameraState::kTps: {
      Vec3 dir = facing;
      f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
      if (len > 0.0f) {
        dir.x /= len;
        dir.z /= len;
      }

      pos->x = target_pos.x - (dir.x * ctx.tps_dist) +
               (-dir.z * ctx.tps_offset);
      pos->y = target_pos.y + ctx.tps_height;
      pos->z = target_pos.z - (dir.z * ctx.tps_dist) +
               (dir.x * ctx.tps_offset);

      *target = {
          target_pos.x, target_pos.y + ctx.tps_height,
          target_pos.z
      };
      *up = {0.0f, 1.0f, 0.0f};
      break;
    }

    case CameraState::kIdle:
    default:
      break;
  }

  return core::HookManager::Call<Mtx34*>(HookId::kUpdateLookAt, output, pos, up,
                                   target);
}

} // namespace overworld
