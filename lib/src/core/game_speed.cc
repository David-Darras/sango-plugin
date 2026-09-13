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

#include "core/patch/game_speed.h"
#include "core/hook_manager.h"

namespace core {

void GameSpeed::Initialize() {
  HookManager::Initialize(HookId::kUpdateFrame, address::kUpdateFrame,
                          (uptr)UpdateFrameHook);
  HookManager::Initialize(HookId::kStartBackupThread,
                          address::kStartBackupThread,
                          (uptr)StartBackupThread);
}

void GameSpeed::StartBackupThread(uptr self, u32 a, u32 b, u32 c, u32 d) {
  GetInstance().game_speed = 1;
  HookManager::Call<void>(HookId::kStartBackupThread, self, a, b, c, d);
}

s32 GameSpeed::UpdateFrameHook(uptr addr) {
  Hook* hook = HookManager::GetInstance().Get(HookId::kUpdateFrame);

  auto& ctx = GetInstance();
  ctx.frame_count++;

  if (ctx.game_speed >= 1) {
    s32 res = 0;
    for (s32 i = 0; i < ctx.game_speed; i++) {
      res = hook->CallOriginal<s32>(addr);
    }
    return res;
  }

  if (ctx.game_speed < 0) {
    s32 divider = -ctx.game_speed;
    if (ctx.frame_count % divider == 0) {
      return hook->CallOriginal<s32>(addr);
    }
    return 1;
  }

  return hook->CallOriginal<s32>(addr);
}

} // namespace core
