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

#include <cstring>

#include "common.h"
#include "core/native/base_process.h"
#include "core/native/game_manager.h"
#include "core/native/process_handle.h"
#include "core/native/process_virtual_table.h"
#include "core/utils.h"
#include "ui/log_application.h"

namespace core {

class GameManager;

/**
 * @brief Singleton manager that controls the lifecycle of game processes.
 */
class ProcessManager {
  SINGLETON(ProcessManager)

public:
  /**
 * @brief Retrieves the singleton instance of the GameProcManager.
 * @return A reference to the active manager instance.
 */
  STATIC_INLINE ProcessManager& GetInstance() {
    return GameManager::GetInstance().GetProcessManager();
  }

  /**
 * @brief Retrieves the main (root) process handle.
 * @return A reference to the main ProcessHandle.
 */
  INLINE ProcessHandle& GetMainHandle() const { return *handle_; }

  BaseProcess* GetCurrentProcess() const {
    if (handle_ == nullptr) return nullptr;
    BaseProcess* process = handle_->GetProcess();
    if (process == nullptr || process->vtable == nullptr) return nullptr;
    return process;
  }

  const char* GetCurrentProcessName(uptr& vtable) const {
    BaseProcess* process = GetCurrentProcess();
    if (process == nullptr) return "";
    vtable = (uptr)process->vtable;
    return Utils::GetClassNameFromVTable(process->vtable);
  }

  uptr GetCurrentVTable() const {
    BaseProcess* process = GetCurrentProcess();
    if (process == nullptr) return 0;
    return (uptr)process->vtable;
  }

  INLINE bool IsCurrentProcess(const char* name) const {
    uptr vtable = 0;
    return std::strcmp(GetCurrentProcessName(vtable), name) == 0;
  }

  INLINE bool IsCurrentProcess(u32 vtable) const {
    return vtable == GetCurrentVTable();
  }

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

  STATIC_INLINE bool IsOverworldActive() {
    return GetInstance().IsCurrentProcess(overworld::address::kVtable);
  }

  STATIC_INLINE bool IsBattleActive() {
    return GetInstance().IsCurrentProcess(battle::address::kVtable);
  }

private:
  // Member fields mapped to the engine memory layout
  void* vtable_; ///< Pointer to the virtual method table.
  void* heap_base_; ///< Base address of the process heap.

  u32 flags_;
  void* data_;

public:
  ProcessHandle* handle_; ///< Pointer to the root process handle.
  GameManager* game_manager_;
  ///< Pointer back to the parent GameManager.
};
} // namespace core
