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

#include "core/types.h"
#include "pokemon/constant/gender.h"

namespace battle {
struct TrainerAppearance {
  u32 type;
  u8 battle_model_sequence : 4;
  Gender gender : 2;
  u8 plural : 2;
  u8 group;
  u8 battle_effect_id;
  u8 model_3d;
  u32 cut_in_2d;
  u32 overworld_model;
  u32 encounter_bgm;
};
} // namespace battle
