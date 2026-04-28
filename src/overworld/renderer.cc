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
#include "menu/log_menu.h"
#include "menu/plugin_menu.h"
#include "overworld/model_manager.h"

namespace overworld {

enum class CameraState { kIdle, kFree, kRotate, kFpv, kTps };

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
  f32 tps_dist = 80.0f;
  f32 tps_height = 33.0f;
  f32 tps_offset = 0.0f;

  f32 outline_scale = 1.0f;
  Color8 outline_color = Color8{0, 0, 0, 255};
  Color8 ambient_light_color = Color8{255, 255, 255, 255};
  Color8 diffuse_light_color = Color8{255, 255, 255, 255};
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
          player.draw_pos.z - (dir.z * ctx.tps_dist) + (dir.x * ctx.tps_offset);

      *target = {player.draw_pos.x, player.draw_pos.y + ctx.tps_height,
                 player.draw_pos.z};
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

void StereoCamera::LoadMenu(menu::PluginMenu& menu, void* args) {
  static const c8* STATES[] = {"Idle", "Free", "Rotate", "Fpv", "Tps"};
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
      .WithFactor(0.01f)
      .Add("TPS Distance", ctx.tps_dist)
      .Add("TPS Height", ctx.tps_height)
      .Add("TPS Shoulder Offset", ctx.tps_offset);
}

void StereoCamera::SetupHooks() {
  HookManager& man = HookManager::GetInstance();
  man.Add(HookID::kUpdateMatrices, ADDRESS_STEREO_CAMERA_UPDATE_MATRICES,
          (uptr)UpdateMatrices);
  man.Add(HookID::kUpdateLookAt, ADDRESS_STEREO_CAMERA_UPDATE_LOOK_AT,
          (uptr)UpdateLookAt);
}

void OnChangeOutlineScale(void* outline_manager, f32 screen_width,
                          f32 screen_height, f32 scale) {
  const f32 r = (f32)ctx.outline_color.r / 255.0f;
  const f32 g = (f32)ctx.outline_color.g / 255.0f;
  const f32 b = (f32)ctx.outline_color.b / 255.0f;
  const f32 a = (f32)ctx.outline_color.a / 255.0f;

  ((void (*)(void*, f32, f32, f32, f32))0x00379C7C)(outline_manager, r, g, b,
                                                    a);

  HookManager::GetInstance()
      .Get(HookID::kOnChangeOutlineScale)
      ->CallOriginal<void>(outline_manager, screen_width, screen_height,
                           ctx.outline_scale);
}

void OnChangeAmbientLightColor(void* light_manager, f32* colors) {
  colors[0] = (f32)ctx.ambient_light_color.r / 255.0f;
  colors[1] = (f32)ctx.ambient_light_color.g / 255.0f;
  colors[2] = (f32)ctx.ambient_light_color.b / 255.0f;
  colors[3] = (f32)ctx.ambient_light_color.a / 255.0f;

  HookManager::GetInstance()
      .Get(HookID::kOnChangeAmbientLightColor)
      ->CallOriginal<void>(light_manager, colors);
}

void OnChangeDiffuseLightColor(void* light_manager, f32* colors) {
  colors[0] = (f32)ctx.diffuse_light_color.r / 255.0f;
  colors[1] = (f32)ctx.diffuse_light_color.g / 255.0f;
  colors[2] = (f32)ctx.diffuse_light_color.b / 255.0f;
  colors[3] = (f32)ctx.diffuse_light_color.a / 255.0f;

  HookManager::GetInstance()
      .Get(HookID::kOnChangeDiffuseLightColor)
      ->CallOriginal<void>(light_manager, colors);
}

void Renderer::LoadMenu(menu::PluginMenu& menu, void* args) {
  HookManager& man = HookManager::GetInstance();
  man.Add(HookID::kOnChangeOutlineScale, 0x00379E3C,
          (uptr)OnChangeOutlineScale);
  man.Add(HookID::kOnChangeAmbientLightColor, 0x00139710,
          (uptr)OnChangeAmbientLightColor);
  man.Add(HookID::kOnChangeDiffuseLightColor, 0x001397C0,
          (uptr)OnChangeDiffuseLightColor);

  menu.Add("Outline Scale", ctx.outline_scale)
      .Add("Outline Color - Red", ctx.outline_color.r)
      .Add("Outline Color - Green", ctx.outline_color.g)
      .Add("Outline Color - Blue", ctx.outline_color.b)
      .Add("Outline Color - Alpha", ctx.outline_color.a)
      .Add("Ambient Light - Red", ctx.ambient_light_color.r)
      .Add("Ambient Light - Green", ctx.ambient_light_color.g)
      .Add("Ambient Light - Blue", ctx.ambient_light_color.b)
      .Add("Ambient Light - Alpha", ctx.ambient_light_color.a)
      .Add("Diffuse Light - Red", ctx.diffuse_light_color.r)
      .Add("Diffuse Light - Green", ctx.diffuse_light_color.g)
      .Add("Diffuse Light - Blue", ctx.diffuse_light_color.b)
      .Add("Diffuse Light - Alpha", ctx.diffuse_light_color.a);
}

}  // namespace overworld