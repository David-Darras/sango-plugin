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
 * @file poke_info.h
 * @brief The look of a Pokémon: species, form, gender, shiny state.
 */

#pragma once

#include "core/types.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender.h"
#include "pokemon/constant/species.h"

namespace pokemon {

/**
 * @brief The look of a Pokémon. The game uses it to make a Pokémon model.
 *
 * pokemon::ModelReplacement::on_create can change it.
 */
struct PokeInfo {
  SpeciesId species;
  FormId form;
  Gender gender;
  bool is_shiny;
  bool is_egg;
  u32 _0;
};

} // namespace pokemon

using pokemon::PokeInfo;
