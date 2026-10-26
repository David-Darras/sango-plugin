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
 * @file frame_trace.h
 * @brief Writes the state of the menu and of the GPU to the SD card, to find
 *        the cause of a crash.
 */

#pragma once

#include "common.h"

namespace core {
/**
 * @brief The black box of the plugin.
 *
 * When it is on and the menu is open, the trace writes the file
 * `sdmc:/sango/frame_trace.txt` three times each second. The file always
 * has the same size, and each write replaces the last one. When the game
 * crashes, the file shows the last state: the page and the entry of the
 * menu, the number of texts and rectangles of the frame, the size of the
 * command list of the GPU, and the free memory with its lowest value.
 *
 * To use it: reproduce the crash, then read the file on the SD card (the
 * game does not need to close it). The values near the crash tell the
 * cause: a command list that is almost full, or a memory that is almost
 * empty, or a page that always shows in the file.
 */
class FrameTrace {
public:
  /// true: the trace writes the file. The writes can make the game stutter
  /// on a console, so it is off until the player needs it (page System).
  static bool is_enabled;

  /// Reads the size of the command list after the drawings of a screen.
  /// plugin::DrawFrame() calls it for each screen.
  static void OnScreenDrawn(bool is_top);

  /// Counts the frame and writes the file when it is time. plugin::DrawFrame()
  /// calls it one time at the end of each frame.
  static void EndFrame();
};
} // namespace core
