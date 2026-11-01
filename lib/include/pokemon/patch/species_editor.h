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
 * @file species_editor.h
 * @brief Changes the data of a species: stats, types, abilities, TMs,
 * learnset and evolutions.
 *
 * @code
 * const pokemon::LevelMove kPikachuMoves[] = {
 *     {1, MoveId::kThunderShock},
 *     {5, MoveId::kQuickAttack},
 *     {20, MoveId::kThunderbolt},
 * };
 * const pokemon::EvolutionStep kPikachuEvolutions[] = {
 *     {EvolutionMethod::kLevelUp, 30, SpeciesId::kRaichu},
 * };
 *
 * pokemon::Species(SpeciesId::kPikachu)
 *     .WithStats(45, 70, 40, 60, 50, 100)
 *     .WithTypes(TypeId::kElectric, TypeId::kFairy)
 *     .WithAbilities(AbilityId::kStatic, AbilityId::kStatic,
 *                    AbilityId::kLightningRod)
 *     .WithTm(24)
 *     .WithLearnset(kPikachuMoves)
 *     .WithEvolutions(kPikachuEvolutions);
 * @endcode
 *
 * Call it in SetUpHack(). The library applies the changes when the data of
 * the species is ready: the new species and the new forms of the library
 * exist only after the start.
 *
 * pokemon::NewSpecies() adds a species to the game (Alpha Sapphire only):
 *
 * @code
 * const SpeciesId kSangoat =
 *     pokemon::NewSpecies(u"Sangoat", SpeciesId::kSkiddo, SpeciesId::kGogoat)
 *         .WithStats(80, 95, 70, 60, 75, 85)
 *         .WithTypes(TypeId::kGrass, TypeId::kRock)
 *         .GetSpecies();
 * @endcode
 *
 * @see docs/tutorials/16-change-a-species.md
 * @see docs/tutorials/12-add-a-new-pokemon.md
 */

#pragma once

#include "common.h"
#include "pokemon/constant/ability.h"
#include "pokemon/constant/evolution_method.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/species.h"
#include "pokemon/constant/type.h"
#include "pokemon/patch/level_move.h"

namespace pokemon {
struct EvolutionData;

/**
 * @brief Changes the data of one species or of one form.
 *
 * Each With function changes one part. The parts that you do not change keep
 * the data of the game.
 */
class Species {
public:
  /// The number of moves of a learnset.
  static constexpr u32 kMaxLearnsetMoves = 24;
  /// The number of evolutions of a species.
  static constexpr u32 kMaxEvolutions = 8;

  explicit Species(SpeciesId species, FormId form = FormId::kNormal)
    : species_(species), form_(form) {}

  /// Returns the species that this object changes.
  SpeciesId GetSpecies() const { return species_; }

  /// Sets the base stats, in the order of the summary: HP, Attack, Defense,
  /// Sp. Atk, Sp. Def, Speed.
  Species& WithStats(u8 hp, u8 attack, u8 defense, u8 sp_attack,
                     u8 sp_defense, u8 speed);
  /// Sets the types. Give the same type two times for one type.
  Species& WithTypes(TypeId first, TypeId second);
  /// Sets one type.
  Species& WithType(TypeId type) { return WithTypes(type, type); }
  /// Sets the two abilities and the hidden ability.
  Species& WithAbilities(AbilityId first, AbilityId second, AbilityId hidden);
  /// Sets the catch rate (3 for a legendary Pokémon, 255 for the easiest).
  Species& WithCatchRate(u8 rate);
  /// Lets the species learn a TM (1 to 100), or stops it.
  Species& WithTm(u8 number, bool can_learn = true);
  /// Lets the species learn an HM (1 to 7, or 1 to 5 in X), or stops it.
  Species& WithHm(u8 number, bool can_learn = true);

  /// Replaces the learnset: the moves that the species learns when its level
  /// increases. 24 moves at most. The table must stay in memory: use a global
  /// `const` array.
  Species& WithLearnset(const LevelMove* moves, u32 count);
  template <u32 N>
  Species& WithLearnset(const LevelMove (&moves)[N]) {
    return WithLearnset(moves, N);
  }

  /// Replaces the evolutions. 8 at most. The table must stay in memory.
  /// The evolutions are for all the forms of the species.
  Species& WithEvolutions(const EvolutionStep* steps, u32 count);
  template <u32 N>
  Species& WithEvolutions(const EvolutionStep (&steps)[N]) {
    return WithEvolutions(steps, N);
  }

private:
  SpeciesId species_;
  FormId form_;
};

/**
 * @brief Adds a new species to the game. Alpha Sapphire only.
 *
 * Call it in SetUpHack(). The first new species gets the id
 * kFirstNewSpeciesId (1021), the next one gets 1022, and so on (see
 * NewSpeciesId()). 64 new species at most.
 *
 * The new species starts with the data and the learnset of @a data_from. It
 * has no evolution. Change its data with the With functions of the result.
 *
 * @param name The name of the species. The text must stay in memory: use a
 *        text in quotes (u"Sangoat").
 * @param data_from The species that gives the first data and the learnset.
 * @param model The species that gives its 3D model. SpeciesId::kNone: the
 *        model of @a data_from.
 * @param model_form The form of the model.
 * @return The changes of the new species. Its species is SpeciesId::kNone
 *         when the game cannot add it (see the problems screen).
 */
Species NewSpecies(const c16* name, SpeciesId data_from,
                   SpeciesId model = SpeciesId::kNone,
                   FormId model_form = FormId::kNormal);

/// The id of the first species of NewSpecies().
constexpr u16 kFirstNewSpeciesId = 1021;

/// Returns the id that the call number @a number of NewSpecies() gives (0 for
/// the first call). Use it to write the id in global tables.
constexpr SpeciesId NewSpeciesId(u32 number) {
  return static_cast<SpeciesId>(kFirstNewSpeciesId + number);
}

/// The changes of pokemon::Species. The library uses this class.
class SpeciesChanges {
  MAKE_SINGLETON(SpeciesChanges)

public:
  /// The kinds of changes of the species data.
  enum class Kind : u8 { kStats, kTypes, kAbilities, kCatchRate, kTm };

  /// One change of the species data.
  struct Change {
    SpeciesId species;
    FormId form;
    Kind kind;
    u8 values[6];
  };

  static constexpr u32 kMaxChanges = 1024;
  static constexpr u32 kMaxLearnsets = 256;
  static constexpr u32 kMaxEvolutionLists = 256;

  /// Applies a change now, or after the start when the data is not ready.
  static void Add(const Change& change);
  /// Applies the changes that wait. pokemon::SpeciesTable calls it when the
  /// data of the species is ready.
  static void ApplyPending();
  /// Returns true when the data of the species is ready.
  static bool IsReady();

  static bool AddLearnset(SpeciesId species, FormId form,
                          const LevelMove* moves, u32 count);
  static bool AddEvolutions(SpeciesId species, const EvolutionStep* steps,
                            u32 count);

  /// Removes the learnsets and the evolution lists whose table makes
  /// `is_removed(table)` true (the address of the LevelMove or of the
  /// EvolutionStep table). core::DataFiles uses it to read the files again.
  static void RemoveLists(bool (*is_removed)(const void* table));

  /// Returns the number of evolution lists of the product.
  static u32 GetEvolutionListCount() { return GetInstance().evolution_count_; }
  /// Gives the evolution list number @a index of the product. Returns false
  /// when @a index is too big.
  static bool GetEvolutionList(u32 index, SpeciesId* species,
                               const EvolutionStep** steps, u32* count);

  /// Replaces the learnset that the game loaded. The library calls it.
  static void PatchLearnset(SpeciesId species, FormId form);
  /// Replaces the evolutions that the game loaded. The library calls it.
  static void PatchEvolutions(SpeciesId species, EvolutionData& data);

private:
  struct Learnset {
    SpeciesId species;
    FormId form;
    const LevelMove* moves;
    u32 count;
  };
  struct EvolutionList {
    SpeciesId species;
    const EvolutionStep* steps;
    u32 count;
  };

  static void Apply(const Change& change);

  Change pending_[kMaxChanges] = {};
  u32 pending_count_ = 0;
  Learnset learnsets_[kMaxLearnsets] = {};
  u32 learnset_count_ = 0;
  EvolutionList evolutions_[kMaxEvolutionLists] = {};
  u32 evolution_count_ = 0;
};

} // namespace pokemon
