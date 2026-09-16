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

namespace pokemon {
struct EvolutionTable;
struct MegaEvolutionTable;
struct SpeciesData;

struct Database {
  STATIC_INLINE Database& GetInstance() {
    return *(Database*)address::kDatabase;
  }

  SpeciesData* species;
  void* _0[2];
  Message* ability_names;
  Message* ability_descriptions;
  void* _1[9];
  EvolutionTable* evolution;
  void* _2[2];
  MegaEvolutionTable* mega_evolution;
  void* _3[9];
  Message* move_names;
};

static_assert(offsetof(Database, ability_names) == 0x0C &&
              offsetof(Database, evolution) == 0x38 &&
              offsetof(Database, mega_evolution) == 0x44 &&
              offsetof(Database, move_names) == 0x6C,
              "Database must match the game's pml::Library layout");
} // namespace pokemon
