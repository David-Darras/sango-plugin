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
 * @file weather_mode.h
 * @brief The colors of the rain of the plugin.
 */

#pragma once

#include <types.h>

namespace overworld {

/// The color of the rain. The example abilities of the overlay (Toxic
/// Drizzle...) set it. renderer::ModelFilter changes the rain particles.
enum class WeatherMode : u8 {
  kNormal = 0,
  kToxic = 1,
  kRadioactive = 2,
};

} // namespace overworld
