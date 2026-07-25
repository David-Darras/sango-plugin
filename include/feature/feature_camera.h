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
#include <cmath>

#include "common.h"
#include "hook_manager.h"
#include "game/battle/manager.h"
#include "game/process_manager.h"
#include "game/overworld/model_manager.h"
#include "game/overworld/renderer.h"

namespace overworld {
class StereoCamera;
}

namespace feature {
struct Camera {
  enum State { kIdle, kTps, kRotate, kTop, kFpv, kFree };

  void SetCameraIdle() {
    state = State::kIdle;
  }

  void SetCameraFree(f32 x, f32 y, f32 z, f32 yaw, f32 pitch) {
    state = State::kFree;
    pos.x = x;
    pos.y = y;
    pos.z = z;
    rot.y = yaw;
    rot.x = pitch;
    old_state = state;
  }

  void SetCameraRotate(f32 r, f32 h, f32 w = 0.0f) {
    state = State::kRotate;
    radius = r;
    height = h;
    theta = w;
  }

  void SetCameraTPS(f32 dist, f32 height, f32 offset) {
    state = State::kTps;
    tps_dist = dist;
    tps_height = height;
    tps_offset = offset;
  }

  MAKE_SINGLETON(Camera)
  u32 state = kIdle;
  u8 old_state = 0;
  Vec3 rot;
  Vec3 pos;
  Vec3 up;
  Vec3 target;
  f32 radius = 200.0f;
  f32 height = 100.0f;
  f32 theta = 0.0f;
  f32 theta_speed = 0.005f;
  bool is_updating_camera = false;
  f32 fpv_height = 30.0f;
  f32 fpv_offset = 24.0f;

  f32 tps_dist = 152;
  f32 tps_height = 32;
  f32 tps_offset = 30;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kUpdateMatrices,
                            ADDRESS_STEREO_CAMERA_UPDATE_MATRICES,
                            (uptr)UpdateMatricesHook);
    HookManager::Initialize(HookID::kUpdateLookAt,
                            ADDRESS_STEREO_CAMERA_UPDATE_LOOK_AT,
                            (uptr)UpdateLookAtHook);
  }

  static f32 RandomFloatRange(f32 min, f32 max) {
    constexpr u32 kPrecision = 10000;
    f32 t = static_cast<f32>(Utils::GetRandomValue(kPrecision)) / kPrecision;
    return min + t * (max - min);
  }

  static u32 UpdateMatricesHook(overworld::StereoCamera* stereo_camera,
                                bool update) {
    auto& ctx = GetInstance();

    if (game::ProcessManager::GetInstance().
      IsCurrentProcess(PROCESS_NAME_FIELD_MAP)) {
      if (&overworld::Renderer::GetInstance().GetStereoCamera() ==
          stereo_camera) {
        ctx.is_updating_camera = true;
      }
    }
    // if (ProcessManager::GetInstance().IsCurrentProcess(PROCESS_NAME_BATTLE)) {
    //   if (&battle::Graphics::GetInstance().GetStereoCamera() == stereo_camera) {
    //     is_updating_camera = true;
    //   }
    // }

    return HookManager::Call<u32>(HookID::kUpdateMatrices, stereo_camera,
                                  update);
  }

  static Mtx34* UpdateLookAtHook(Mtx34* output, Vec3* pos, Vec3* up,
                                 Vec3* target) {
    auto& ctx = GetInstance();
    if (!ctx.is_updating_camera) {
      return HookManager::GetInstance()
             .Get(HookID::kUpdateLookAt)
             ->CallOriginal<Mtx34*>(output, pos, up, target);
    }
    ctx.is_updating_camera = false;

    if (ctx.state != ctx.old_state) {
      if (static_cast<State>(ctx.state) == State::kFree) {
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

    auto& player = overworld::ModelManager::GetInstance().GetPlayer();

    switch (static_cast<State>(ctx.state)) {
      case State::kRotate:
        *target = player.draw_pos;
        *up = {0.0f, 1.0f, 0.0f};
        pos->x = target->x + ctx.radius * std::cos(ctx.theta);
        pos->z = target->z + ctx.radius * std::sin(ctx.theta);
        pos->y = target->y + ctx.height;
        ctx.theta += ctx.theta_speed;
        break;

      case State::kTop:
        *target = player.draw_pos;
        *up = {0.0f, 0.0f, -1.0f};
        pos->x = target->x;
        pos->z = target->z;
        pos->y = target->y + 300.0f;
        break;

      case State::kFree: {
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

      case State::kFpv: {
        Vec3 dir = player.facing_direction;
        f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (len > 0.0001f) {
          dir.x /= len;
          dir.z /= len;
        }

        pos->x = player.draw_pos.x - (dir.x * ctx.fpv_offset);
        pos->y = player.draw_pos.y + ctx.fpv_height;
        pos->z = player.draw_pos.z - (dir.z * ctx.fpv_offset);

        *target = {
            pos->x + dir.x,
            pos->y,
            pos->z + dir.z
        };
        *up = {0.0f, 1.0f, 0.0f};
        break;
      }

      case State::kTps: {
        Vec3 dir = player.facing_direction;
        f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (len > 0.0f) {
          dir.x /= len;
          dir.z /= len;
        }

        pos->x = player.draw_pos.x - (dir.x * ctx.tps_dist) +
                 (-dir.z * ctx.tps_offset);
        pos->y = player.draw_pos.y + ctx.tps_height;
        pos->z =
            player.draw_pos.z - (dir.z * ctx.tps_dist) + (
              dir.x * ctx.tps_offset);

        *target = {
            player.draw_pos.x, player.draw_pos.y + ctx.tps_height,
            player.draw_pos.z
        };
        *up = {0.0f, 1.0f, 0.0f};
        break;
      }

      case State::kIdle:
      default:
        break;
    }

    return HookManager::GetInstance()
           .Get(HookID::kUpdateLookAt)
           ->CallOriginal<Mtx34*>(output, pos, up, target);
  }
};
} // namespace feature