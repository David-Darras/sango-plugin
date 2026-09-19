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

#include <cstddef>

#include "common.h"
#include "core/game.h"

namespace pokemon {
struct EvolutionTable;
struct MegaEvolutionTable;
struct SpeciesData;

struct Database {
  STATIC_INLINE Database& GetInstance() {
    return *(Database*)address::kDatabase;
  }

  SpeciesData* species;
  void* _0[GAME_CONSTANT(6, 2)];
  Message* ability_names;
  Message* ability_descriptions;
  void* _1[GAME_CONSTANT(7, 9)];
  EvolutionTable* evolution;
  void* _2[2];
  MegaEvolutionTable* mega_evolution;
  void* _3[GAME_CONSTANT(47, 9)];
  Message* move_names;
};

static_assert(offsetof(Database, ability_names) == GAME_CONSTANT(0x1C, 0x0C) &&
              offsetof(Database, evolution) == GAME_CONSTANT(0x40, 0x38) &&
              offsetof(Database, mega_evolution) == GAME_CONSTANT(0x4C, 0x44) &&
              offsetof(Database, move_names) == GAME_CONSTANT(0x10C, 0x6C),
              "Database must match the game's pml::Library layout");
} // namespace pokemon
