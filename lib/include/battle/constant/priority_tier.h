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
 * @file priority_tier.h
 * @brief The order of the listeners at the same moment.
 */

#pragma once

#include <types.h>

namespace battle {
/// The order of the listeners. A lower tier reacts first.
enum class PriorityTier : u8 {
  kActiveMoveDefault, ///< The default tier of the move listeners.
  kFieldPositionDefault, ///< The default tier of the position listeners.
  kTeamSideDefault, ///< The default tier of the side listeners.
  kFieldDefault, ///< The default tier of the battlefield listeners.

  /// The tier of Poison Touch (poison after a contact move).
  kAbilityPoisonTouch,
  kAbilityDefault, ///< The default tier of the ability listeners.

  kHeldItemDefault, ///< The default tier of the held item listeners.
  /// The tier of Stall: it always reacts last, also after the items.
  kAbilityStall,

  kCount,
};
}