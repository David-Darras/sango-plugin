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
 * @file berry_plot_value.h
 * @brief The values that the scripts use for the berry plot value.
 */

#pragma once

#include <types.h>

namespace script {

/// The berry plot value, as a script writes it.
enum class BerryPlotValue : u8 {
  kState = 0,
  kMinutesSinceBerryWasPlanted = 1,
  kMinutesUntilSoilIsDry = 2,
  kBerryThatIsPlanted = 3,
  kNumberBerriesHarvest = 4,
};

} // namespace script
