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

#ifndef SANGO_PLUGIN_POKEMON_RUNTIME_DATA_H
#define SANGO_PLUGIN_POKEMON_RUNTIME_DATA_H
#include "common.h"

struct PokemonRuntimeData {
  u32 status_condition;
  u8 level;
  u8 furfrou_remaining_trim_days;
  u8 furfrou_elapsed_days;
  u8 training_flags;
  u16 hp;
  u16 max_hp;
  u16 attack;
  u16 defense;
  u16 speed;
  u16 special_attack;
  u16 special_defense;
  u16 _0;
  bool is_mega_evolved;
  u8 last_form;
  u16 _1;
};

#endif  // SANGO_PLUGIN_POKEMON_RUNTIME_DATA_H
