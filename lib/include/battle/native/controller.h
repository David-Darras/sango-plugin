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
 * @file controller.h
 * @brief The object that changes a battle: mutations, weather, moves.
 *
 * The reactions receive it. See docs/concepts/battle-engine.md.
 */

#pragma once

#include "common.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/mutation_message.h"
#include "battle/constant/weather.h"
#include "battle/native/listener.h"
#include "battle/native/pokemon.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"

namespace battle {
struct Mutation;

/// Changes the battle. A reaction receives it.
class Controller {
public:
  /// Makes a mutation. Cast it to its structure, fill it, then call Apply().
  INLINE Mutation* Create(MutationKind kind, UID owner) {
    return ((Mutation*(*)(Controller*, MutationKind, UID))
      address::kControllerCreateMutation)(this, kind, owner);
  }

  /// Applies a mutation: the battle changes, with its animations and messages.
  INLINE void Apply(Mutation* mutation) {
    ((void(*)(Controller*, Mutation*))
      address::kControllerApplyMutation)(this, mutation);
  }

  /// Sets the text of a message.
  STATIC_INLINE void SetMessage(Message* message, u8 p0,
                                MutationMessageId id) {
    ((void(*)(Message*, u8, MutationMessageId))
      address::kControllerSetMessage)(message, p0, id);
  }

  /// Fills a value of a message.
  STATIC_INLINE void FillMessageSlot(Message* message, u8 p0,
                                     MutationMessageId id) {
    ((void(*)(Message*, u8, MutationMessageId))
      address::kControllerFillMessageSlot)(message, p0, id);
  }

  /// Returns the battle data of a Pokémon.
  INLINE Pokemon* GetPokemon(UID uid) {
    return ((Pokemon*(*)(Controller*, UID))
      address::kControllerGetPokemon)(this, uid);
  }

  /**
   * @brief Changes the weather.
   * @param owner The Pokémon that changes the weather.
   * @param weather The new weather.
   * @param item The item that makes the weather last 8 turns (for example
   *        Damp Rock for the rain), or ItemId::kNone.
   * @param infinite true: the weather never stops. false: 5 turns (8 turns
   *        when the owner holds `item`).
   */
  INLINE void SetWeather(UID owner, Weather weather, ItemId item,
                         bool infinite) {
    ((void(*)(Controller*, UID, Weather, ItemId, bool))
      address::kControllerSetWeather)(this, owner, weather, item, infinite);
  }

  /// Makes a Pokémon use a move.
  INLINE void ExecuteMove(Pokemon* attacker, MoveId move, u8 target = 0) {
    union {
      u32 raw;

      struct {
        u32 kind : 4;
        u32 target : 4;
        u32 move_id : 16;
        u32 rotation_direction : 3;
        u32 has_move_info : 1;
        u32 mega_evolve : 1;
        u32 _padding : 3;
      } fight;
    } action;
    action.raw = 0;
    action.fight.kind = 1;
    action.fight.target = target;
    action.fight.move_id = static_cast<u16>(move);

    ((void(*)(Controller*, Pokemon*, void*, u32, s32))
      address::kControllerExecuteMove)(this, attacker, &action, 0, 0);
  }
};
} // namespace battle
