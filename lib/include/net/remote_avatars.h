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

#include "common.h"
#include "net/network.h"
#include "overworld/constant/facing.h"
#include "overworld/native/model.h"
#include "script/patch/context.h"

namespace net {

using Facing = overworld::Facing;

class RemoteAvatars {
  MAKE_SINGLETON(RemoteAvatars)

public:
  static constexpr u32 kPoolSize = 4;

  bool is_enabled = true;

  bool is_pool_enabled = true;
  bool is_local_outfit_enabled = true;

  static void Initialize();
  static void Update();

private:
  enum class Pose : u8 { kUnknown, kIdle, kWalking };

  struct Avatar {
    u16 slot;
    u16 local_id;
    bool is_placed;
    Pose pose;
    Facing facing;
    u32 animation_age;
    u32 still_frames;
    f32 draw_x, draw_z;
  };

  static constexpr f32 kWorldPerDraw = 1.0f / 9.0f;
  static constexpr f32 kDrawPerTile = 18.0f;
  static constexpr f32 kSmoothing = 0.3f;
  static constexpr f32 kSnapDistance = 200.0f;
  static constexpr f32 kMovingDistance = 0.2f;
  static constexpr u32 kStillFramesBeforeIdle = 4;
  static constexpr u32 kWalkLoopFrames = 8;
  static constexpr f32 kParkedHeight = -10000.0f;
  static constexpr u32 kSettleFrames = 45;

  template <u32 Index>
  static void Talk(script::Context& script);
  static void TalkTo(script::Context& script, u32 index);

  static ScriptId ScriptFor(u32 index);
  static overworld::Model* FindModel(u32 index);
  static void Park(overworld::Model& model);
  static void Drive(overworld::Model& model, Avatar& avatar,
                    const RemotePlayer& remote);
  static void Play(overworld::Model& model, Avatar& avatar, Pose pose,
                   Facing facing);
  static void Assign(u16 zone);
  static bool ApplyLocalOutfit();
  static const RemotePlayer* FindRemote(u16 slot, u16 zone);

  Avatar avatars_[kPoolSize] = {};
  u16 last_zone_ = 0xFFFF;
  u32 settle_frames_ = 0;
  s32 applied_outfit_ = -1;
};

} // namespace net
