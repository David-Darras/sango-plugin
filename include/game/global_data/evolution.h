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

#include "common.h"
#include "game/constant/evolution_method.h"
#include "game/constant/species.h"

namespace global_data {
struct EvolutionData {
  struct {
    EvolutionMethod method;
    u8 _0;
    u16 arg;
    Species species;
  } data[8];
};

struct EvolutionTable {
  uptr vtable;
  Species species;
  u16 _0;
  EvolutionData* data;
};
} // namespace global_data
