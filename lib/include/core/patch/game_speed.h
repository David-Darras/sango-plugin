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

#pragma once

#include <type_traits>

#include "common.h"

namespace core {

struct GameSpeedSettings {
  /// Frames simulated per real frame; negative values slow the game down
  /// (one frame every -game_speed frames).
  s32 game_speed = 1;
};
static_assert(std::is_standard_layout<GameSpeedSettings>::value,
              "GameSpeedSettings must have standard layout");

struct GameSpeed : public GameSpeedSettings {
  MAKE_SINGLETON(GameSpeed)

  static void Initialize();
  static void StartBackupThread(uptr self, u32 a, u32 b, u32 c, u32 d);
  static s32 UpdateFrameHook(uptr addr);

private:
  s32 frame_count = 0;
};
} // namespace core
