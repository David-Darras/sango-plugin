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
 * @file pokemon_param.h
 * @brief A Pokémon of the party.
 */

#pragma once

#include "common.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/data_accessor.h"
#include "pokemon/native/runtime_data.h"

namespace savedata {

/**
 * @brief A Pokémon of the party: its saved data, its calculated data and
 * its encryption.
 *
 * @code
 * pokemon->accessor->Decrypt();
 * pokemon->core->moves[0] = MoveId::kSurf;
 * pokemon->accessor->Encrypt();
 * @endcode
 */
struct PokemonParam {
  void* vtable;
  pokemon::CoreData* core; ///< The saved data (encrypted).
  pokemon::RuntimeData* runtime; ///< The calculated data: level, HP, stats.
  pokemon::DataAccessor* accessor; ///< Decrypts and encrypts `core`.

  /// Calculates the level, the HP and the stats again. Call it after a change of the saved data.
  INLINE void UpdateRuntimeData() {
    ((void(*)(PokemonParam*))pokemon::address::kUpdateRuntimeData)(this);
  }

  /// Sets the name of the species as the nickname.
  INLINE void ResetNickname() {
    ((void(*)(PokemonParam*))pokemon::address::kResetNickname)(this);
  }
};

} // namespace savedata
