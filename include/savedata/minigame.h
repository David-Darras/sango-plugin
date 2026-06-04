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
#include "savedata.h"
#include "game/data_manager.h"

namespace savedata {
struct Minigame {
  STATIC_INLINE Minigame& GetInstance() {
    return SaveData::GetInstance().GetMinigame();
  }

  void* vtable;
  u16 berry_picker_high_scores[4];
  u16 head_it_high_scores[4];

  struct {
    u16 time[5];
    u16 moves[5];
    u16 swaps[5];
    u16 total_score[5];
  } tile_puzzle_scores[4];

  u8 berry_picker_best_ratings[4];
  u8 head_it_best_ratings[4];
  u8 tile_puzzle_best_ratings[4];
};
} // namespace savedata
// 08C6FBC0