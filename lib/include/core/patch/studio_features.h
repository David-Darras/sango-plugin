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
 * @brief The features that Sango Studio turns on or off.
 */

#pragma once

#include "common.h"

namespace core {
/**
 * @brief Turns features of the plugin on or off, as the Features page of Sango
 * Studio asks.
 *
 * The file is `sdmc:/sango-studio/features.ini`. Its section `[features]` has
 * one line for each feature that the user changed, with 1 (on) or 0 (off). A
 * feature without a line keeps the value that the product gives it. The menu
 * of the plugin can still change a feature while the game runs.
 *
 * @code
 * [features]
 * title_screen = 0
 * shiny_entry = 1
 * @endcode
 */
class StudioFeatures {
public:
  /// Reads the file and sets the features. plugin::Start() calls it, after the
  /// settings of the product. Without the file, nothing changes.
  static void Apply();
};
} // namespace core
