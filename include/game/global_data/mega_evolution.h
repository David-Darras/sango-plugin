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
#include "game/constant/item.h"
#include "game/constant/mega_evolution_method.h"

namespace global_data {
struct MegaEvolutionData {
  struct {
    Form form;
    u8 _0;
    MegaEvolutionMethod method;
    u8 _1;
    ItemID item;
    u16 _2;
  } entry[3];
};

struct MegaEvolutionTable {
  uptr vtable;
  Species species;
  u16 _0;
  MegaEvolutionData* data;
};
} // namespace global_data