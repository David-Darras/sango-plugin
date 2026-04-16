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

#include "overworld/model_manager.h"

#include <math.h>

#include "menu/log_menu.h"
#include "menu/plugin_menu.h"
#include "system/device.h"

namespace overworld {

static struct {
  Vec3 speed;
  f32 theta;
  f32 radius;
  f32 theta_speed;
  u32 model_idx;
  u16 model_animation;
} ctx = {{1, 1, 1}, 0, 5, 1, 0};

void ModelManager::Noclip(void*) {
  Model& player = GetInstance().GetPlayer();
  Controller& controller = Controller::GetInstance();

#define ADD_MOVEMENT(key, composant, val)                                    \
  if (controller.IsKeyDown(Key::key)) {                                      \
    player.map_pos.coords.composant += (val) * ctx.speed.composant;          \
    player.world_pos.coords.composant += (val) * 9.0f * ctx.speed.composant; \
    player.draw_pos.composant += (val) * 9.0f * ctx.speed.composant;         \
  }

  ADD_MOVEMENT(kLeft, x, -1)
  ADD_MOVEMENT(kRight, x, 1)
  ADD_MOVEMENT(kL, y, -1)
  ADD_MOVEMENT(kR, y, 1)
  ADD_MOVEMENT(kUp, z, -1)
  ADD_MOVEMENT(kDown, z, 1)

#undef ADD_MOVEMENT
}

void ModelManager::SwarmMod(void*) {
  ModelManager& man = GetInstance();
  Model& player = man.GetPlayer();

  u32 npc_count = 0;
  for (u32 i = 0; i < kMaxModels; i++) {
    Model& model = man.GetModel(i);
    if (model.IsUsed() && &model != &player) {
      npc_count++;
    }
  }

  for (u32 i = 0; i < kMaxModels; i++) {
    Model& model = man.GetModel(i);
    if (!(model.IsUsed() && &model != &player)) {
      continue;
    }

    float phi = ctx.theta + (f32)i * 2.0 * M_PI / (f32)npc_count;

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

void PlayAnimation(void*) {
  menu::LogMenu::GetInstance().Add(u"Play");
  ((void (*)(Model*, u16, u8))ADDRESS_MODEL_PLAY_ANIMATION)(
      &ModelManager::GetInstance().GetPlayer(), ctx.model_animation, 0);
}

void ModelManager::LoadMenu(menu::PluginMenu& menu, void* args) {
  static const char* sep = "--------------------";

  ModelManager& man = GetInstance();
  ModelResource& rsrc = man.GetResource(ctx.model_idx);
  menu.Add(sep)
      .Add("Noclip", CheatCodeId::kNoclip)
      .Add("Speed-X", ctx.speed.x)
      .Add("Speed-Y", ctx.speed.y)
      .Add("Speed-Z", ctx.speed.z)
      .Add(sep)
      .Add("Swarm Mod", CheatCodeId::kSwarmMod)
      .Add("Circle Radius", ctx.radius)
      .Add("Rotation Speed", ctx.theta_speed)
      .Add(sep)
      .Add("Model Index", ctx.model_idx)
      .WithBounds(0, kMaxModels - 1)
      .WithRefresh()
      .Add("Model", rsrc.model_id)
      .Add("Animation", ctx.model_animation)
      .WithCallback(PlayAnimation);
}

}  // namespace overworld
