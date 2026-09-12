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

#include <types.h>

/// Bitmask of AI behaviours enabled for a trainer (battle::TrainerData::ai_flags).
enum class BattleAi : u32 {
  kCasual = 1u << 0,
  kCompetitive = 1u << 1,
  kStrategist = 1u << 2,
  kMulti = 1u << 7,
  kHorde = 1u << 14,
};

constexpr BattleAi operator|(BattleAi a, BattleAi b) {
  return static_cast<BattleAi>(static_cast<u32>(a) | static_cast<u32>(b));
}

constexpr BattleAi operator&(BattleAi a, BattleAi b) {
  return static_cast<BattleAi>(static_cast<u32>(a) & static_cast<u32>(b));
}

inline BattleAi& operator|=(BattleAi& a, BattleAi b) {
  a = a | b;
  return a;
}

inline BattleAi& operator&=(BattleAi& a, BattleAi b) {
  a = a & b;
  return a;
}
