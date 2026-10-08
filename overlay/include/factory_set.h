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
 * @file factory_set.h
 * @brief The Pokémon sets of the Battle Factory script of the overlay.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/ball.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/nature.h"
#include "pokemon/constant/species.h"
#include "savedata/native/pokemon_param.h"

namespace factory {
struct PokemonSpec {
  pokemon::SpeciesId species;
  pokemon::ItemId item;
  pokemon::AbilityId ability;
  pokemon::Nature nature;
  pokemon::MoveId moves[4];
  u8 evs[6]; // hp, attack, defense, special attack, special defense, speed
  pokemon::Ball ball;
};

pokemon::SpeciesId GetRandomSpecies(const pokemon::SpeciesId* excluded,
                                    u32 excluded_count);
PokemonSpec BuildSpec(pokemon::SpeciesId species,
                      const pokemon::ItemId* used_items, u32 used_count);
void ApplySpec(savedata::PokemonParam* pokemon, const PokemonSpec& spec);
} // namespace factory