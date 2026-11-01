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
 * @file pokemon_team.h
 * @brief The party of the player.
 */

#pragma once
#include "common.h"
#include "core/native/data_manager.h"
#include "pokemon/edit_pokemon.h"
#include "savedata/native/pokemon_param.h"
#include "pokemon/address.h"

namespace savedata {

/**
 * @brief The party of the player (6 Pokémon at most).
 *
 * @code
 * auto& team = savedata::PokemonTeam::GetInstance();
 * for (u32 i = 0; i < team.count; i++) { ... team.pokemons[i] ... }
 *
 * // The first Pokémon, or null when the party is empty:
 * savedata::PokemonParam* first = team.Get(0);
 * @endcode
 */
struct PokemonTeam {
  SINGLETON(PokemonTeam)
  STATIC_INLINE PokemonTeam& GetInstance() {
    return core::DataManager::GetInstance().GetPokemonTeam();
  }

  /// Returns the Pokémon of a slot (0 to count - 1), or null when the slot
  /// is empty.
  PokemonParam* Get(u32 slot) const {
    return slot < count ? pokemons[slot] : nullptr;
  }

  /// Returns the highest level of the party.
  u8 GetMaxLevel() const {
    u8 max_level = 1;
    for (u32 i = 0; i < count; i++) {
      const pokemon::CoreData core = pokemon::ReadPokemon(*pokemons[i]->core);
      u8 level = pokemon::Utils::GetLevelFromExperience(core.species, core.form,
                                                        core.experience);
      if (level > max_level) {
        max_level = level;
      }
    }
    return max_level;
  }

  /// Heals all the Pokémon of the party.
  INLINE void HealAllPokemons() {
    core::GameFunction<void(PokemonTeam*)>::Call(
        pokemon::address::kHealTeam, this);
  }

  /// Removes the fainted Pokémon from the party.
  void ThrowAllDeadPokemons() {
    bool is_dead[6] = {0, 0, 0, 0, 0, 0};
    for (u32 i = 0; i < count; i++) {
      // The game encrypts the HP with the data.
      pokemon::EditPokemon edit(*pokemons[i], false);
      is_dead[i] = edit.GetRuntimeData()->hp == 0;
    }
    const u8 c = count;
    for (u8 i = 0; i < c; i++) {
      u8 end = c - i - 1;
      if (is_dead[end]) {
        core::GameFunction<void(PokemonTeam*, u8)>::Call(
            pokemon::address::kRemovePokemonFromTeam, this, end);
      }
    }
  }

  static constexpr u32 kMaxSlots = 6;

  PokemonParam* pokemons[kMaxSlots];
  u8 count; ///< The number of Pokémon in the party.
  u8 _0[3];
};

} // namespace savedata
