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
#include <math.h>

#include "cheat_code_manager.h"
#include "common.h"
#include "game/process_manager.h"
#include "game/overworld/model_manager.h"
#include "system/device.h"

class Controller;

namespace overworld {
class ModelManager;
struct Model;
}

namespace feature {
struct OverworldModel {
  MAKE_SINGLETON(OverworldModel)
  Vec3 speed = Vec3{1, 1, 1};
  f32 theta = 0.0f;
  f32 radius = 5.0f;
  f32 theta_speed = 1.0f;
  u32 model_idx = 0;
  u16 model_animation = 0;

  STATIC_INLINE void Initialize() {
    CheatCodeManager::Initialize(CheatCodeId::kSwarmMod, SwarmMod, nullptr,
                                 true);
    CheatCodeManager::Initialize(CheatCodeId::kNoclip, Noclip, nullptr, true);
  }

  static void PlayAnimation(void*) {
    if (!game::ProcessManager::GetInstance().IsCurrentProcess(
        ADDRESS_OVERWORLD_VTABLE))
      return;

    auto& ctx = GetInstance();

    ((void (*)(overworld::Model*, u16, u8))ADDRESS_MODEL_PLAY_ANIMATION)(
        &overworld::ModelManager::GetInstance().GetPlayer(),
        ctx.model_animation, 3);
  }

  static void SwarmMod() {
    if (!game::ProcessManager::GetInstance().IsCurrentProcess(
        ADDRESS_OVERWORLD_VTABLE))
      return;
    auto& ctx = GetInstance();

    auto& man = overworld::ModelManager::GetInstance();
    auto& player = man.GetPlayer();

    u32 npc_count = 0;
    for (u32 i = 0; i < overworld::ModelManager::kMaxModels; i++) {
      auto& model = man.GetModel(i);
      if (model.IsUsed() && &model != &player) {
        npc_count++;
      }
    }

    for (u32 i = 0; i < overworld::ModelManager::kMaxModels; i++) {
      auto& model = man.GetModel(i);
      if (!(model.IsUsed() && &model != &player)) {
        continue;
      }

      f32 phi = ctx.theta + (f32)i * 2.0 * M_PI / (f32)npc_count;

      model.world_pos.coords.x =
          player.world_pos.coords.x + ctx.radius * cosf(phi);
      model.world_pos.coords.z =
          player.world_pos.coords.z + ctx.radius * sinf(phi);
      model.world_pos.coords.y = player.world_pos.coords.y;

      model.draw_pos.x = player.draw_pos.x + 9 * ctx.radius * cosf(phi);
      model.draw_pos.z = player.draw_pos.z + 9 * ctx.radius * sinf(phi);
      model.draw_pos.y = player.draw_pos.y;

      ctx.theta += 0.01f * ctx.theta_speed;
    }
  }

  static void Noclip() {
    if (!game::ProcessManager::GetInstance().IsCurrentProcess(
        ADDRESS_OVERWORLD_VTABLE))
      return;

    auto& ctx = GetInstance();
    auto& player = overworld::ModelManager::GetInstance().GetPlayer();
    auto& draw_model = player.GetDrawModel();
    auto& controller = Controller::GetInstance();

#define ADD_MOVEMENT(key, composant, val)                                    \
if (controller.IsKeyDown(Key::key)) {                                      \
player.map_pos.coords.composant += (val) * ctx.speed.composant;          \
player.world_pos.coords.composant += (val) * 9.0f * ctx.speed.composant; \
player.draw_pos.composant += (val) * 9.0f * ctx.speed.composant;         \
draw_model.position.composant += (val) * 9.0f * ctx.speed.composant;         \
}

    ADD_MOVEMENT(kLeft, x, -1)
    ADD_MOVEMENT(kRight, x, 1)
    ADD_MOVEMENT(kL, y, -1)
    ADD_MOVEMENT(kR, y, 1)
    ADD_MOVEMENT(kUp, z, -1)
    ADD_MOVEMENT(kDown, z, 1)

#undef ADD_MOVEMENT
  }
};
}