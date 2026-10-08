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
 * @file healer_follower.cc
 * @brief A nurse that follows the player and heals the party.
 *
 * The declarations are in overworld/patch/healer_follower.h.
 */

#include "overworld/patch/healer_follower.h"

#include <cmath>

#include "core/native/process_manager.h"
#include "overworld/constant/action_command.h"
#include "overworld/constant/model.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/patch/map_character.h"
#include "renderer/address.h"
#include "savedata/native/pokemon_team.h"
#include "script/patch/native_script.h"

namespace overworld {
namespace {
constexpr ModelId kNurseModel = ModelId::kMomOras;

f32 Abs(f32 value) { return value < 0.0f ? -value : value; }
} // namespace

void HealerFollower::Initialize() {
  script::NativeScript::Register(ScriptId::kHealerFollower, Talk);

  MapCharacterRequest nurse;
  nurse.model_id = kNurseModel;
  nurse.script_id = ScriptId::kHealerFollower;
  nurse.is_everywhere = true;
  nurse.facing = Facing::kDown;
  nurse.is_enabled = &GetInstance().is_enabled;
  MapCharacter::Add(nurse);
}

void HealerFollower::Talk(script::Context& script) {
  script.TalkStart();
  script.Talk(u"Hello! Would you like me to heal your Pokemon?");
  if (script.AskYesNo()) {
    savedata::PokemonTeam::GetInstance().HealAllPokemons();
    script.PlayJingle(script::kJingleLevelUp);
    script.Talk(u"Your Pokemon are fully healed. See you soon!");
  } else {
    script.Talk(u"Take care!");
  }
  script.TalkEnd();
}

Model* HealerFollower::FindModel() {
  auto& ctx = GetInstance();
  ctx.local_id_ = MapCharacter::GetLocalId(ScriptId::kHealerFollower);
  if (ctx.local_id_ == 0xFFFF) return nullptr;

  auto& models = ModelManager::GetInstance();
  auto& player = models.GetPlayer();
  for (u32 i = 0; i < ModelManager::kMaxModels; i++) {
    Model& model = models.GetModel(i);
    if (&model == &player || !model.IsUsed()) continue;
    if (model.id == ctx.local_id_ && model.model_id == kNurseModel) {
      return &model;
    }
  }
  return nullptr;
}

Facing HealerFollower::ToFacing(f32 dx, f32 dz, Facing current) {
  const f32 ax = Abs(dx);
  const f32 az = Abs(dz);
  if (ax == 0.0f && az == 0.0f) return current;
  if (ax > az * kDiagonalRatio) return dx > 0.0f ? Facing::kRight : Facing::kLeft;
  if (az > ax * kDiagonalRatio) return dz > 0.0f ? Facing::kDown : Facing::kUp;
  if (dx > 0.0f) return dz > 0.0f ? Facing::kDownRight : Facing::kUpRight;
  return dz > 0.0f ? Facing::kDownLeft : Facing::kUpLeft;
}

void HealerFollower::Play(Model& nurse, State state, Facing facing) {
  auto& ctx = GetInstance();
  if (ctx.state_ == state && ctx.facing_ == facing) {
    if (state != State::kWalking || ++ctx.animation_age_ < kWalkLoopFrames) {
      return;
    }
  }
  ctx.state_ = state;
  ctx.facing_ = facing;
  ctx.animation_age_ = 0;
  const ActionCommand action = state == State::kWalking
                                 ? ActionCommand::kWalkInPlace8Frames
                                 : ActionCommand::kFace;
  ((u32 (*)(Model*, u32, u32))renderer::address::kModelPlayAnimation)(
      &nurse, static_cast<u32>(action), static_cast<u32>(facing));
}

void HealerFollower::Follow(Model& nurse) {
  auto& ctx = GetInstance();
  auto& player = ModelManager::GetInstance().GetPlayer();
  const f32 player_x = player.draw_pos.x;
  const f32 player_z = player.draw_pos.z;

  if (!ctx.is_placed_) {
    ctx.nurse_x_ = player_x - player.facing_direction.x * kFarDistance;
    ctx.nurse_z_ = player_z - player.facing_direction.z * kFarDistance;
    ctx.still_frames_ = 0;
    ctx.state_ = State::kUnknown;
    ctx.is_placed_ = true;
  }

  f32 away_x = ctx.nurse_x_ - player_x;
  f32 away_z = ctx.nurse_z_ - player_z;
  f32 distance = std::sqrt(away_x * away_x + away_z * away_z);
  if (distance < 0.001f) {
    away_x = 0.0f;
    away_z = 1.0f;
    distance = 1.0f;
  }
  f32 wanted_distance = distance;
  if (wanted_distance > kFarDistance) wanted_distance = kFarDistance;
  if (wanted_distance < kNearDistance) wanted_distance = kNearDistance;
  const f32 target_x = player_x + away_x / distance * wanted_distance;
  const f32 target_z = player_z + away_z / distance * wanted_distance;

  const f32 step_x = target_x - ctx.nurse_x_;
  const f32 step_z = target_z - ctx.nurse_z_;
  const f32 moved = std::sqrt(step_x * step_x + step_z * step_z);
  ctx.nurse_x_ = target_x;
  ctx.nurse_z_ = target_z;

  State state = ctx.state_ == State::kWalking ? State::kWalking : State::kIdle;
  if (moved > kMovingDistance) {
    ctx.still_frames_ = 0;
    state = State::kWalking;
  } else if (++ctx.still_frames_ >= kStillFramesBeforeIdle) {
    state = State::kIdle;
  }

  const f32 look_x = moved > kMovingDistance ? step_x : player_x - ctx.nurse_x_;
  const f32 look_z = moved > kMovingDistance ? step_z : player_z - ctx.nurse_z_;
  const Facing wanted = ToFacing(look_x, look_z, ctx.facing_);
  Facing facing = ctx.facing_;
  if (wanted == ctx.facing_) {
    ctx.facing_frames_ = 0;
  } else if (++ctx.facing_frames_ >= kFramesBeforeTurn ||
             ctx.state_ == State::kUnknown) {
    facing = wanted;
    ctx.facing_frames_ = 0;
  }

  nurse.draw_pos.x = ctx.nurse_x_;
  nurse.draw_pos.z = ctx.nurse_z_;
  nurse.draw_pos.y = player.draw_pos.y;
  nurse.world_pos.coords.x =
      player.world_pos.coords.x + (ctx.nurse_x_ - player_x) * kWorldPerDraw;
  nurse.world_pos.coords.z =
      player.world_pos.coords.z + (ctx.nurse_z_ - player_z) * kWorldPerDraw;
  nurse.world_pos.coords.y = player.world_pos.coords.y;
  nurse.map_pos.coords.x = std::floor(ctx.nurse_x_ / kDrawPerTile);
  nurse.map_pos.coords.z = std::floor(ctx.nurse_z_ / kDrawPerTile);
  nurse.map_pos.coords.y = player.map_pos.coords.y;
  nurse.GetDrawModel().position = nurse.draw_pos;

  Play(nurse, state, facing);
}

void HealerFollower::Update() {
  auto& ctx = GetInstance();
  if (!ctx.is_enabled) return;
  if (!core::ProcessManager::IsOverworldActive()) {
    ctx.is_placed_ = false;
    return;
  }
  if (MapManager::GetInstance().GetNextMapId() != MapManager::kNoMap) {
    ctx.is_placed_ = false;
    return;
  }
  auto* engine = script::Engine::GetInstance();
  if (engine != nullptr && engine->IsScriptRunning()) return;

  const u32 map = static_cast<u32>(MapManager::GetInstance().GetMap());
  if (map != ctx.last_map_) {
    ctx.last_map_ = map;
    ctx.is_placed_ = false;
  }

  Model* nurse = FindModel();
  if (nurse == nullptr) {
    ctx.is_placed_ = false;
    return;
  }
  Follow(*nurse);
}
} // namespace overworld
