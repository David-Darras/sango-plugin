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

#include <types.h>

#include "core/game.h"

namespace battle {

struct EntryAnimationData {
  u32 view;
  u32 _0;
  u32 state;
  u8 position[GAME_CONSTANT(3, 5)];
  u32 trainer_model[2];

  bool skip_pokeball_animation;
  bool is_long_encounter_animation;
  bool use_trainer_pov;
  bool show_fade_in;
  bool show_shiny_animation;
  bool dont_show_trainer;
};

} // namespace battle
