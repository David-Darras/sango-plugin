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
 * @file type_multiplier.h
 * @brief The values of the type chart.
 */

#pragma once

#include <types.h>

namespace battle {

/// One cell of the type chart: the damage multiplier of an attacking type
/// against a defending type.
enum class TypeMultiplier : u8 {
  k0 = 0, ///< No effect.
  k05 = 1, ///< Not very effective (x0.5).
  k1 = 2, ///< Normal (x1).
  k2 = 4
};

} // namespace battle
