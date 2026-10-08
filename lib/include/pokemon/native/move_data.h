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
 * @file move_data.h
 * @brief The data of one move.
 *
 * @see docs/tutorials/04-add-a-move.md (the table of the values)
 */

#pragma once

#include "common.h"
#include "battle/constant/status_condition.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/type.h"

namespace pokemon {
/// The data of one move: type, power, accuracy, effects...
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

  TypeId type; ///< The type.
  u8 category; ///< 0 damage, 1 status, 2 stat change, 3 recovery, 4 damage + status...
  u8 damage_category; ///< 0 status, 1 physical, 2 special.
  u8 power; ///< The power. 0 for a status move.

  u8 accuracy; ///< The accuracy in percent. 101: the move never misses.
  u8 base_pp; ///< The PP.
  s8 priority; ///< The priority, from -7 to +5.
  u8 hit_count; ///< The number of hits.

  StatusCondition effect_id; ///< The status condition that the move gives.
  u8 _1;
  u8 effect_rate; ///< The chance of the status condition, in percent.
  u8 effect_turn_type; ///< How long the status condition stays.

  u8 min_turns; ///< The minimum number of turns of the effect.
  u8 max_turns; ///< The maximum number of turns of the effect.
  u8 crit_stage; ///< The critical hit stage.
  u8 flinch_rate; ///< The chance of a flinch, in percent.

  u16 _0;
  s8 recoil; ///< The recoil in percent of the damage (negative value).
  s8 drain; ///< The HP drain in percent of the damage.

  u8 target; ///< The target (0 one other Pokémon, 3 one opponent, 7 the user...).
  u8 stat_id[3]; ///< The stats that the move changes.
  s8 stat_stages[3]; ///< The number of stages for each stat.
  u8 stat_rate[3]; ///< The chance of each stat change, in percent.

  u32 flags; ///< The flags: contact, sound, punch...
};
} // namespace pokemon
