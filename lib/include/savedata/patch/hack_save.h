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
 * @file hack_save.h
 * @brief The flags and the variables of the ROM hack in the save data.
 *
 * The game has only 14 free event flags. The save data also has a reserved
 * segment of 2000 bytes that the game saves and never uses. The library puts
 * the flags and the variables of the ROM hack there:
 * - 4096 flags (true or false): HackSave::GetFlag(), HackSave::SetFlag(),
 * - 512 variables (0 to 65535): HackSave::GetVariable(),
 *   HackSave::SetVariable().
 *
 * A new game starts with all the flags false and all the variables 0. The
 * player saves the game to keep them, like the other data of the game.
 *
 * The save data also remembers the product that wrote it. A save data of an
 * other ROM hack gives a message in the log of the plugin.
 *
 * Alpha Sapphire only: in X, the functions do nothing and return false or 0.
 *
 * @code
 * constexpr u16 kMetTheRival = 0;      // A flag.
 * constexpr u16 kBadgesOfMyHack = 0;   // A variable.
 *
 * if (!savedata::HackSave::GetFlag(kMetTheRival)) {
 *   savedata::HackSave::SetFlag(kMetTheRival);
 * }
 * savedata::HackSave::SetVariable(kBadgesOfMyHack, 3);
 * @endcode
 */

#pragma once

#include "common.h"

namespace savedata {

/// The flags and the variables of the ROM hack in the save data.
class HackSave {
public:
  /// The number of flags: 0 to 4095.
  static constexpr u32 kFlagCount = 4096;
  /// The number of variables: 0 to 511.
  static constexpr u32 kVariableCount = 512;

  /// Returns true when the save data can keep the flags and the variables.
  static bool IsAvailable();
  /// Returns true when an other product wrote the save data.
  static bool IsFromOtherHack();

  /// Returns a flag. False for a flag that is not set.
  static bool GetFlag(u16 flag);
  /// Sets a flag (true) or clears it (false).
  static void SetFlag(u16 flag, bool value = true);
  /// Clears a flag.
  static void ResetFlag(u16 flag) { SetFlag(flag, false); }

  /// Returns a variable. 0 for a variable that is not set.
  static u16 GetVariable(u16 variable);
  /// Sets a variable.
  static void SetVariable(u16 variable, u16 value);
  /// Adds `amount` to a variable (for example a counter), and returns the
  /// new value. The value stops at 65535.
  static u16 AddToVariable(u16 variable, u16 amount = 1);

private:
  /// The start of the reserved segment, with its header.
  struct Layout;
  /// Returns the data in the save, ready to use, or null.
  static Layout* GetLayout();
};

} // namespace savedata
