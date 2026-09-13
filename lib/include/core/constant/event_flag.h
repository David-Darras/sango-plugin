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

namespace core {

/// The game's event flags: the bits of savedata::EventTable, read and set by
/// the field scripts (FlagGet/FlagSet).
enum class EventFlag : u16 {
  kFirstTrainerDefeated = 1740,

  kGameFinished = 2720,

  kRoute101Unlocked = 2774,
  kRoute102Unlocked = 2775,
  kRoute103Unlocked = 2776,

  // Flags the game never touches, free for the plugin's own scripts.
  kFirstFree = 3026,
  kLastFree = 3039,
  kStarterGiven = kFirstFree,
};

} // namespace core

using core::EventFlag;
