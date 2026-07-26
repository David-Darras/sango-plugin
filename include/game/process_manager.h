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

#include "manager.h"
#include "common.h"
#include "utils.h"
#include "ui/log_application.h"

namespace game {
class ProcessHandle;
class BaseProcess;

/**
 * @brief Execution states for the game processes.
 */
enum class ProcessState : u32 {
  kLoading = 0, ///< Process is loading resources.
  kInitializing, ///< Process is setting up internal data.
  kRunning, ///< Process is actively updating.
  kStopped, ///< Process is paused or halted.
  kCustom, ///< Custom engine-specific state.
  kMax ///< State count sentinel.
};

struct ProcessVirtualTable {
  uptr destructor;
  uptr destructor2;
  uptr initialize;
  uptr update;
  uptr draw;
  uptr finalize;
};

/**
 * @brief Base class for all game process logic.
 * This structure reflects the memory layout of the engine's base process.
 */
class BaseProcess {
public:
  void* vtable; ///< Pointer to the virtual method table.
  u32 sub_state; ///< Internal sub-state of the process.
  bool is_done; ///< Flag indicating if the process has finished.
  BaseProcess* parent_; ///< Pointer to the parent process logic.
  void* ro_; ///< Pointer to the associated executable module.
  void** ro_child_; ///< Array of child module pointers.
  u32 ro_child_count_; ///< Number of child modules attached.
};

/**
 * @brief Container for process execution and hierarchy.
 * Links the logical BaseProcess with its position in the process tree.
 */
class ProcessHandle {
public:
  /**
 * @brief Accesses the underlying process logic.
 * @return A reference to the BaseProcess.
 */
  BaseProcess* GetProcess() const { return process_; }

public:
  ProcessState state_; ///< Current execution state of the handle.
  ProcessHandle* parent_; ///< Pointer to the parent handle.
  ProcessHandle* child_; ///< Pointer to the first child handle.
  BaseProcess* process_; ///< Pointer to the underlying process logic.
};

class Manager;

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
    return Manager::GetInstance().GetProcessManager();
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

private:
  // Member fields mapped to the engine memory layout
  void* vtable_; ///< Pointer to the virtual method table.
  void* heap_base_; ///< Base address of the process heap.

  u32 flags_;
  void* data_;

public:
  ProcessHandle* handle_; ///< Pointer to the root process handle.
  Manager* game_manager_;
  ///< Pointer back to the parent game::GameManager.
};
} // namespace game