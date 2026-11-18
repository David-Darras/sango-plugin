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
 * @file berry_plot_state.h
 * @brief The values that the scripts use for the berry plot state.
 */

#pragma once

#include <types.h>

namespace script {

/// The berry plot state, as a script writes it.
enum class BerryPlotState : u8 {
  kEmpty = 0,
  kSeed = 1,
  kSprout = 2,
  kGrownStem = 3,
  kInBloom = 4,
  kWithBerries = 5,
  kDead = 6,
};

} // namespace script
