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
 * @file listener.h
 * @brief A listener of the battle engine.
 */

#pragma once
#include <types.h>

#include "battle/constant/listener_source.h"
#include "battle/constant/moment_kind.h"
#include "battle/native/reaction_table.h"
#include "battle/native/uid.h"

namespace battle {
struct Listener;
class Controller;

/// A function that can stop a reaction before it runs.
typedef bool (*SkipPredicate)(Listener* self, Controller* controller,
                              ListenerSource source, MomentKind moment,
                              u16 source_id, UID owner_id);

/// A listener: it waits for some moments and calls its reactions. See docs/concepts/battle-engine.md.
struct Listener {
  Listener* previous_listener;
  Listener* next_listener;
  /// The pairs {moment, reaction} of the listener.
  const ReactionTable* reaction_table;
  /// An optional function that can stop a reaction before it runs.
  SkipPredicate skip_predicate;
  /// What registered the listener (ability, move, item...).
  ListenerSource source;
  u32 dispatch_priority; ///< The order of the listeners at the same moment.

  /// The nesting depth when the listener was registered.
  u32 created_at_depth : 16;
  u32 reaction_count : 8; ///< The number of entries in reaction_table.
  /// true while a reaction runs: it cannot run again inside itself.
  u32 is_reacting : 1;
  u32 is_paused : 1;
  /// Reacts only to the moment when an item is used.
  u32 is_temporary_item_listener : 1;
  u32 pending_removal : 1;
  u32 allow_reentry : 1; ///< Lets the listener react again inside a reaction.
  u32 is_active : 1; ///< true when this listener slot is in use.
  /// Paused because its Pokémon is not in front (Rotation Battle).
  u32 is_paused_for_rotation : 1;
  /// Keeps the current dispatch safe when a listener is added or removed.
  u32 reserved_for_next_dispatch : 1;
  /// Seven numbers that the reactions keep between calls (for example, a turn
  /// counter).
  int local_state[7];
  /// The id of the move, the ability or the item, in its source category.
  u16 source_id;
  /// The owner: a Pokémon id, or a position id for a battlefield listener.
  u8 owner_id;
  UID linked_pokemon_id; ///< The Pokémon of the listener, or UID::None().
};
}
