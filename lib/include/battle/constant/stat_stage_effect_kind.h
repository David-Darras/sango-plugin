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
enum class StatStageEffectKind : u8 {
  kNone = 0,

  kAttack = 1,
  kDefense,
  kSpecialAttack,
  kSpecialDefense,
  kSpeed,
  kAccuracy,
  kEvasion,

  kCount,

  kCriticalHitStage = kCount,
  kAllStatsAtOnce, ///< Attack, Sp. Atk, Defense, Sp. Def and Speed all at once
};
}
