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
 * @file process_patch.h
 * @brief Calls the patches of the game processes when they start.
 */

#pragma once

#include "common.h"

namespace core {
class ProcessManager;
}

namespace core {

/**
 * @brief Calls the patches of a process when it starts and at each frame.
 *
 * When a process starts, it writes again the hooks of the process (see
 * core::Hook). Then it calls the patches, for example
 * battle::Battle::PatchLoad() when a battle starts.
 */
class ProcessPatch {
  MAKE_SINGLETON(ProcessPatch)

public:
  /// Called with the vtable of each process that starts, before the patches
  /// of the library. Compare the vtable with the `kVtable` addresses.
  typedef void (*ProcessLoadCallback)(uptr vtable);
  ProcessLoadCallback on_process_load = nullptr;

  static void Initialize();

private:
  static u32 MainProcessLoopHook(ProcessManager* manager);
  static void OnUpdate(uptr vtable);
  static void OnLoad(uptr vtable);
};

} // namespace core
