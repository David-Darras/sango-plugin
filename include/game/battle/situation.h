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
#include "constant/situation_key.h"

namespace battle {
class Situation {
public:
  STATIC_INLINE void Begin() {
    ((void(*)())0x007648D0)();
  }

  STATIC_INLINE void End() {
    ((void(*)())0x007645CC)();
  }

  STATIC_INLINE void Set(SituationKey key, s32 value) {
    ((void(*)(SituationKey, s32))0x0076426C)(key, value);
  }

  STATIC_INLINE s32 Get(SituationKey key) {
    return ((s32(*)(SituationKey))0x00764668)(key);
  }
};
}