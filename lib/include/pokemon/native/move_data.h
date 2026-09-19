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
#include "battle/constant/status_condition.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/type.h"

namespace pokemon {
struct MoveData {
#ifdef GAME_XY
  STATIC_INLINE MoveData& GetInstance(const MoveId move) {
    static struct {
      void* heap;
      u16 move;
      u16 _0;
      MoveData* data;
    } accessor = {nullptr, 0, 0, nullptr};
    static u32 buffer[16];
    accessor.data = (MoveData*)buffer;
    ((void (*)(void*, u16))address::kLoadMoveData)(&accessor,
                                                    static_cast<u16>(move));
    return *accessor.data;
  }
#else
  STATIC_INLINE MoveData* GetTable() {
    return (MoveData*)READ32(address::kMoveDataTable);
  }

  STATIC_INLINE MoveData& GetInstance(const MoveId move) {
    return GetTable()[static_cast<u16>(move)];
  }
#endif

  TypeId type;
  u8 category;
  u8 damage_category;
  u8 power;

  u8 accuracy;
  u8 base_pp;
  s8 priority;
  u8 hit_count;

  StatusCondition effect_id;
  u8 _1;
  u8 effect_rate;
  u8 effect_turn_type;

  u8 min_turns;
  u8 max_turns;
  u8 crit_stage;
  u8 flinch_rate;

  u16 _0;
  s8 recoil;
  s8 drain;

  u8 target;
  u8 stat_id[3];
  s8 stat_stages[3];
  u8 stat_rate[3];

  u32 flags;
};
} // namespace pokemon
