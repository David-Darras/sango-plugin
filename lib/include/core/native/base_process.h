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
 * @file base_process.h
 * @brief The base structure of a game process.
 */

#pragma once

#include "core/types.h"

namespace core {

/**
 * @brief A process of the game: the title screen, the overworld, a battle...
 *
 * The vtable identifies the process. The `kVtable` addresses of the domains
 * (for example battle::address::kVtable) are vtables of processes.
 */
class BaseProcess {
public:
  void* vtable; ///< Identifies the type of the process.
  u32 sub_state; ///< The step of the process.
  bool is_done; ///< true when the process ends.
  BaseProcess* parent_; ///< The process that started this process.
  void* ro_; ///< The CRO module of the process.
  void** ro_child_; ///< The child CRO modules.
  u32 ro_child_count_;
};

} // namespace core
