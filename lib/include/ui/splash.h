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
 * @file splash.h
 * @brief The splash screen of the plugin.
 */

#pragma once

#include "common.h"

namespace ui {
/**
 * @brief Shows the name of the plugin, its version, the date of the build and
 *        the author on the top screen, a few seconds after the start.
 */
class Splash {
public:
  /// The frames that the splash screen stays on the screen (60 = 1 second).
  static constexpr u32 kFrames = 100;

  /// Draws the splash screen on the top screen. Call it one time for each
  /// frame, after the other drawings: the screen counts its own frames.
  static void DrawTop();
};
} // namespace ui
