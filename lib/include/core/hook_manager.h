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

#include "core/constant/hook_id.h"
#include "core/hook.h"

namespace core {

/**
 * @brief Singleton registry for managing the lifecycle of all plugin hooks.
 */
class HookManager {
  MAKE_SINGLETON(HookManager)

public:
  void Add(HookId id, u32 src, u32 dst, bool enable);
  Hook* Get(HookId id);

  template <typename R, typename... Args>
  STATIC_INLINE R Call(HookId id, Args... args) {
    return GetInstance().Get(id)->CallOriginal<R>(args...);
  }

  static void Initialize(HookId id, u32 src, u32 dst, bool enable = true);
  static void Enable(HookId id);
  static void ForceEnable(HookId id);
  static void Disable(HookId id);
  static void Clear(HookId id);

private:
  static constexpr int kMaxHooks = (int)HookId::kMax;

  Hook hooks_[kMaxHooks];
  u32 count_ = 0;
};

} // namespace core

