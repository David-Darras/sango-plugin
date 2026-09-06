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

namespace battle {
// The exact slot a Pokémon occupies on the field - distinct from UID (which
// Pokémon), needed for anything that depends on physical position: spread
// moves, targeting in Double/Triple Battles, adjacency checks. These ids are
// used as raw array indices elsewhere, so don't reorder them.
// Slots 3-4 only exist in Triple Battles.
enum class FieldPosition : u8 {
  kFirstSideSlot0, ///< The player's side in single-player, the server's side in online play
  kSecondSideSlot0,
  kFirstSideSlot1,
  kSecondSideSlot1,
  kFirstSideSlot2,
  kSecondSideSlot2,
  kFirstSideSlot3,
  kSecondSideSlot3,
  kFirstSideSlot4,
  kSecondSideSlot4,

  kCount,
  kNone = kCount,
};
}
