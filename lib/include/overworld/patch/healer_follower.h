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
#include "overworld/native/model.h"
#include "script/patch/context.h"

namespace overworld {

class HealerFollower {
  MAKE_SINGLETON(HealerFollower)

public:
  bool is_enabled = true;

  static void Initialize();
  static void Update();
  static Facing ToFacing(f32 dx, f32 dz, Facing current);

private:
  static constexpr f32 kFarDistance = 18.0f;
  static constexpr f32 kNearDistance = 16.0f;
  static constexpr f32 kMovingDistance = 0.2f;
  static constexpr u32 kStillFramesBeforeIdle = 4;
  static constexpr u32 kFramesBeforeTurn = 3;
  static constexpr u32 kWalkLoopFrames = 8;
  static constexpr f32 kDiagonalRatio = 2.4f;
  static constexpr f32 kDrawPerTile = 18.0f;
  static constexpr f32 kWorldPerDraw = 1.0f / 9.0f;

  enum class State : u8 { kUnknown, kIdle, kWalking };

  static void Talk(script::Context& script);
  static Model* FindModel();
  static void Follow(Model& nurse);
  static void Play(Model& nurse, State state, Facing facing);

  u16 local_id_ = 0xFFFF;
  u32 last_map_ = 0xFFFFFFFF;
  bool is_placed_ = false;
  f32 nurse_x_ = 0.0f;
  f32 nurse_z_ = 0.0f;
  u32 still_frames_ = 0;
  u32 facing_frames_ = 0;
  u32 animation_age_ = 0;
  State state_ = State::kUnknown;
  Facing facing_ = Facing::kDown;
};
} // namespace overworld
