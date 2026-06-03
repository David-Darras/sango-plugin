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

#ifndef SANGO_PLUGIN_FEATURE_ENGINE_H
#define SANGO_PLUGIN_FEATURE_ENGINE_H

#include "common.h"
#include "core/core.h"
#include "feature/hook_manager.h"

namespace feature {
struct EngineHookContext {
  MAKE_SINGLETON(EngineHookContext)
  s32 game_speed = 1;
  s32 frame_count = 0;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kUpdateFrame, ADDRESS_UPDATE_FRAME,
                            (uptr)UpdateFrameHook);
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
};
} // namespace feature

#endif //SANGO_PLUGIN_FEATURE_ENGINE_H