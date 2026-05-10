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

#include "savedata/minigame.h"

#include "menu/log_menu.h"
#include "menu/plugin_menu.h"

namespace savedata {
void Minigame::LoadMenu(menu::PluginMenu& menu, void* args) {
  static u32 choice = 0;
  static u32 puzzle_idx = 0;

  static const char* DIFFICULTIES[] = {
      "Easy", "Normal", "Hard", "Unlimited"
  };

  static const char* RATINGS[] = {
      "None", "1 Star", "2 Stars", "3 Stars", "4 Stars", "4.5 Stars", "5 Stars"
  };

  Minigame& data = GetInstance();

  menu.Add("Difficulty Choice", choice)
      .WithArray(DIFFICULTIES, 4)
      .WithRefresh();

  menu.AddSeparator();

  menu.Add("Berry Picker Score", data.berry_picker_high_scores[choice])
      .WithBounds(0, 999)
      .Add("Berry Picker Rating", data.berry_picker_best_ratings[choice])
      .WithArray(RATINGS, SIZE(RATINGS))
      .AddSeparator();

  menu.Add("Head It Score", data.head_it_high_scores[choice])
      .WithBounds(0, 9999)
      .Add("Head It Rating", data.head_it_best_ratings[choice])
      .WithArray(RATINGS, SIZE(RATINGS))
      .AddSeparator();

  menu.Add("Tile Puzzle Index", puzzle_idx)
      .WithBounds(0, 4)
      .WithRefresh();

  auto& puzzle = data.tile_puzzle_scores[choice];

  menu.Add("Puzzle Score", puzzle.total_score[puzzle_idx])
      .Add("Puzzle Time (s)", puzzle.time[puzzle_idx])
      .Add("Puzzle Moves", puzzle.moves[puzzle_idx])
      .Add("Puzzle Swaps", puzzle.swaps[puzzle_idx])
      .Add("Puzzle Rating", data.tile_puzzle_best_ratings[choice])
      .WithArray(RATINGS, SIZE(RATINGS));
}
} // namespace savedata