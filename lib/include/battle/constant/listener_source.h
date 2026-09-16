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

enum class ListenerSource : u8 {
  kActiveMove, ///< Registered by the move currently being resolved
  kFieldPosition, ///< Registered by an effect tied to one specific field slot
  kTeamSide, ///< Registered by a team-side effect (Light Screen, Tailwind...)
  kField, ///< Registered by a whole-field effect (Trick Room, Gravity...)
  kAbility, ///< Registered by an ability
  kHeldItem, ///< Registered by a held item

  kDetached,
  ///< Converted from any of the above once it no longer depends on
   ///< the Pokémon/item that created it - keeps reacting even if that
   ///< Pokémon faints or that item gets consumed. All swept away
   ///< together at the end of every turn.

  kCount,
};

} // namespace battle
