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
 * @file reaction_table.h
 * @brief The reactions of a listener.
 */

#pragma once

#include <types.h>

#include "battle/constant/moment_kind.h"
#include "battle/native/uid.h"

namespace battle {
struct Listener;
class Controller;

/**
 * @brief A reaction: a function that the battle engine calls at a moment.
 * @param self The listener.
 * @param controller The object that changes the battle.
 * @param owner_id The owner of the listener.
 * @param local_state Seven numbers that the listener keeps between calls.
 */
typedef void (*Reaction)(Listener* self, Controller* controller,
                         UID owner_id, s32* local_state);

/// One reaction and its moment.
struct ReactionTable {
  MomentKind moment;
  Reaction reaction;
};

} // namespace battle
