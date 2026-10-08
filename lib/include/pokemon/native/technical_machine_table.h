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
 * @file technical_machine_table.h
 * @brief The moves of the TMs and HMs.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/move.h"

namespace pokemon {
/// The moves of the TMs and HMs.
class TechnicalMachineTable {
  SINGLETON(TechnicalMachineTable)
public:
  /// Returns the table of the moves. The index is the TM number - 1.
  STATIC_INLINE MoveId* GetTable() {
    return (MoveId*)address::kTechnicalMachineMoveTable;
  }
};
} // namespace pokemon
