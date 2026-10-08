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

/**
 * @file mega_evolution_data.h
 * @brief The Mega Evolutions of one species.
 */

#pragma once

#include "core/types.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/mega_evolution_method.h"

namespace pokemon {

/// The Mega Evolutions of one species: 3 at most (for example Mega Charizard X and Y).
struct MegaEvolutionData {
  struct {
    FormId form;
    u8 _0;
    MegaEvolutionMethod method;
    u8 _1;
    ItemId item;
    u16 _2;
  } entry[3];
};

} // namespace pokemon
