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
#include "battle/constant/mutation_kind.h"
#include "battle/constant/situation_key.h"
#include "battle/constant/weather.h"
#include "battle/native/listener.h"
#include "battle/native/mutation_message.h"
#include "battle/native/pokemon.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/move.h"

namespace battle {
struct Mutation;

class Controller {
public:
  INLINE Mutation* Create(MutationKind kind, UID owner) {
    return ((Mutation*(*)(Controller*, MutationKind, UID))
      address::kControllerCreateMutation)(this, kind, owner);
  }

  INLINE void Apply(Mutation* mutation) {
    ((void(*)(Controller*, Mutation*))
      address::kControllerApplyMutation)(this, mutation);
  }

  STATIC_INLINE void SetMessage(Message* message, u8 p0,
                                MutationMessageId id) {
    ((void(*)(Message*, u8, MutationMessageId))
      address::kControllerSetMessage)(message, p0, id);
  }

  STATIC_INLINE void FillMessageSlot(Message* message, u8 p0,
                                     MutationMessageId id) {
    ((void(*)(Message*, u8, MutationMessageId))
      address::kControllerFillMessageSlot)(message, p0, id);
  }

  INLINE Pokemon* GetPokemon(UID uid) {
    return ((Pokemon*(*)(Controller*, UID))
      address::kControllerGetPokemon)(this, uid);
  }

  INLINE void SetWeather(UID owner, Weather weather, ItemId item,
                         bool infinite) {
    ((void(*)(Controller*, UID, Weather, ItemId, bool))
      address::kControllerSetWeather)(this, owner, weather, item, infinite);
  }

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
