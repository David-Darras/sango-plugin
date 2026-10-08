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
 * @file ai.h
 * @brief The AI flags of the trainers.
 */

#pragma once

#include "core/bitmask.h"
#include "core/types.h"

namespace battle {

/// The AI of a trainer. Combine the flags with `|`.
enum class AiFlags : u32 {
  kNone = 0,
  kCasual = 1u << 0, ///< Basic choices.
  kCompetitive = 1u << 1, ///< Better move choices.
  kStrategist = 1u << 2, ///< Uses the status moves and switches the Pokémon.
  kMulti = 1u << 7, ///< For a battle with several trainers.
  kHorde = 1u << 14, ///< For a horde battle.
};
ENABLE_BITMASK_OPERATORS(AiFlags)

} // namespace battle

using battle::AiFlags;
