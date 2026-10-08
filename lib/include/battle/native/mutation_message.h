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
 * @file mutation_message.h
 * @brief A battle message of a mutation.
 */

#pragma once

#include <types.h>

#include "battle/constant/mutation_message.h"

namespace battle {

/// A battle message and its values.
struct MutationMessage {
  MutationMessageId id;
  u16 category : 7; ///< The message table of the id.
  u16 slot_count : 7;
  u16 append_sound_effect : 1; ///< Adds a sound effect id as the last value.
  u16 is_failure_message : 1;
  s32 slots[8];
};
}
