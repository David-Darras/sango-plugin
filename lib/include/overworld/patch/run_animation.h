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

#include "common.h"

namespace overworld {

class RunAnimation {
  MAKE_SINGLETON(RunAnimation)

public:
  bool enabled = true;

  static void Initialize();
  static void PatchLoad();

private:
  static constexpr u32 kRunAction = 0x0E;
  static constexpr u32 kWalkAction = 0x13;
  static constexpr u32 kRunFrameStep = 4;
  static constexpr u32 kFrameStepOffset = 0x10;

  static u32 PlayAnimationHook(void* model, u32 action, u32 direction);
  static u32 UpdateMotionHook(void* model, bool flag);

  bool running_ = false;
  u32 remaining_frames_ = 0;
};

} // namespace overworld
