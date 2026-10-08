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
 * @file positional_effect_kind.h
 * @brief The effects that stay on one position of the battlefield.
 */

#pragma once
#include <types.h>

namespace battle {
/// An effect on one position of the battlefield.
enum class PositionalEffectKind : u8 {
  kWish, ///< Heals the Pokémon at this position one turn later.
  /// Fully heals the next Pokémon at this position and restores its PP.
  kLunarDance,
  kHealingWish, ///< Like Lunar Dance, but without the PP.
  /// Damage one or more turns later (Future Sight, Doom Desire).
  kDelayedAttack,
  /// The stat stages for the next Pokémon at this position (Baton Pass).
  kBatonTouchPending,

  kCount,
};
}
