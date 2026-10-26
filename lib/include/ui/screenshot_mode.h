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
 * @file screenshot_mode.h
 * @brief Hides the menu, the overlays and the 2D menus of the game, so that
 *        the player can take a clean screenshot.
 */

#pragma once

#include "common.h"

namespace ui {
/// The settings of ui::ScreenshotMode.
struct ScreenshotSettings {
  bool hide_game_ui = true; ///< Hides the 2D menus of the game (the HUD).
  bool hide_overlays = true; ///< Hides the overlays of the plugin.
  bool freeze_game = false; ///< Stops the game.
};

/**
 * @brief The screenshot mode.
 *
 * The mode starts from a page of the menu, which closes the menu. The player
 * can then move with the free camera or in walk mode. The key of the menu
 * stops the mode and opens the menu again.
 */
class ScreenshotMode : public ScreenshotSettings {
  MAKE_SINGLETON(ScreenshotMode)
public:
  /// Closes the menu and starts the mode.
  static void Start();

  /// Stops the mode and shows everything again.
  static void Stop();

  /// Call it one time for each frame, after the menu: the mode stops when
  /// the menu opens.
  static void Update();

  /// true while the mode is on.
  static bool IsActive() { return GetInstance().is_active_; }

  /// true when the overlays of the plugin must not draw.
  static bool AreOverlaysHidden() {
    return GetInstance().is_active_ && GetInstance().hide_overlays;
  }

  /// true when the game must not run its frames.
  static bool IsFrozen() {
    return GetInstance().is_active_ && GetInstance().freeze_game;
  }

private:
  bool is_active_ = false;
};
} // namespace ui
