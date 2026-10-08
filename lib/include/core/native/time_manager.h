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
 * @file time_manager.h
 * @brief The object of the game that counts the play time.
 */

#pragma once

#include "core/native/game_manager.h"
#include "common.h"

namespace core {
/// Counts the play time.
struct TimeManager {
  SINGLETON(TimeManager)
public:
  STATIC_INLINE TimeManager& GetInstance() {
    return GameManager::GetInstance().GetGameTimeManager();
  }

  bool is_enabled;
  u64 last_tick;
  u64 accumulated_seconds;
  u64 first_tick;
  /// Counts the frames: the game reads the system time one time every
  /// 20 frames only.
  u32 frame_counter;
};
} // namespace core