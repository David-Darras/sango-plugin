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
 * @file process_manager.h
 * @brief The object of the game that runs the processes.
 */

#pragma once

#include <cstring>

#include "common.h"
#include "core/native/base_process.h"
#include "core/native/game_manager.h"
#include "core/native/process_handle.h"
#include "core/utils.h"

namespace core {

class GameManager;

/**
 * @brief Runs the processes of the game: the title screen, the overworld...
 *
 * @code
 * if (core::ProcessManager::IsOverworldActive()) { ... }
 * @endcode
 */
class ProcessManager {
  SINGLETON(ProcessManager)

public:
  STATIC_INLINE ProcessManager& GetInstance() {
    return GameManager::GetInstance().GetProcessManager();
  }

  INLINE ProcessHandle& GetMainHandle() const { return *handle_; }

  /// Returns the current main process, or null.
  BaseProcess* GetCurrentProcess() const {
    if (handle_ == nullptr) return nullptr;
    BaseProcess* process = handle_->GetProcess();
    if (process == nullptr || process->vtable == nullptr) return nullptr;
    return process;
  }

  /// Returns the class name of the current process, and writes its vtable.
  const char* GetCurrentProcessName(uptr& vtable) const {
    BaseProcess* process = GetCurrentProcess();
    if (process == nullptr) return "";
    vtable = (uptr)process->vtable;
    return Utils::GetClassNameFromVTable(process->vtable);
  }

  /// Returns the vtable of the current process, or 0.
  uptr GetCurrentVTable() const {
    BaseProcess* process = GetCurrentProcess();
    if (process == nullptr) return 0;
    return (uptr)process->vtable;
  }

  /// Returns true when the current process has this class name.
  INLINE bool IsCurrentProcess(const char* name) const {
    uptr vtable = 0;
    return std::strcmp(GetCurrentProcessName(vtable), name) == 0;
  }

  /// Returns true when the current process has this vtable.
  INLINE bool IsCurrentProcess(u32 vtable) const {
    return vtable == GetCurrentVTable();
  }

  /// Calls `on_load` for a process that starts, else `on_update`.
  INLINE void Patch(void (*on_load)(uptr), void (*on_update)(uptr)) {
    if (handle_ == nullptr) return;
    auto* process = handle_->process_;
    if (process == nullptr) return;
    uptr vtable = (uptr)process->vtable;
    if (handle_->state_ == ProcessState::kLoading) {
      on_load(vtable);
    } else {
      on_update(vtable);
    }
  }

  /// Returns true when the player is in the overworld.
  STATIC_INLINE bool IsOverworldActive() {
    return GetInstance().IsCurrentProcess(overworld::address::kVtable);
  }

  /// Returns true during a battle.
  STATIC_INLINE bool IsBattleActive() {
    return GetInstance().IsCurrentProcess(battle::address::kVtable);
  }

private:
  void* vtable_;
  void* heap_base_;

  u32 flags_;
  void* data_;

public:
  ProcessHandle* handle_;
  GameManager* game_manager_;
};
} // namespace core
