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

#include "overworld/patch/run_animation.h"
#include "core/hook_manager.h"

namespace overworld {

void RunAnimation::Initialize() {
  core::HookManager::Initialize(HookId::kModelPlayAnimation,
                          renderer::address::kModelPlayAnimation, (uptr)PlayAnimationHook);
  core::HookManager::Initialize(HookId::kModelUpdateMotion,
                          renderer::address::kModelUpdateMotion, (uptr)UpdateMotionHook,
                          false);
}

void RunAnimation::PatchLoad() {
  core::HookManager::ForceEnable(HookId::kModelUpdateMotion);
}

u32 RunAnimation::PlayAnimationHook(void* model, u32 action, u32 direction) {
  auto& feat = GetInstance();
  feat.running_ = feat.enabled && action == kRunAction;
  if (feat.running_) action = kWalkAction;
  return core::HookManager::Call<u32>(HookId::kModelPlayAnimation, model, action,
                                direction);
}

u32 RunAnimation::UpdateMotionHook(void* model, bool flag) {
  auto& feat = GetInstance();
  if (feat.running_) feat.remaining_frames_ = kRunFrameStep;
  if (feat.remaining_frames_ != 0) {
    *(u8*)((uptr)model + kFrameStepOffset) = kRunFrameStep;
    feat.remaining_frames_--;
  }
  return core::HookManager::Call<u32>(HookId::kModelUpdateMotion, model, flag);
}

} // namespace overworld
