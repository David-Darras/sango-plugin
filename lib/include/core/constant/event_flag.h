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
 * @file event_flag.h
 * @brief The event flags of the game.
 *
 * An event flag remembers that something happened. The game saves the flags.
 */

#pragma once

#include <types.h>

namespace core {

/// The event flags of the game: the bits of savedata::EventTable. The
/// scripts read and set them (natives FlagGet and FlagSet).
enum class EventFlag : u16 {
  kFirstTrainerDefeated = 1740,

  kGameFinished = 2720,

  kRoute101Unlocked = 2774,
  kRoute102Unlocked = 2775,
  kRoute103Unlocked = 2776,

  /// @name Free flags
  /// The game never uses these flags. Use them for your scripts.
  /// @{
  kFirstFree = 3026,
  kLastFree = 3039,
  kStarterGiven = kFirstFree, ///< Undertow: the boss gave the starter.
  /// @} ///< Undertow: the boss gave the starter.
  /// @}
};

} // namespace core

using core::EventFlag;
