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
 * @file hook_manager.h
 * @brief Installs the hooks and calls the original functions.
 *
 * @see docs/concepts/hooks-and-addresses.md
 * @see docs/tutorials/03-add-a-hook.md
 */

#pragma once

#include "core/constant/hook_id.h"
#include "core/hook.h"

namespace core {

/**
 * @brief Keeps one core::Hook for each core::HookId.
 *
 * @code
 * core::HookManager::Initialize(HookId::kProduct0,
 *                               pokemon::address::kAddPokemonToTeam,
 *                               (uptr)AddPokemonToTeamHook);
 * // In the hook:
 * return core::HookManager::Call<bool>(HookId::kProduct0, team, pokemon);
 * @endcode
 */
class HookManager {
  MAKE_SINGLETON(HookManager)

public:
  /// Prepares a hook. Use Initialize() instead.
  void Add(HookId id, u32 src, u32 dst, bool enable);
  /// Returns the hook with this id, or null.
  Hook* Get(HookId id);

  /**
   * @brief Runs the original game function of a hook.
   * @tparam R The return type of the game function.
   * @param id The id of the hook.
   * @param args The parameters of the game function.
   */
  template <typename R, typename... Args>
  STATIC_INLINE R Call(HookId id, Args... args) {
    return GetInstance().Get(id)->CallOriginal<R>(args...);
  }

  /**
   * @brief Prepares a hook, and installs it when `enable` is true.
   * @param id The id of the hook. A product uses HookId::kProduct0 to kProduct15.
   * @param src The address of the game function.
   * @param dst The address of the plugin function: `(uptr)MyHook`.
   * @param enable false for a hook in a CRO: enable it later.
   *
   * The function does nothing when `src` is 0 (address not found yet).
   */
  static void Initialize(HookId id, u32 src, u32 dst, bool enable = true);
  /// Installs the hook if it is not installed.
  static void Enable(HookId id);
  /// Installs the hook again. Use it after the game loads a CRO again.
  static void ForceEnable(HookId id);
  /// Removes the hook: the game function is normal again.
  static void Disable(HookId id);
  /// Forgets the state of the hook, without a write to memory.
  static void Clear(HookId id);

private:
  static constexpr int kMaxHooks = (int)HookId::kMax;

  Hook hooks_[kMaxHooks];
  u32 count_ = 0;
};

} // namespace core

