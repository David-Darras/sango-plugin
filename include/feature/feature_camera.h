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
  enum CameraState { kIdle, kFree, kRotate, kFpv, kTps };

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
  f32 theta_speed = 0.01f;
  bool is_updating_camera = false;
  f32 tps_dist = 80.0f;
  f32 tps_height = 33.0f;
  f32 tps_offset = 0.0f;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kUpdateMatrices,
                            ADDRESS_STEREO_CAMERA_UPDATE_MATRICES,
                            (uptr)UpdateMatricesHook);
    HookManager::Initialize(HookID::kUpdateLookAt,
                            ADDRESS_STEREO_CAMERA_UPDATE_LOOK_AT,
                            (uptr)UpdateLookAtHook);
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

    auto& player = overworld::ModelManager::GetInstance().GetPlayer();

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

      case CameraState::kFpv:
        pos->x = player.draw_pos.x;
        pos->y = player.draw_pos.y + 30.0f;
        pos->z = player.draw_pos.z;

        *target = {
            pos->x + player.facing_direction.x,
            pos->y + player.facing_direction.y,
            pos->z + player.facing_direction.z
        };
        *up = {0.0f, 1.0f, 0.0f};
        break;

      case CameraState::kTps: {
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

      case CameraState::kIdle:
      default:
        break;
    }

    return HookManager::GetInstance()
           .Get(HookID::kUpdateLookAt)
           ->CallOriginal<Mtx34*>(output, pos, up, target);
  }
};
} // namespace feature