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
#include "game/savedata/savedata.h"

namespace savedata {
struct OverworldMenu {
  STATIC_INLINE OverworldMenu& GetInstance() {
    return SaveData::GetInstance().GetOverworldMenu();
  }

  void* vtable;

  union {
    u32 flags;

    u32 is_pokemon_list_visible : 1;
    u32 is_pokedex_visible : 1;
    u32 is_bag_visible : 1;
    u32 is_trainer_card_visible : 1;
    u32 is_save_visible : 1;
    u32 is_options_visible : 1;

    u32 pokemon_list_position : 3;
    u32 pokedex_position : 3;
    u32 bag_position : 3;
    u32 trainer_card_position : 3;
    u32 save_position : 3;
    u32 options_position : 3;

    u32  : 8;
  };
};
} // namespace savedata