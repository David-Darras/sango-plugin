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
 * @file player_cheats.cc
 * @brief The cheat codes of the player: Noclip and Swarm Mod.
 *
 * The declarations are in overworld/patch/player_cheats.h.
 */

#include "overworld/patch/player_cheats.h"
#include <cmath>
#include "core/cheat_code_manager.h"
#include "core/hook.h"
#include "core/native/process_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/patch/pokemon_flight.h"
#include "system/native/controller.h"
#include "overworld/address.h"

namespace overworld {

#ifdef GAME_ORAS
namespace {
// The hit bit of the end of the map: the player never leaves the map space.
constexpr u32 kHitEndOfMap = 1 << 4;

core::Hook<u32(void*, u32, void*, void*, void*, u32)> hit_check_grid_hook;
core::Hook<u32(void*, void*, void*, u32, void*, f32, void*, void*, void*, u32,
               void*, void*, void*)>
    hit_check_analog_hook;

// The commands of the game that move the player one tile in a number of
// frames, from the fastest to the slowest: 1, 2, 3, 4, 5, 6, 7, 8 and 12.
constexpr u32 kStepCommands[] = {16, 15, 21, 14, 22, 20, 19, 13, 17};
constexpr u32 kWalkCommand = 19; // The walk of the game: 7 frames.
// The number of frames of each step command, in the order of kStepCommands.
constexpr u32 kStepFrames[] = {1, 2, 3, 4, 5, 6, 7, 8, 12};
constexpr u32 kSitWaitCommand = 134; // The player sits and waits.
constexpr u32 kRunCommand = 14;  // The run of the game: 4 frames.

core::Hook<void(void*, u32, u32)> set_command_hook;

// The player asks the game for the command of the walk or the run: the plugin
// gives another command, which takes more or fewer frames for one tile.
void SetCommandHook(void* player, u32 command, u32 direction) {
  auto& cheats = PlayerCheats::GetInstance();
  // The player sits on a Pokémon in flight: the game cannot make it walk.
  if (PokemonFlight::IsFlying()) {
    // All the orders of the game become the sit order (a walk, a wait, a
    // turn), with the direction of the flight: the player stays seated, and it
    // turns with the flight.
    set_command_hook(player, kSitWaitCommand, PokemonFlight::GetSitDirection());
    return;
  }
  // The player rides a Pokémon on the ground: the game moves the player, and
  // the sit pose replaces the pose of the walk.
  if (PokemonFlight::IsRiding()) {
    bool is_step = false;
    for (u32 step_command : kStepCommands) {
      if (command == step_command) is_step = true;
    }
    if (is_step) {
      // The mount is faster: the step takes fewer frames.
      f32 speed = PokemonFlight::GetInstance().ride_speed;
      if (speed < 1.0f) speed = 1.0f;
      u32 frames = 7;
      for (u32 i = 0; i < SIZE(kStepCommands); i++) {
        if (kStepCommands[i] == command) frames = kStepFrames[i];
      }
      const f32 wanted = static_cast<f32>(frames) / speed;
      u32 best = 0;
      f32 best_distance = 1000.0f;
      for (u32 i = 0; i < SIZE(kStepFrames); i++) {
        const f32 distance = std::fabs(static_cast<f32>(kStepFrames[i]) - wanted);
        if (distance < best_distance) {
          best_distance = distance;
          best = i;
        }
      }
      set_command_hook(player, kStepCommands[best], direction);
    } else {
      set_command_hook(player, kSitWaitCommand, direction);
    }
    return;
  }
  if (command == kWalkCommand && cheats.walk_frames < SIZE(kStepCommands)) {
    command = kStepCommands[cheats.walk_frames];
  } else if (command == kRunCommand && cheats.run_frames < SIZE(kStepCommands)) {
    command = kStepCommands[cheats.run_frames];
  }
  set_command_hook(player, command, direction);
}

u32 FilterHit(u32 hit) {
  return PlayerCheats::GetInstance().no_collision ? (hit & kHitEndOfMap) : hit;
}

u32 HitCheckGridHook(void* player, u32 dir, void* attr, void* next_pos,
                     void* hit_model, u32 check_slice) {
  return FilterHit(
      hit_check_grid_hook(player, dir, attr, next_pos, hit_model, check_slice));
}

u32 HitCheckAnalogHook(void* player, void* map, void* model, u32 dir,
                       void* way, f32 distance, void* attr, void* next_pos,
                       void* limit, u32 skipped_bits, void* slice_dir,
                       void* next_check, void* forced_way) {
  return FilterHit(hit_check_analog_hook(player, map, model, dir, way, distance,
                                         attr, next_pos, limit, skipped_bits,
                                         slice_dir, next_check, forced_way));
}
} // namespace
#endif

void PlayerCheats::Initialize() {
#ifdef GAME_ORAS
  hit_check_grid_hook.Install(address::kPlayerHitCheckGrid, HitCheckGridHook,
                              address::kVtable);
  hit_check_analog_hook.Install(address::kPlayerHitCheckAnalog,
                                HitCheckAnalogHook, address::kVtable);
  set_command_hook.Install(address::kPlayerSetCommand, SetCommandHook,
                           address::kVtable);
#endif
  core::CheatCodeManager::Initialize(CheatCodeId::kSwarmMod, SwarmMod, nullptr,
                               true);
  core::CheatCodeManager::Initialize(CheatCodeId::kNoclip, Noclip, nullptr, true);
}

void PlayerCheats::SwarmMod() {
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(
      address::kVtable))
    return;
  auto& ctx = GetInstance();

  auto& man = ModelManager::GetInstance();
  auto& player = man.GetPlayer();

  u32 npc_count = 0;
  for (u32 i = 0; i < ModelManager::kMaxModels; i++) {
    auto& model = man.GetModel(i);
    if (model.IsUsed() && &model != &player) {
      npc_count++;
    }
  }

  for (u32 i = 0; i < ModelManager::kMaxModels; i++) {
    auto& model = man.GetModel(i);
    if (!(model.IsUsed() && &model != &player)) {
      continue;
    }

    f32 phi = ctx.theta + (f32)i * 2.0f * (f32)M_PI / (f32)npc_count;

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

void PlayerCheats::Noclip() {
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(
      address::kVtable))
    return;

  auto& ctx = GetInstance();
  auto& player = ModelManager::GetInstance().GetPlayer();
  // The drawn model does not exist while the map loads.
  DrawModel* draw_model = player.GetDrawModelOrNull();
  if (draw_model == nullptr) return;
  auto& controller = sys::Controller::GetInstance();

#define ADD_MOVEMENT(key, composant, val)                                    \
  if (controller.IsKeyDown(Key::key)) {                                      \
    player.map_pos.coords.composant += (val) * ctx.speed.composant;          \
    player.world_pos.coords.composant += (val) * 9.0f * ctx.speed.composant; \
    player.draw_pos.composant += (val) * 9.0f * ctx.speed.composant;         \
    draw_model->position.composant += (val) * 9.0f * ctx.speed.composant;    \
  }

  ADD_MOVEMENT(kLeft, x, -1)
  ADD_MOVEMENT(kRight, x, 1)
  ADD_MOVEMENT(kL, y, -1)
  ADD_MOVEMENT(kR, y, 1)
  ADD_MOVEMENT(kUp, z, -1)
  ADD_MOVEMENT(kDown, z, 1)

#undef ADD_MOVEMENT
}

} // namespace overworld
