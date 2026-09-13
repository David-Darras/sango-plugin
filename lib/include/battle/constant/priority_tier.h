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
enum class PriorityTier : u8 {
  kActiveMoveDefault, ///< Default tier for move listeners
  kFieldPositionDefault, ///< Default tier for field-position listeners
  kTeamSideDefault, ///< Default tier for team-side listeners
  kFieldDefault, ///< Default tier for whole-field listeners

  kAbilityPoisonTouch,
  ///< Poison Touch's own tier (inflicts poison on a contact hit)
  kAbilityDefault, ///< Default tier for ability listeners

  kHeldItemDefault, ///< Default tier for held-item listeners
  kAbilityStall, ///< Stall's own tier (always resolves last, even after items)

  kCount,
};
}