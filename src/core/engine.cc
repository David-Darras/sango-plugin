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

#include "core/engine.h"

#include "core/core.h"
#include "hack/hook_manager.h"

namespace core {
void Engine::Initialize() {
  HookManager::GetInstance().Add(HookID::kOnUpdateFrame, ADDRESS_UPDATE_FRAME,
                                 (uptr)Engine::UpdateFrame);
}

s32 Engine::UpdateFrame(uptr addr) {
  Engine& engine = GetInstance();
  Hook* hook = HookManager::GetInstance().Get(HookID::kOnUpdateFrame);

  s32 res = 0;
  s32 frame_count = engine.UpdateFrameCount();
  s32 game_speed = engine.GetGameSpeed();

  if (game_speed >= 1) {
    for (s32 i = 0; i < game_speed; i++) {
      res = hook->CallOriginal<s32>(addr);
    }
    return res;
  }

  if (game_speed < 0) {
    s32 divider = -game_speed;
    if (frame_count % divider == 0) {
      return hook->CallOriginal<s32>(addr);
    }
    return 1;
  }

  return hook->CallOriginal<s32>(addr);
}
} // namespace core