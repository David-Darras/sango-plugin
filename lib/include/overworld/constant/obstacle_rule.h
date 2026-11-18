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
 * @file obstacle_rule.h
 * @brief The values that the scripts use for the obstacle rule.
 */

#pragma once

#include <types.h>

namespace overworld {

/// The obstacle rule, as a script writes it.
enum class ObstacleRule : u8 {
  kDoNotIgnoreObstacles = 0,
  kIgnoreCharactersOnly = 1,
  kIgnoreMapOnly = 2,
  kIgnoreEverything = 3,
};

} // namespace overworld
