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
 * @file remote_avatars.cc
 * @brief Shows the other players as characters in the overworld.
 *
 * The declarations are in net/remote_avatars.h.
 */

#include "net/remote_avatars.h"

#include <cmath>
#include <cstring>

#include "core/native/data_manager.h"
#include "core/native/process_manager.h"
#include "overworld/constant/action_command.h"
#include "overworld/constant/model.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/patch/healer_follower.h"
#include "overworld/patch/map_character.h"
#include "renderer/address.h"
#include "script/patch/native_script.h"
#include "system/native/file.h"

static const c16* kFlagsFilename =
    u"sdmc:/luma/plugins/000400000011C500/net_flags.txt";

namespace net {
namespace {
using overworld::ActionCommand;
using overworld::MapCharacter;
using overworld::MapCharacterRequest;
using overworld::MapManager;
using overworld::Model;
using overworld::ModelManager;

constexpr f32 kTwoPi = 6.28318530f;

ModelId ModelFor(u32 index) {
  static const ModelId kOutfits[wire::kOutfitCount] = {
      ModelId::kWally, ModelId::kStevenStone, ModelId::kRoxanne,
      ModelId::kMaxie};
  return kOutfits[index % wire::kOutfitCount];
}

f32 Clamp(f32 value, f32 low, f32 high) {
  return value < low ? low : (value > high ? high : value);
}
} // namespace

ScriptId RemoteAvatars::ScriptFor(u32 index) {
  return static_cast<ScriptId>(static_cast<u32>(ScriptId::kRemotePlayer0) +
                               index);
}

void RemoteAvatars::Initialize() {
  auto& self = GetInstance();
  c8 flags[128] = {};
  if (sys::File::ReadAll(kFlagsFilename, flags, sizeof(flags) - 1) > 0) {
    self.is_pool_enabled = strstr(flags, "noavatars") == nullptr;
    self.is_local_outfit_enabled = strstr(flags, "nooutfit") == nullptr;
  }
  if (!self.is_pool_enabled) return;

  script::NativeScript::Register(ScriptId::kRemotePlayer0, Talk<0>);
  script::NativeScript::Register(ScriptId::kRemotePlayer1, Talk<1>);
  script::NativeScript::Register(ScriptId::kRemotePlayer2, Talk<2>);
  script::NativeScript::Register(ScriptId::kRemotePlayer3, Talk<3>);

  for (u32 i = 0; i < kPoolSize; i++) {
    MapCharacterRequest request;
    request.model_id = ModelFor(i);
    request.script_id = ScriptFor(i);
    request.is_everywhere = true;
    request.facing = Facing::kDown;
    MapCharacter::Add(request);
  }
}

template <u32 Index>
void RemoteAvatars::Talk(script::Context& script) {
  TalkTo(script, Index);
}

void RemoteAvatars::TalkTo(script::Context& script, u32 index) {
  const Avatar& avatar = GetInstance().avatars_[index];
  const RemotePlayer* remote = FindRemote(
      avatar.slot, (u16) static_cast<u32>(MapManager::GetInstance().GetMap()));

  static const c16 kPrefix[] = u"Hi, I'm ";
  c16 text[64];
  u32 length = 0;
  for (; kPrefix[length] != 0; length++) text[length] = kPrefix[length];
  if (remote != nullptr) {
    for (u32 i = 0; remote->name[i] != 0 && length < 60; i++) {
      text[length++] = (c16)(u8)remote->name[i];
    }
  }
  text[length++] = u'!';
  text[length] = 0;

  script.TalkStart();
  script.Talk(text);
  script.TalkEnd();
}

Model* RemoteAvatars::FindModel(u32 index) {
  Avatar& avatar = GetInstance().avatars_[index];
  avatar.local_id = MapCharacter::GetLocalId(ScriptFor(index));
  if (avatar.local_id == 0xFFFF) return nullptr;

  auto& models = ModelManager::GetInstance();
  auto& player = models.GetPlayer();
  for (u32 i = 0; i < ModelManager::kMaxModels; i++) {
    Model& model = models.GetModel(i);
    if (&model == &player || !model.IsUsed()) continue;
    if (model.id == avatar.local_id && model.model_id == ModelFor(index)) {
      return &model;
    }
  }
  return nullptr;
}

const RemotePlayer* RemoteAvatars::FindRemote(u16 slot, u16 zone) {
  if (slot == 0) return nullptr;
  const RemotePlayer* remotes = Network::GetInstance().GetRemotePlayers();
  for (u32 i = 0; i < Network::kMaxRemotePlayers; i++) {
    if (remotes[i].is_active && remotes[i].slot == slot &&
        remotes[i].zone == zone) {
      return &remotes[i];
    }
  }
  return nullptr;
}

void RemoteAvatars::Park(Model& model) {
  overworld::DrawModel* draw_model = model.GetDrawModelOrNull();
  if (draw_model == nullptr) return;
  model.draw_pos.y = kParkedHeight;
  draw_model->position = model.draw_pos;
}

void RemoteAvatars::Play(Model& model, Avatar& avatar, Pose pose,
                         Facing facing) {
  if (avatar.pose == pose && avatar.facing == facing) {
    if (pose != Pose::kWalking || ++avatar.animation_age < kWalkLoopFrames) {
      return;
    }
  }
  avatar.pose = pose;
  avatar.facing = facing;
  avatar.animation_age = 0;
  const ActionCommand action = pose == Pose::kWalking
                                   ? ActionCommand::kWalkInPlace8Frames
                                   : ActionCommand::kFace;
  ((u32 (*)(Model*, u32, u32))renderer::address::kModelPlayAnimation)(
      &model, static_cast<u32>(action), static_cast<u32>(facing));
}

void RemoteAvatars::Drive(Model& model, Avatar& avatar,
                          const RemotePlayer& remote) {
  auto& player = ModelManager::GetInstance().GetPlayer();
  overworld::DrawModel* draw_model = model.GetDrawModelOrNull();
  if (draw_model == nullptr) return;

  const f32 target_x = remote.position.x;
  const f32 target_z = remote.position.z;
  const f32 target_y = remote.position.y;

  f32 step_x = target_x - avatar.draw_x;
  f32 step_z = target_z - avatar.draw_z;
  const bool must_snap = !avatar.is_placed || (remote.flags & wire::kPoseSnap) ||
      std::sqrt(step_x * step_x + step_z * step_z) > kSnapDistance;
  if (must_snap) {
    avatar.draw_x = target_x;
    avatar.draw_z = target_z;
    avatar.is_placed = true;
    avatar.pose = Pose::kUnknown;
    avatar.still_frames = 0;
    step_x = 0.0f;
    step_z = 0.0f;
  } else {
    step_x *= kSmoothing;
    step_z *= kSmoothing;
    avatar.draw_x += step_x;
    avatar.draw_z += step_z;
  }
  const f32 moved = std::sqrt(step_x * step_x + step_z * step_z);

  Pose pose = avatar.pose == Pose::kWalking ? Pose::kWalking : Pose::kIdle;
  if (moved > kMovingDistance) {
    avatar.still_frames = 0;
    pose = Pose::kWalking;
  } else if (++avatar.still_frames >= kStillFramesBeforeIdle) {
    pose = Pose::kIdle;
  }

  f32 look_x = step_x;
  f32 look_z = step_z;
  if (moved <= kMovingDistance) {
    const f32 angle = (remote.heading / 256.0f - 0.5f) * kTwoPi;
    look_x = std::sin(angle);
    look_z = std::cos(angle);
  }
  const Facing facing =
      overworld::HealerFollower::ToFacing(look_x, look_z, avatar.facing);

  model.draw_pos.x = avatar.draw_x;
  model.draw_pos.z = avatar.draw_z;
  model.draw_pos.y = Clamp(target_y, player.draw_pos.y - 60.0f,
                           player.draw_pos.y + 60.0f);
  model.world_pos.coords.x = player.world_pos.coords.x +
      (avatar.draw_x - player.draw_pos.x) * kWorldPerDraw;
  model.world_pos.coords.z = player.world_pos.coords.z +
      (avatar.draw_z - player.draw_pos.z) * kWorldPerDraw;
  model.world_pos.coords.y = player.world_pos.coords.y;
  model.map_pos.coords.x = std::floor(avatar.draw_x / kDrawPerTile);
  model.map_pos.coords.z = std::floor(avatar.draw_z / kDrawPerTile);
  model.map_pos.coords.y = player.map_pos.coords.y;
  draw_model->position = model.draw_pos;

  Play(model, avatar, pose, facing);
}

bool RemoteAvatars::ApplyLocalOutfit() {
  auto& self = GetInstance();
  if (!self.is_local_outfit_enabled) return false;
  auto& network = Network::GetInstance();
  if (!network.IsConnected()) return false;

  const s32 outfit = network.GetLocalOutfit() % wire::kOutfitCount;
  if (outfit == self.applied_outfit_) return false;

  auto& models = ModelManager::GetInstance();
  auto& player = models.GetPlayer();
  const ModelId wanted = ModelFor((u32)outfit);
  if (player.model_id == wanted) {
    self.applied_outfit_ = outfit;
    return false;
  }

  const u32 count = models.resource_count_ < ModelManager::kMaxModels
                        ? models.resource_count_
                        : ModelManager::kMaxModels;
  for (u32 i = 0; i < count; i++) {
    overworld::ModelResource& resource = models.GetResource(i);
    if (resource.model_id != player.model_id) continue;
    resource.model_id = wanted;
    resource.uses_outfit = 0; // outfit parts only exist on the trainers
    break;
  }
  self.applied_outfit_ = outfit;

  Facing facing = core::DataManager::GetInstance().GetPlayerDirection();
  if (facing >= Facing::kCount) facing = Facing::kUp;
  MapManager::ChangeMap(MapManager::GetInstance().GetMap(), player.world_pos,
                        facing, true, false);
  return true;
}

void RemoteAvatars::Assign(u16 zone) {
  auto& self = GetInstance();

  for (auto& avatar : self.avatars_) {
    if (avatar.slot != 0 && FindRemote(avatar.slot, zone) == nullptr) {
      avatar.slot = 0;
      avatar.is_placed = false;
    }
  }

  const RemotePlayer* remotes = Network::GetInstance().GetRemotePlayers();
  for (u32 i = 0; i < Network::kMaxRemotePlayers; i++) {
    const RemotePlayer& remote = remotes[i];
    if (!remote.is_active || remote.zone != zone) continue;

    Avatar& avatar = self.avatars_[remote.outfit % kPoolSize];
    if (avatar.slot == 0) {
      avatar.slot = remote.slot;
      avatar.is_placed = false;
    }
  }
}

void RemoteAvatars::Update() {
  auto& self = GetInstance();
  if (!self.is_enabled) return;

  auto& maps = MapManager::GetInstance();
  if (!core::ProcessManager::IsOverworldActive() ||
      ModelManager::GetInstanceOrNull() == nullptr ||
      maps.GetNextMapId() != MapManager::kNoMap) {
    for (auto& avatar : self.avatars_) avatar.is_placed = false;
    self.settle_frames_ = kSettleFrames;
    return;
  }
  auto* engine = script::Engine::GetInstance();
  if (engine != nullptr && engine->IsScriptRunning()) return;

  const u16 zone = (u16) static_cast<u32>(maps.GetMap());
  if (zone != self.last_zone_) {
    self.last_zone_ = zone;
    self.settle_frames_ = kSettleFrames;
    for (auto& avatar : self.avatars_) avatar.is_placed = false;
  }
  if (self.settle_frames_ > 0) {
    --self.settle_frames_;
    return;
  }
  if (ApplyLocalOutfit()) return;
  Assign(zone);

  for (u32 i = 0; i < kPoolSize; i++) {
    Model* model = FindModel(i);
    if (model == nullptr) {
      self.avatars_[i].is_placed = false;
      continue;
    }
    const RemotePlayer* remote = FindRemote(self.avatars_[i].slot, zone);
    if (remote == nullptr) {
      Park(*model);
    } else {
      Drive(*model, self.avatars_[i], *remote);
    }
  }
}

} // namespace net
