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
 * @file broadcaster.h
 * @brief Registers the listeners of the battle engine.
 */

#pragma once

#include "common.h"
#include "battle/constant/priority_tier.h"
#include "battle/native/listener.h"

namespace battle {
/// Registers and removes the listeners of the battle engine.
class Broadcaster {
public:
  /**
   * @brief Registers a listener with a reaction table.
   * @param source What registers the listener (an ability, a move...).
   * @param source_id The id of the ability, of the move...
   * @param base_priority The order of the listener.
   * @param priority_tiebreak The order inside the same tier.
   * @param owner_id The owner of the listener.
   * @param reaction_table The reactions.
   * @param reaction_count The number of reactions.
   */
  STATIC_INLINE Listener* Register(ListenerSource source, u32 source_id,
                                   PriorityTier base_priority,
                                   u32 priority_tiebreak, UID owner_id,
                                   ReactionTable* reaction_table,
                                   u32 reaction_count) {
    return ((Listener*(*)(ListenerSource, u32, PriorityTier, u32, u8,
                          ReactionTable*, u32))
      address::kBroadcasterRegister)(source, source_id, base_priority,
                                     priority_tiebreak, owner_id.value,
                                     reaction_table, reaction_count);
  }

  /// Returns the listener of an owner, or null.
  STATIC_INLINE Listener* FindListener(ListenerSource source, u8 owner_id) {
    return ((Listener*(*)(ListenerSource, u8))
      address::kBroadcasterFindListener)(source, owner_id);
  }

  /// Removes a listener.
  STATIC_INLINE void Unregister(Listener* listener) {
    ((void(*)(Listener*))address::kBroadcasterUnregister)(listener);
  }
};
} // namespace battle
