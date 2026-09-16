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

#include "battle/constant/listener_source.h"
#include "battle/constant/moment_kind.h"
#include "battle/native/reaction_table.h"
#include "battle/native/uid.h"

namespace battle {
struct Listener;
class Controller;

typedef bool (*SkipPredicate)(Listener* self, Controller* controller,
                              ListenerSource source, MomentKind moment,
                              u16 source_id, UID owner_id);

struct Listener {
  Listener* previous_listener;
  Listener* next_listener;
  const ReactionTable* reaction_table;
  ///< The {moment, reaction} pairs this listener responds to
  SkipPredicate skip_predicate;
  ///< Optional callback that can veto a reaction before it runs
  ListenerSource source;
  ///< What kind of thing registered this (ability, move, item, status, field...)
  u32 dispatch_priority;
  ///< Resolves the order listeners react in when several answer the same moment

  u32 created_at_depth : 16;
  ///< Dispatch-nesting depth this listener was registered at
  u32 reaction_count : 8; ///< Number of entries in reaction_table
  u32 is_reacting : 1;
  ///< Guards against a reaction re-entering itself while it's already running
  u32 is_paused : 1;
  u32 is_temporary_item_listener : 1;
  ///< Only reacts to the one-off "item used" moment, not the listener's normal moments
  u32 pending_removal : 1;
  u32 allow_reentry : 1;
  ///< Permits this listener to react again while already reacting (overrides is_reacting)
  u32 is_active : 1;
  ///< Whether this listener slot currently holds a live registration
  u32 is_paused_for_rotation : 1;
  ///< Suspended specifically because its Pokémon is benched in a Rotation Battle
  u32 reserved_for_next_dispatch : 1;
  ///< Internal bookkeeping so a concurrent registration/removal doesn't corrupt the current dispatch pass
  int local_state[7];
  ///< Small per-listener scratch storage a reaction can persist between calls (e.g. a turn counter)
  u16 source_id;
  ///< Which specific move/ability/item/etc. this is, within its `source` category
  u8 owner_id;
  ///< What this listener is tied to: a Pokémon id, or a field position id for
  ///< field-scoped listeners - kept as a raw id (not UID) precisely
  ///< because it isn't always a Pokémon
  UID linked_pokemon_id;
  ///< The Pokémon this listener is scoped to, or UID::None() if it
  ///< isn't Pokémon-scoped at all - a cached shortcut so lookups don't need
  ///< to check `source` first
};
}
