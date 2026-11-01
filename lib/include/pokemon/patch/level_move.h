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
 * @file level_move.h
 * @brief A move of a learnset and an evolution, for pokemon::Species.
 *
 * The file has no game structure: the tests of tests/ compile it on a
 * computer (see core/maker_reader.h).
 */

#pragma once

#include "core/types.h"
#include "pokemon/constant/evolution_method.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/species.h"

namespace pokemon {

/// A move of a learnset: the level, then the move.
struct LevelMove {
  u8 level;
  MoveId move;

  constexpr LevelMove() : level(1), move(MoveId::kNone) {}
  constexpr LevelMove(u8 level, MoveId move) : level(level), move(move) {}
};

/**
 * @brief An evolution: the method, its value, then the species after the
 * evolution.
 *
 * The value depends on the method: a level (kLevelUp), an item (kItem,
 * kTradeItem...), a move (kKnowMove) or a species (kPartyPokemon). Give 0
 * when the method has no value.
 */
struct EvolutionStep {
  EvolutionMethod method;
  u16 value;
  SpeciesId species;

  constexpr EvolutionStep()
    : method(EvolutionMethod::kNone), value(0), species(SpeciesId::kNone) {}
  constexpr EvolutionStep(EvolutionMethod method, u16 value,
                          SpeciesId species)
    : method(method), value(value), species(species) {}
  /// An evolution with an item: EvolutionStep(EvolutionMethod::kItem,
  /// ItemId::kThunderStone, SpeciesId::kRaichu).
  constexpr EvolutionStep(EvolutionMethod method, ItemId item,
                          SpeciesId species)
    : method(method), value(static_cast<u16>(item)), species(species) {}
  /// An evolution with a move: EvolutionStep(EvolutionMethod::kKnowMove,
  /// MoveId::kAncientPower, SpeciesId::kYanmega).
  constexpr EvolutionStep(EvolutionMethod method, MoveId move,
                          SpeciesId species)
    : method(method), value(static_cast<u16>(move)), species(species) {}
};

} // namespace pokemon
