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

#ifndef SANGO_PLUGIN_PROCESS_MANAGER_H
#define SANGO_PLUGIN_PROCESS_MANAGER_H

#include "core/game_manager.h"
#include "common.h"

class ProcessHandle;
class BaseProcess;

/**
 * @brief Execution states for the game processes.
 */
enum class ProcessState : u32 {
  kLoading = 0,   ///< Process is loading resources.
  kInitializing,  ///< Process is setting up internal data.
  kRunning,       ///< Process is actively updating.
  kStopped,       ///< Process is paused or halted.
  kCustom,        ///< Custom engine-specific state.
  kMax            ///< State count sentinel.
};

/**
 * @brief Base class for all game process logic.
 * This structure reflects the memory layout of the engine's base process.
 */
class BaseProcess {
 public:
  void* vtable;          ///< Pointer to the virtual method table.
  u32 sub_state;         ///< Internal sub-state of the process.
  bool is_done;          ///< Flag indicating if the process has finished.
  BaseProcess* parent_;  ///< Pointer to the parent process logic.
  void* ro_;             ///< Pointer to the associated executable module.
  void** ro_child_;      ///< Array of child module pointers.
  u32 ro_child_count_;   ///< Number of child modules attached.
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
  BaseProcess& GetProcess() const { return *process_; }

 private:
  ProcessState state_;     ///< Current execution state of the handle.
  ProcessHandle* parent_;  ///< Pointer to the parent handle.
  ProcessHandle* child_;   ///< Pointer to the first child handle.
  BaseProcess* process_;   ///< Pointer to the underlying process logic.
};

class GameManager;

/**
 * @brief Singleton manager that controls the lifecycle of game processes.
 */
class GameProcessManager {
 private:
  /** @brief Private constructor to enforce singleton pattern or memory mapping.
   */
  GameProcessManager() = default;

 public:
  /**
   * @brief Retrieves the singleton instance of the GameProcManager.
   * @return A reference to the active manager instance.
   */
  static GameProcessManager& GetInstance() {
    return GameManager::GetInstance().GetGameProcessManager();
  }

  /**
   * @brief Retrieves the main (root) process handle.
   * @return A reference to the main ProcessHandle.
   */
  ProcessHandle& GetMainHandle() const { return *handle_; }

 private:
  // Member fields mapped to the engine memory layout
  void* vtable_;    ///< Pointer to the virtual method table.
  void* heap_base_;  ///< Base address of the process heap.

  u32 flags_;
  void* data_;

  ProcessHandle* handle_;      ///< Pointer to the root process handle.
  GameManager* game_manager_;  ///< Pointer back to the parent GameManager.
};

#endif  // SANGO_PLUGIN_PROCESS_MANAGER_H