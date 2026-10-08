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
 * @file situation.h
 * @brief The values of the current moment of the battle engine.
 */

#pragma once
#include "common.h"
#include "battle/constant/situation_key.h"

namespace battle {
/// The values of the current moment: the Pokémon, the move, the targets...
class Situation {
public:
  /// Starts a nested situation.
  STATIC_INLINE void Begin() {
    ((void(*)())battle::address::kSituationBegin)();
  }

  /// Ends a nested situation.
  STATIC_INLINE void End() {
    ((void(*)())battle::address::kSituationEnd)();
  }

  /// Sets a value of the current moment.
  STATIC_INLINE void Set(SituationKey key, s32 value) {
    ((void(*)(SituationKey, s32))battle::address::kSituationSet)(key, value);
  }

  /// Returns a value of the current moment.
  STATIC_INLINE s32 Get(SituationKey key) {
    return ((s32(*)(SituationKey))battle::address::kSituationGet)(key);
  }
};
}