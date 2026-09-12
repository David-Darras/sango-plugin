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
#include "feature/core/hook_manager.h"

namespace feature {
struct Engine {
  MAKE_SINGLETON(Engine)
  s32 game_speed = 1;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kUpdateFrame, ADDRESS_UPDATE_FRAME,
                            (uptr)UpdateFrameHook);
    HookManager::Initialize(HookID::kStartBackupThread,
                            ADDRESS_START_BACKUP_THREAD,
                            (uptr)StartBackupThread);
  }

  static void StartBackupThread(uptr self, u32 a, u32 b, u32 c, u32 d) {
    GetInstance().game_speed = 1;
    HookManager::Call<void>(HookID::kStartBackupThread, self, a, b, c, d);
  }

  static s32 UpdateFrameHook(uptr addr) {
    Hook* hook = HookManager::GetInstance().Get(HookID::kUpdateFrame);

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

private:
  s32 frame_count = 0;
};
} // namespace feature