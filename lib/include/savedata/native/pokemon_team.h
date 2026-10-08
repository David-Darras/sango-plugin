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
#include "savedata/native/pokemon_param.h"

namespace savedata {

/**
 * @brief The party of the player (6 Pokémon at most).
 *
 * @code
 * auto& team = savedata::PokemonTeam::GetInstance();
 * for (u32 i = 0; i < team.count; i++) { ... team.pokemons[i] ... }
 * @endcode
 */
struct PokemonTeam {
  SINGLETON(PokemonTeam)
  STATIC_INLINE PokemonTeam& GetInstance() {
    return core::DataManager::GetInstance().GetPokemonTeam();
  }

  /// Returns the highest level of the party.
  u8 GetMaxLevel() const {
    u8 max_level = 1;
    for (u32 i = 0; i < count; i++) {
      pokemons[i]->accessor->Decrypt();
      SpeciesId species = pokemons[i]->core->species;
      FormId form = pokemons[i]->core->form;
      u32 experience = pokemons[i]->core->experience;
      pokemons[i]->accessor->Encrypt();
      u8 level =
          pokemon::Utils::GetLevelFromExperience(species, form, experience);
      if (level > max_level) {
        max_level = level;
      }
    }
    return max_level;
  }

  /// Heals all the Pokémon of the party.
  INLINE void HealAllPokemons() {
    ((void(*)(PokemonTeam*))pokemon::address::kHealTeam)(this);
  }

  /// Removes the fainted Pokémon from the party.
  void ThrowAllDeadPokemons() {
    bool is_dead[6] = {0, 0, 0, 0, 0, 0};
    for (u32 i = 0; i < count; i++) {
      pokemons[i]->accessor->Decrypt();
      if (pokemons[i]->runtime->hp == 0) {
        is_dead[i] = true;
      }
      pokemons[i]->accessor->Encrypt();
    }
    const u8 c = count;
    for (u8 i = 0; i < c; i++) {
      u8 end = c - i - 1;
      if (is_dead[end]) {
        ((void(*)(PokemonTeam*, u8))
          pokemon::address::kRemovePokemonFromTeam)(this, end);
      }
    }
  }

  static constexpr u32 kMaxSlots = 6;

  PokemonParam* pokemons[kMaxSlots];
  u8 count; ///< The number of Pokémon in the party.
  u8 _0[3];
};

} // namespace savedata
