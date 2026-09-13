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
#include "pokemon/native/data_accessor.h"
#include "savedata/native/savedata.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/runtime_data.h"

class pokemon::DataAccessor;

namespace savedata {
struct PokemonParam {
  void* vtable;
  pokemon::CoreData* core;
  pokemon::RuntimeData* runtime;
  pokemon::DataAccessor* accessor;

  INLINE void UpdateRuntimeData() {
    ((void(*)(PokemonParam*))pokemon::address::kUpdateRuntimeData)(this);
  }

  INLINE void ResetNickname() {
    ((void(*)(PokemonParam*))pokemon::address::kResetNickname)(this);
  }
};

struct PokemonTeam {
  SINGLETON(PokemonTeam)
  STATIC_INLINE PokemonTeam& GetInstance() {
    return core::DataManager::GetInstance().GetPokemonTeam();
  }

  u8 GetMaxLevel() const {
    u8 max_level = 1;
    for (u32 i = 0; i < count; i++) {
      pokemons[i]->accessor->Decrypt();
      SpeciesId species = pokemons[i]->core->species;
      Form form = pokemons[i]->core->form;
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

  INLINE void HealAllPokemons() {
    ((void(*)(PokemonTeam*))pokemon::address::kHealTeam)(this);
  }

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
  u8 count;
  u8 _0[3];
};

// struct PokemonTeam {
//   static PokemonTeam& GetInstance() {
//     return SaveData::GetInstance().GetPokemonTeam();
//   }
//   static constexpr u32 kMaxSlots = 6;
//
//   void* vtable;
//   struct PokemonData {
//     pokemon::CoreData core;
//     pokemon::RuntimeData runtime;
//   } pokemons[kMaxSlots];
//   u8 count;
//   u8 pokemon_amie_index;
// };
} // namespace savedata