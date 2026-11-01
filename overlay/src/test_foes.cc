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
 * @file test_foes.cc
 * @brief Test help: each wild Pokémon becomes a strong attacker with damaging
 * moves, to test the items and the reactions of a battle.
 */

#include "custom_battle.h"
#include "pokemon/edit_pokemon.h"

#include "battle/native/config.h"
#include "battle/patch/setup.h"
#include "overworld/native/wild_pokemon.h"
#include "overworld/patch/wild_encounter.h"
#include "pokemon/native/core_data.h"
#include "savedata/native/pokemon_param.h"
#include "savedata/native/pokemon_team.h"

namespace battle {
namespace {

// The foe is a Garchomp of level 70 with four moves that hit at once (no move
// of two turns, like Dig) and an ability with no effect (Illuminate), so no
// window of ability opens. The game chooses the moves from the level, so the
// function sets them again when the battle is ready.
constexpr MoveId kFoeMoves[4] = {MoveId::kEarthquake, MoveId::kOutrage,
                                 MoveId::kStoneEdge, MoveId::kFireFang};

// The game calls this function after it selects the wild Pokemon of a battle.
// `count` is 5 for a horde: all of them change.
void OnWildPokemon(MapId map_id, overworld::WildPokemon* pokemons, u32 count) {
  for (u32 i = 0; i < count; i++) {
    overworld::WildPokemon& foe = pokemons[i];
    foe.species = SpeciesId::kGarchomp;
    foe.form = FormId::kNormal;
    foe.level = 70;
    foe.ability = AbilityId::kIlluminate;
    for (u32 move = 0; move < 4; move++) foe.moves[move] = kFoeMoves[move];
  }
}

// The battle is ready: the moves and the ability go in the data of the foes.
void OnWildBattle(Config& config) {
  savedata::PokemonTeam* team = config.pokemon_teams[1];
  for (u32 i = 0; i < team->count; i++) {
    pokemon::EditPokemon foe(*team->pokemons[i]);
    foe->ability = AbilityId::kIlluminate;
    for (u32 move = 0; move < 4; move++) {
      foe->moves[move] = kFoeMoves[move];
      foe->pp[move] = 10;
    }
  }
}

} // namespace

void RegisterTestFoes() {
  overworld::WildEncounter::GetInstance().on_wild_pokemon = OnWildPokemon;
  Setup::GetInstance().on_wild_battle = OnWildBattle;
}

} // namespace battle
