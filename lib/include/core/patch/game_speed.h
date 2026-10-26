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
 * @file game_speed.h
 * @brief Makes the game faster or slower.
 */

#pragma once

#include <type_traits>

#include "common.h"

namespace core {

/// The settings of core::GameSpeed.
struct GameSpeedSettings {
  /// The number of game frames for each real frame. A negative value makes
  /// the game slower: one game frame every -game_speed real frames.
  s32 game_speed = 1;
};
static_assert(std::is_standard_layout<GameSpeedSettings>::value,
              "GameSpeedSettings must have standard layout");

/// Makes the game faster or slower. The player changes `game_speed` in the menu.
struct GameSpeed : public GameSpeedSettings {
  MAKE_SINGLETON(GameSpeed)

  static void Initialize();
  static void StartBackupThread(uptr self, u32 a, u32 b, u32 c, u32 d);
  static s32 UpdateFrameHook(uptr addr);

  /// The frame rate of the game: 0 (the game chooses), 1 (30 FPS) or 2
  /// (60 FPS). ORAS only.
  /// The engine runs the field at 30 frames in each second: 60 makes it
  /// smoother, but some animations and scripts then run too fast.
  u8 frame_rate = 0;
  /// true: the game stops while the menu is open (the menu needs less time).
  bool freeze_in_menu = false;

private:
  s32 frame_count = 0;
};
} // namespace core
