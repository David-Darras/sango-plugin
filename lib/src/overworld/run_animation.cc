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
 * @file run_animation.cc
 * @brief Uses the run animation when the player runs.
 *
 * The declarations are in overworld/patch/run_animation.h.
 */

#include "overworld/patch/run_animation.h"
#include "core/hook.h"
#include "core/native/process_manager.h"
#include "overworld/native/model.h"
#include "overworld/address.h"
#include "renderer/address.h"

namespace overworld {

namespace {
core::Hook<u32(void*, u32, u32)> play_animation_hook;
core::Hook<u32(void*, bool)> update_motion_hook;
} // namespace

void RunAnimation::Initialize() {
  play_animation_hook.Install(renderer::address::kModelPlayAnimation,
                              PlayAnimationHook);
  update_motion_hook.Install(renderer::address::kModelUpdateMotion,
                             UpdateMotionHook, address::kVtable);
}

// The player characters and Steven already have a run animation.
static bool HasRunAnimation(void* model) {
  if (!core::ProcessManager::IsOverworldActive()) return false;
  switch (static_cast<Model*>(model)->model_id) {
    case ModelId::kMay:
    case ModelId::kBrendan:
    case ModelId::kMayContest:
    case ModelId::kBrendanContest:
    case ModelId::kMayMagmaSuit:
    case ModelId::kMayAquaSuit:
    case ModelId::kBrendanMagmaSuit:
    case ModelId::kBrendanAquaSuit:
    case ModelId::kSupportMay:
    case ModelId::kSupportBrendan:
    case ModelId::kStevenStone:
      return true;
    default:
      return false;
  }
}

u32 RunAnimation::PlayAnimationHook(void* model, u32 action, u32 direction) {
  auto& feat = GetInstance();
  feat.running_ = feat.enabled && action == kRunAction &&
                  !HasRunAnimation(model);
  if (feat.running_) action = kWalkAction;
  return play_animation_hook(model, action, direction);
}

u32 RunAnimation::UpdateMotionHook(void* model, bool flag) {
  auto& feat = GetInstance();
  if (feat.running_) feat.remaining_frames_ = kRunFrameStep;
  if (feat.remaining_frames_ != 0) {
    *(u8*)((uptr)model + kFrameStepOffset) = kRunFrameStep;
    feat.remaining_frames_--;
  }
  return update_motion_hook(model, flag);
}

} // namespace overworld
