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

u32 RunAnimation::PlayAnimationHook(void* model, u32 action, u32 direction) {
  auto& feat = GetInstance();
  feat.running_ = feat.enabled && action == kRunAction;
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
