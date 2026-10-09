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
 * @file gift_pokemon.cc
 * @brief Changes the gift Pokémon.
 *
 * The declarations are in overworld/patch/gift_pokemon.h.
 */

#include "overworld/patch/gift_pokemon.h"
#include "core/hook.h"
#include "pokemon/native/gift_pokemon_data.h"
#include "core/utils.h"

namespace overworld {

namespace {
core::Hook<s32(u32*, u32*)> script_add_pokemon_to_team_hook;
} // namespace

void GiftPokemon::Initialize() {
  script_add_pokemon_to_team_hook.Install(
      pokemon::address::kScriptAddPokemonToTeam, ScriptAddPokemonToTeamHook,
      address::kVtable);
}

s32 GiftPokemon::AddPokemonWithoutRandomizer(u32* a1, u32* a2) {
  if (script_add_pokemon_to_team_hook.IsInitialized()) {
    return script_add_pokemon_to_team_hook(a1, a2);
  }
  return ((s32(*)(u32*, u32*))pokemon::address::kScriptAddPokemonToTeam)(
      a1, a2);
}

void GiftPokemon::RandomizeSpecies(u32 idx) {
  auto& entry = pokemon::GiftPokemonData::GetInstance(idx);
  entry.species = core::Utils::GetRandomEnum<SpeciesId>();
  entry.form = FormId::kNormal;
}

s32 GiftPokemon::ScriptAddPokemonToTeamHook(u32* a1, u32* a2) {
  if (GetInstance().randomize_species) RandomizeSpecies(a2[1]);
  return script_add_pokemon_to_team_hook(a1, a2);
}

} // namespace overworld
