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
  /// The game never uses these 14 flags. Use them for your scripts, with
  /// HackFlag().
  /// @{
  kFirstFree = 3026,
  kLastFree = 3039,
  /// @}
};

/// The number of free flags: HackFlag(0) to HackFlag(13).
constexpr u16 kHackFlagCount = 14;

/**
 * @brief Returns a free flag for a script of the ROM hack.
 *
 * @code
 * constexpr EventFlag kCandyReceived = core::HackFlag(0);
 * @endcode
 *
 * @param index A number from 0 to 13. Give each flag its own number.
 */
constexpr EventFlag HackFlag(u16 index) {
  return static_cast<EventFlag>(static_cast<u16>(EventFlag::kFirstFree) +
                                index);
}

} // namespace core

using core::EventFlag;
