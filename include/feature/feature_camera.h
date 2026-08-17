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
#include "game/process_manager.h"
#include "game/overworld/model_manager.h"
#include "game/overworld/renderer.h"
#include "game/battle/manager.h"

namespace overworld {
class StereoCamera;
}

namespace feature {
// Shared "look-at" camera used both by the overworld and by battles: the
// engine funnels both through the same ADDRESS_STEREO_CAMERA_UPDATE_* pair,
// so a single hook handles both contexts. Overworld and battle each keep
// their own independent mode/target so switching one doesn't disturb the
// other; `active_context` records which one is live for the current frame,
// set by UpdateMatricesHook and consumed by UpdateLookAtHook right after.
struct Camera {
  enum State { kIdle, kTps, kRotate, kTop, kFpv, kFree };

  enum class Context { kNone, kOverworld, kBattle };

  void SetCameraIdle(bool is_battle) {
    (is_battle ? battle_state : overworld_state) = State::kIdle;
  }

  void SetCameraFree(bool is_battle, f32 x, f32 y, f32 z, f32 yaw,
                     f32 pitch) {
    u32& state_ref = is_battle ? battle_state : overworld_state;
    u8& old_state_ref = is_battle ? battle_old_state : overworld_old_state;
    state_ref = State::kFree;
    pos.x = x;
    pos.y = y;
    pos.z = z;
    rot.y = yaw;
    rot.x = pitch;
    old_state_ref = (u8)state_ref;
  }

  void SetCameraRotate(bool is_battle, f32 r, f32 h, f32 w = 0.0f) {
    (is_battle ? battle_state : overworld_state) = State::kRotate;
    radius = r;
    height = h;
    theta = w;
  }

  void SetCameraTPS(bool is_battle, f32 dist, f32 height, f32 offset) {
    (is_battle ? battle_state : overworld_state) = State::kTps;
    tps_dist = dist;
    tps_height = height;
    tps_offset = offset;
  }

  MAKE_SINGLETON(Camera)
  u32 overworld_state = kIdle;
  u8 overworld_old_state = 0;
  u32 battle_state = kIdle;
  u8 battle_old_state = 0;
  Context active_context = Context::kNone;

  // Which of the 6 on-field Pokemon model slots (see
  // battle::Graphics::GetPokemonModel) the battle camera targets for
  // kRotate/kTop/kFpv/kTps.
  u8 battle_target_pokemon_slot = 0;

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

  static u32 UpdateMatricesHook(overworld::StereoCamera* stereo_camera,
                                bool update) {
    auto& ctx = GetInstance();
    auto& process_manager = game::ProcessManager::GetInstance();

    if (process_manager.IsCurrentProcess(ADDRESS_OVERWORLD_VTABLE)) {
      if (&overworld::Renderer::GetInstance().GetStereoCamera() ==
          stereo_camera) {
        ctx.is_updating_camera = true;
        ctx.active_context = Context::kOverworld;
      }
    } else if (process_manager.IsCurrentProcess(ADDRESS_BATTLE_VTABLE)) {
      ui::LogApplication::Print(u"Update battle camera");
//      if (&battle::Manager::GetInstance().GetGraphics().GetStereoCamera() ==
//          stereo_camera) {
        ctx.is_updating_camera = true;
        ctx.active_context = Context::kBattle;
//      }
    }

    return HookManager::Call<u32>(HookID::kUpdateMatrices, stereo_camera,
                                  update);
  }

  static Mtx34* UpdateLookAtHook(Mtx34* output, Vec3* pos, Vec3* up,
                                 Vec3* target) {
    auto& ctx = GetInstance();
    if (!ctx.is_updating_camera) {
      return HookManager::Call<Mtx34*>(HookID::kUpdateLookAt, output, pos, up,
                                       target);
    }
    ctx.is_updating_camera = false;

    bool is_battle = (ctx.active_context == Context::kBattle);
    u32& state = is_battle ? ctx.battle_state : ctx.overworld_state;
    u8& old_state = is_battle ? ctx.battle_old_state : ctx.overworld_old_state;

    if (state != old_state) {
      if (static_cast<State>(state) == State::kFree) {
        ctx.pos = *pos;
        f32 dx = target->x - pos->x;
        f32 dy = target->y - pos->y;
        f32 dz = target->z - pos->z;
        f32 dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        ctx.rot.y = std::atan2(dz, dx);
        ctx.rot.x = (dist > 0.0001f) ? std::asin(dy / dist) : 0.0f;
      }
      old_state = (u8)state;
    }

    // Resolve what "the subject" is for this frame: the overworld player
    // model, or the selected battle Pokemon model. kFree ignores both (it
    // only depends on ctx.pos/ctx.rot), so it behaves identically in both
    // contexts.
    Vec3 target_pos;
    Vec3 facing;
    if (is_battle) {
      auto& model = battle::Manager::GetInstance().GetGraphics().
                                                   GetPokemonModel(
                                                       ctx.
                                                       battle_target_pokemon_slot);
      target_pos = model.position;
      // BaseModel has no ready-made facing vector like the overworld
      // player does, so it's derived from its yaw (rotation.y). Only used
      // by kFpv/kTps; kRotate/kTop only need target_pos.
      facing = {std::sin(model.rotation.y), 0.0f, std::cos(model.rotation.y)};
    } else {
      auto& player = overworld::ModelManager::GetInstance().GetPlayer();
      target_pos = player.draw_pos;
      facing = player.facing_direction;
    }

    switch (static_cast<State>(state)) {
      case State::kRotate:
        *target = target_pos;
        *up = {0.0f, 1.0f, 0.0f};
        pos->x = target->x + ctx.radius * std::cos(ctx.theta);
        pos->z = target->z + ctx.radius * std::sin(ctx.theta);
        pos->y = target->y + ctx.height;
        ctx.theta += ctx.theta_speed;
        break;

      case State::kTop:
        *target = target_pos;
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

      case State::kTps: {
        Vec3 dir = facing;
        f32 len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
        if (len > 0.0f) {
          dir.x /= len;
          dir.z /= len;
        }

        pos->x = target_pos.x - (dir.x * ctx.tps_dist) +
                 (-dir.z * ctx.tps_offset);
        pos->y = target_pos.y + ctx.tps_height;
        pos->z =
            target_pos.z - (dir.z * ctx.tps_dist) + (
              dir.x * ctx.tps_offset);

        *target = {
            target_pos.x, target_pos.y + ctx.tps_height,
            target_pos.z
        };
        *up = {0.0f, 1.0f, 0.0f};
        break;
      }

      case State::kIdle:
      default:
        break;
    }

    return HookManager::Call<Mtx34*>(HookID::kUpdateLookAt, output, pos, up,
                                     target);
  }
};
} // namespace feature