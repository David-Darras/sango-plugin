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

#include "core/types.h"

namespace core {

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

} // namespace core
