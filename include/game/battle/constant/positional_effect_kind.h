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
enum class PositionalEffectKind : u8 {
  kWish, ///< Heals whoever occupies this position one turn later
  kLunarDance, ///< Fully heals and restores PP for the next Pokémon sent to this position
  kHealingWish, ///< Same as Lunar Dance, without the PP restoration
  kDelayedAttack, ///< Future Sight/Doom Desire-style delayed damage
  kBatonTouchPending, ///< Stat stages waiting to be handed off to the next Pokémon sent here

  kCount,
};
}
