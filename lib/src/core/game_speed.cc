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
 * @file game_speed.cc
 * @brief Makes the game faster or slower.
 *
 * The declarations are in core/patch/game_speed.h.
 */

#include "core/patch/game_speed.h"
#include "core/hook.h"

namespace core {

namespace {
core::Hook<s32(uptr)> update_frame_hook;
core::Hook<void(uptr, u32, u32, u32, u32)> start_backup_thread_hook;
} // namespace

void GameSpeed::Initialize() {
  update_frame_hook.Install(address::kUpdateFrame, UpdateFrameHook);
  start_backup_thread_hook.Install(address::kStartBackupThread,
                                   StartBackupThread);
}

void GameSpeed::StartBackupThread(uptr self, u32 a, u32 b, u32 c, u32 d) {
  GetInstance().game_speed = 1;
  start_backup_thread_hook(self, a, b, c, d);
}

s32 GameSpeed::UpdateFrameHook(uptr addr) {
  auto& ctx = GetInstance();
  ctx.frame_count++;

  if (ctx.game_speed >= 1) {
    s32 res = 0;
    for (s32 i = 0; i < ctx.game_speed; i++) {
      res = update_frame_hook(addr);
    }
    return res;
  }

  if (ctx.game_speed < 0) {
    s32 divider = -ctx.game_speed;
    if (ctx.frame_count % divider == 0) {
      return update_frame_hook(addr);
    }
    return 1;
  }

  return update_frame_hook(addr);
}

} // namespace core
