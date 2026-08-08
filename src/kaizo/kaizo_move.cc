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
 * along with this program.  If not, see <https:
 */

#include "game/global_data/move.h"
#include "game/constant/move.h"

namespace kaizo {
void PatchMoveData() {
  auto* table = global_data::Move::GetTable();

  table[MOVE_ACUPRESSURE].base_pp = 2;
  table[MOVE_DRAGON_DANCE].base_pp = 1;
  table[MOVE_SWORDS_DANCE].base_pp = 1;
  table[MOVE_NASTY_PLOT].base_pp = 1;
  table[MOVE_AGILITY].base_pp = 1;
  table[MOVE_ROCK_POLISH].base_pp = 2;
  table[MOVE_MINIMIZE].base_pp = 1;
  table[MOVE_DOUBLE_TEAM].base_pp = 2;
  table[MOVE_BULK_UP].base_pp = 2;
  table[MOVE_CALM_MIND].base_pp = 2;
  table[MOVE_QUIVER_DANCE].base_pp = 1;
  table[MOVE_AMNESIA].base_pp = 1;
  table[MOVE_TAIL_GLOW].base_pp = 1;
  table[MOVE_COTTON_GUARD].base_pp = 1;
  table[MOVE_ACID_ARMOR].base_pp = 1;
  table[MOVE_STOCKPILE].base_pp = 2;
  table[MOVE_CURSE].base_pp = 2;
  table[MOVE_SUBSTITUTE].base_pp = 2;
  table[MOVE_PROTECT].base_pp = 2;
}
} // namespace kaizo