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
 * @file shop_pokemon.h
 * @brief One Pokémon that a shop sells (see pokemon::CustomShop).
 */

#pragma once

#include "core/types.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/species.h"

namespace pokemon {

/// One Pokémon that a shop sells.
struct ShopPokemon {
  SpeciesId species;
  FormId form;
  u8 level;
  u32 price;
};

} // namespace pokemon
