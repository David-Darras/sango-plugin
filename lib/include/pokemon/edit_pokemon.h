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
 * @file edit_pokemon.h
 * @brief Changes and reads the data of a Pokémon without Decrypt() and
 * Encrypt().
 *
 * The data of a Pokémon is encrypted. pokemon::EditPokemon decrypts it while
 * the object lives. At the end of the block, it encrypts the data again and
 * calculates the stats (level, HP) again.
 *
 * @code
 * savedata::PokemonParam* first = savedata::PokemonTeam::GetInstance().Get(0);
 * if (first == nullptr) return;
 * {
 *   pokemon::EditPokemon edit(*first);
 *   edit->item = ItemId::kLeftovers;
 *   edit.SetMoves(MoveId::kSurf, MoveId::kIceBeam);
 * } // The data is encrypted here.
 *
 * // Read only: a decrypted copy. The Pokémon does not change.
 * const pokemon::CoreData copy = pokemon::ReadPokemon(*first->core);
 * @endcode
 */

#pragma once

#include <cstring>

#include "common.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/data_accessor.h"
#include "pokemon/native/move_data.h"
#include "savedata/native/pokemon_param.h"

namespace pokemon {

/**
 * @brief Decrypts the data of a Pokémon while the object lives.
 *
 * Use `->` to change the data (CoreData). At the end of the block, the
 * destructor encrypts the data, then calculates the stats again. A `return`
 * in the block is safe.
 *
 * It changes a Pokémon of the party or of a battle (savedata::PokemonParam),
 * or the data of a Pokémon of a box (CoreData).
 *
 * Do not make two EditPokemon objects for the same Pokémon at the same time.
 */
class EditPokemon {
public:
  /**
   * @param pokemon The Pokémon to change.
   * @param update_stats true: at the end, the game calculates the level, the
   *        HP and the stats again (UpdateRuntimeData()). Give false when you
   *        restore the data, or for a Pokémon of a battle that the game
   *        calculates itself.
   */
  explicit EditPokemon(savedata::PokemonParam& pokemon,
                       bool update_stats = true)
    : param_(&pokemon),
      core_(pokemon.core),
      accessor_(pokemon.accessor),
      update_stats_(update_stats) {
    accessor_->Decrypt();
  }

  /// Changes the data of a Pokémon of a box. A Pokémon of a box has no
  /// calculated data.
  explicit EditPokemon(CoreData& box_data)
    : param_(nullptr),
      core_(&box_data),
      accessor_(&own_accessor_),
      update_stats_(false) {
    own_accessor_.Initialize(core_, nullptr);
    accessor_->Decrypt();
  }

  ~EditPokemon() {
    accessor_->Encrypt();
    if (update_stats_ && param_ != nullptr) param_->UpdateRuntimeData();
  }

  EditPokemon(const EditPokemon&) = delete;
  EditPokemon& operator=(const EditPokemon&) = delete;

  /// Gives the decrypted data.
  CoreData* operator->() const { return core_; }
  /// Gives the decrypted data.
  CoreData& operator*() const { return *core_; }
  /// Gives the calculated data (level, HP, stats), or null for a Pokémon of a
  /// box. The game encrypts it with the data: read it here.
  RuntimeData* GetRuntimeData() const {
    return param_ != nullptr ? param_->runtime : nullptr;
  }

  /// Sets the four moves, with the PP of each move and no PP Up.
  void SetMoves(MoveId move1, MoveId move2 = MoveId::kNone,
                MoveId move3 = MoveId::kNone, MoveId move4 = MoveId::kNone) {
    CoreData& core = *core_;
    core.SetMoves(move1, move2, move3, move4);
    for (u32 i = 0; i < 4; i++) {
      core.pp[i] = core.moves[i] == MoveId::kNone
                     ? 0
                     : MoveData::GetInstance(core.moves[i]).base_pp;
      core.pp_up_count[i] = 0;
    }
  }

  /// Changes the species and the form. The nickname becomes the name of the
  /// new species.
  void SetSpecies(SpeciesId species, FormId form = FormId::kNormal) {
    core_->species = species;
    core_->form = form;
    core_->ResetNickname();
  }

private:
  savedata::PokemonParam* param_;
  CoreData* core_;
  DataAccessor* accessor_;
  DataAccessor own_accessor_;
  bool update_stats_;
};

/**
 * @brief Changes a Pokémon with one function: Edit(pokemon, function).
 *
 * @code
 * pokemon::Edit(*first, [](pokemon::CoreData& core) {
 *   core.item = ItemId::kLeftovers;
 * });
 * @endcode
 */
template <typename Change>
void Edit(savedata::PokemonParam& pokemon, Change change) {
  EditPokemon edit(pokemon);
  change(*edit);
}

/// Returns a decrypted copy of encrypted data (a Pokémon of the party or of a
/// box). The data does not change.
inline CoreData ReadPokemon(const CoreData& encrypted) {
  CoreData copy;
  std::memcpy(&copy, &encrypted, sizeof(copy));
  DataAccessor accessor;
  accessor.Initialize(&copy, nullptr);
  accessor.Decrypt();
  return copy;
}

} // namespace pokemon
