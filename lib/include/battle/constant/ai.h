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

#include "core/bitmask.h"
#include "core/types.h"

/// Bitmask of AI behaviours enabled for a trainer (battle::TrainerData::ai_flags).
namespace battle {

enum class AiFlags : u32 {
  kNone = 0,
  kCasual = 1u << 0,
  kCompetitive = 1u << 1,
  kStrategist = 1u << 2,
  kMulti = 1u << 7,
  kHorde = 1u << 14,
};
ENABLE_BITMASK_OPERATORS(AiFlags)

} // namespace battle

using battle::AiFlags;
