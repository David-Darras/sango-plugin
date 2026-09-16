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

namespace battle {

struct LevelUpData {
  u32 exp;
  u16 ev_hp;
  u16 ev_attack;
  u16 ev_defense;
  u16 ev_speed;
  u16 ev_special_attack;
  u16 ev_special_defense;
  bool _0;
  bool _1;
  bool use_exp_share;
  bool _2;
};

} // namespace battle
