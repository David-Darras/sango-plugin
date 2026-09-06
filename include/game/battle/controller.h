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
#include "constant/mutation_kind.h"
#include "constant/situation_key.h"
#include "controller.h"
#include "listener.h"
#include "mutation_message.h"
#include "pokemon.h"
#include "game/constant/item.h"
#include "game/constant/move.h"
#include "game/constant/weather.h"

namespace battle {
struct Mutation;

class Controller {
public:
  INLINE Mutation* Create(MutationKind kind, UID owner) {
    return ((Mutation*(*)(Controller*, MutationKind, UID))0x00762CC0)
        (this, kind, owner);
  }

  INLINE void Apply(Mutation* mutation) {
    ((void(*)(Controller*, Mutation*))0x0076088C)(this, mutation);
  }

  STATIC_INLINE void SetMessage(Message* message, u8 p0, MutationMessageId id) {
    ((void(*)(Message*, u8, MutationMessageId))0x00744210)(message, p0, id);
  }

  STATIC_INLINE void FillMessageSlot(Message* message, u8 p0,
                                     MutationMessageId id) {
    ((void(*)(Message*, u8, MutationMessageId))0x007441D8)(message, p0, id);
  }

  INLINE Pokemon* GetPokemon(UID uid) {
    return ((Pokemon*(*)(Controller*, UID))0x0074BC4C)(this, uid);
  }

  INLINE void SetWeather(UID owner, BattleWeather weather, ItemId item,
                        bool infinite) {
    ((void(*)(Controller*, UID, BattleWeather, ItemId, bool))0x00741CC0)(
        this, owner, weather, item, infinite);
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

    ((void(*)(Controller*, Pokemon*, void*, u32, s32))0x0070EC48)(
        this, attacker, &action, 0, 0);
  }
};
}