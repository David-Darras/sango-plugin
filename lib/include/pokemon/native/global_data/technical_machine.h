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

#include "common.h"
#include "pokemon/constant/move.h"

namespace global_data {
class TechnicalMachine {
  SINGLETON(TechnicalMachine)
public:
  /// The move taught by each TM, indexed by TM number - 1.
  STATIC_INLINE MoveId* GetTable() {
    return (MoveId*)pokemon::address::kTechnicalMachineMoveTable;
  }
};
} // namespace global_data
