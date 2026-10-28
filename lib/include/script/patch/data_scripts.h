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
 * @file data_scripts.h
 * @brief Scripts that the player writes in the menu, without code.
 */

#pragma once

#include "common.h"
#include "script/patch/context.h"

namespace script {

/**
 * @brief Scripts that the player writes in the menu.
 *
 * A script is a list of steps. Each step does one thing: it shows a text, asks
 * a question, gives an item, sets a flag... The scripts have the numbers
 * 32000 to 32011. A sign, a trigger or a character that uses one of these
 * numbers starts the script (see overworld::TriggerEditor).
 *
 * The scripts are in the file `sdmc:/sango/scripts.txt`. Each line is
 * `script step action parameter1 parameter2 text`.
 */
class DataScripts {
  MAKE_SINGLETON(DataScripts)
public:
  static constexpr u32 kMaxScripts = 12;
  static constexpr u32 kMaxSteps = 8;
  static constexpr u32 kFirstScript = 32000;
  /// A text has two parts of 31 characters: the keyboard of the menu holds 31.
  static constexpr u32 kPartLength = 32;

  enum Action : u8 {
    kNone,         ///< The end of the script.
    kSay,          ///< Shows a text. Parameter 1: 0 sign window, 1 talk window.
    kAsk,          ///< Asks Yes or No. No ends the script.
    kGiveItem,     ///< Parameter 1: item. Parameter 2: number (0 is 1).
    kAddMoney,     ///< Parameter 1: amount. Parameter 2: 1 takes the money.
    kSetFlag,      ///< Parameter 1: flag.
    kResetFlag,    ///< Parameter 1: flag.
    kRequireFlag,  ///< The script ends when the flag is not set.
    kSound,        ///< Plays a sound. Parameter 1: sound.
    kWait,         ///< Waits parameter 1 frames.
    kRunScript,    ///< Starts a script of the game. Parameter 1: its number.
    kGivePokemon,  ///< Parameter 1: the number of the gift in the table.
    kActionCount
  };

  struct Step {
    u8 action = kNone;
    u16 parameter_1 = 0;
    u16 parameter_2 = 0;
    c16 text[kPartLength] = {};
    c16 text_2[kPartLength] = {};
  };

  bool is_live = false; ///< Reads the file again when it changes.

  /// Registers the scripts, and reads the file.
  static void Initialize();

  /// Called at each frame: reads the file again when it changed (is_live).
  static void Update();

  static Step* GetStep(u32 script, u32 step);

  /// Returns true when the script has at least one step.
  static bool IsUsed(u32 script);

  /// Writes the file. Returns false when it cannot be written.
  static bool Save();

  /// Runs a script. The game calls it through the C++ scripts.
  static void Run(Context& context, u32 script);

private:
  static bool Load();
  static bool LoadLine(const c8* line);
  static u32 HashFile();

  Step steps_[kMaxScripts][kMaxSteps];
  u32 frame_ = 0;
  u32 file_hash_ = 0;
  bool has_baseline_ = false;
};

} // namespace script
