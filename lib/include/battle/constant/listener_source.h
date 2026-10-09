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
 * @file listener_source.h
 * @brief The sources of the listeners of the battle engine.
 */

#pragma once

#include <types.h>

namespace battle {

/// What registered a listener. See docs/concepts/battle-engine.md.
enum class ListenerSource : u8 {
  kActiveMove, ///< The current move registered it.
  /// An effect on one position of the battlefield registered it.
  kFieldPosition,
  /// An effect on one side registered it (Light Screen, Tailwind...).
  kTeamSide,
  /// An effect on the full battlefield registered it (Trick Room, Gravity...).
  kField,
  kAbility, ///< An ability registered it.
  kHeldItem, ///< A held item registered it.

  /// A listener that does not depend on its Pokémon or its item any more.
  /// It continues when the Pokémon faints. The engine removes it at the end
  /// of the turn.
  kDetached,

  kCount,
};

} // namespace battle
