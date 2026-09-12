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
#include "listener.h"
#include "constant/priority_tier.h"

namespace battle {
class Broadcaster {
public:
  STATIC_INLINE Listener* Register(ListenerSource source,
                                   u32 source_id,
                                   PriorityTier base_priority,
                                   u32 priority_tiebreak,
                                   UID owner_id,
                                   ReactionTable* reaction_table,
                                   u32 reaction_count) {
    return ((Listener*(*)(ListenerSource, u32, PriorityTier, u32, u8,
                          ReactionTable*, u32))0x00743EFC)(
        source, source_id, base_priority, priority_tiebreak, owner_id.value,
        reaction_table, reaction_count);
  }

  STATIC_INLINE Listener* FindListener(ListenerSource source, u8 owner_id) {
    return ((Listener*(*)(ListenerSource, u8))0x007442FC)(source, owner_id);
  }

  STATIC_INLINE void Unregister(Listener* listener) {
    ((void(*)(Listener*))0x00744108)(listener);
  }
};
}