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
 * @file studio_features.h
 * @brief The features that Sango Studio sets.
 */

#pragma once

#include "common.h"

namespace core {
/**
 * @brief Sets the features of the plugin, as the Features page of Sango Studio
 * asks.
 *
 * Without settings, the game is the game: every feature is off. The file is
 * `sdmc:/sango-studio/features.ini`. Its section `[features]` has one line for
 * each setting that the user changed. A setting without a line keeps the value
 * that the product gives it. The menu of the plugin can still change a setting
 * while the game runs.
 *
 * A value is a number: 1 or 0 for a switch, a whole number for a choice or a
 * species, a decimal number, or a color as RRGGBB in hexadecimal.
 *
 * @code
 * [features]
 * battle.hp_gradient = 1
 * graphics.outline_color = FF8000
 * flight.speed = 1.5
 * @endcode
 */
class StudioFeatures {
public:
  /// Reads the file and sets the features. plugin::Start() calls it, after the
  /// settings of the product. Without the file, nothing changes.
  static void Apply();
};
} // namespace core
