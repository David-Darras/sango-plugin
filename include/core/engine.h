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

#ifndef SANGO_PLUGIN_ENGINE_H
#define SANGO_PLUGIN_ENGINE_H

#include "common.h"
#include "hack/hook_manager.h"

namespace core {
class Engine {
  SINGLETON(Engine)

public:
  STATIC_INLINE Engine& GetInstance() {
    static Engine instance;
    return instance;
  }

  static void SetupHooks(HookManager& hook_manager);

  INLINE s32& GetGameSpeed() {
    return game_speed_;
  }

  INLINE s32 UpdateFrameCount() {
    return frame_count_++;
  }

private:
  static s32 UpdateFrameHook(uptr addr);

  s32 game_speed_ = 1;
  s32 frame_count_ = 0;
};
} // namespace core

#endif //SANGO_PLUGIN_ENGINE_H