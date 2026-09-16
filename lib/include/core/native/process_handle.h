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

#include "core/constant/process_state.h"
#include "core/native/base_process.h"

namespace core {

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

} // namespace core
