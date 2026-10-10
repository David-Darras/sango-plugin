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
 * @file performance_overlay.h
 * @brief Shows the frames per second and the free memory on the top screen.
 */

#pragma once

#include "common.h"

namespace ui {
/**
 * @brief Shows the speed of the game and the free memory at the bottom of
 *        the top screen, while the menu is closed.
 *
 * The first line: the frames per second and the time of one frame. The
 * second line: the free linear memory of the system (the textures of the
 * plugin), and the largest free block of the system heap and of the device
 * heap of the game.
 */
class PerformanceOverlay {
  MAKE_SINGLETON(PerformanceOverlay)
public:
  bool is_enabled = false; ///< Shows the overlay.

  /// Measures the frame and draws the overlay. plugin::DrawFrame() calls it.
  static void DrawTop();

private:
  u64 last_tick_ = 0; ///< The system tick of the last frame.
  f32 frame_ms_ = 0; ///< The time of one frame, as a moving average.
  u32 counter_ = 0; ///< Reads the memory again after some frames.
  u32 linear_kb_ = 0;
  u32 heap_kb_[2] = {};
};
} // namespace ui
