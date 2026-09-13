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

struct PlayerCheats {
  MAKE_SINGLETON(PlayerCheats)
  Vec3 speed = Vec3{1, 1, 1};
  f32 theta = 0.0f;
  f32 radius = 5.0f;
  f32 theta_speed = 1.0f;
  u32 model_idx = 0;
  u16 model_animation = 0;

  static void Initialize();
  static void PlayAnimation(void*);
  static void SwarmMod();
  static void Noclip();
};

} // namespace overworld
