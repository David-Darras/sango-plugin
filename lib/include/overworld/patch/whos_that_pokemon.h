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
 * @file whos_that_pokemon.h
 * @brief The mini-game "Who's that Pokémon?".
 */

#pragma once

#include "common.h"
#include "pokemon/constant/species.h"
#include "script/patch/context.h"

namespace overworld {

/// A character asks the player to name a Pokémon from its shape.
class WhosThatPokemon {
  MAKE_SINGLETON(WhosThatPokemon)

public:
  u32 reward_level = 50; ///< The level of the Pokémon that the player wins.
  u32 max_species = 721; ///< The highest species of the questions.

  static void Initialize();
  static bool IsSilhouetteActive() { return GetInstance().is_silhouette_; }
  static bool AreNamesHidden() { return GetInstance().are_names_hidden_; }

private:
  static void Run(script::Context& script);
  static void Play(script::Context& script);
  static pokemon::SpeciesId PickSpecies();
  static void ShowChoice(script::Context& script, pokemon::SpeciesId species,
                         bool is_hidden);
  static void Reveal(script::Context& script, pokemon::SpeciesId species,
                     const c16* name, bool has_given_up);
  static bool AskName(script::Context& script, c16* name, u32 capacity);
  static bool IsSameName(const c16* typed, const c16* expected);

  bool is_silhouette_ = false;
  bool are_names_hidden_ = false;
};
} // namespace overworld
