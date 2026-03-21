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

#ifndef SANGO_PLUGIN_HOOK_MANAGER_H
#define SANGO_PLUGIN_HOOK_MANAGER_H

#include "hook.h"

/**
 * @brief Singleton registry for managing the lifecycle of all plugin hooks.
 */
class HookManager {
 public:
  /**
   * @brief Retrieves the singleton instance of the HookManager.
   * @return Reference to the HookManager instance.
   */
  static HookManager &GetInstance() { return instance_; }

  /**
   * @brief Registers and creates a new hook in the manager.
   * @param id   The identifier for the hook.
   * @param src  The source address to hook.
   * @param dst  The destination address to redirect execution to.
   */
  void Add(HookID id, u32 src, u32 dst);

  /**
   * @brief Searches for a registered hook by its ID.
   * @param id The ID of the hook to find.
   * @return Pointer to the Hook if found, nullptr otherwise.
   */
  Hook *Get(HookID id);

 private:
  /** @brief Private constructor for singleton pattern. */
  HookManager() : count_(0) {}

  static constexpr int kMaxHooks = (int)HookID::kMax;
  static HookManager instance_;  ///< Global static instance.

  Hook hooks_[kMaxHooks];  ///< Array of pre-allocated Hook objects.
  u32 count_;              ///< Current number of registered hooks.
};

#endif  // SANGO_PLUGIN_HOOK_MANAGER_H
