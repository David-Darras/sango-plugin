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
 * @file studio_events.h
 * @brief Runs the events that the user made in Sango Studio, without code.
 */

#pragma once

#include "common.h"
#include "script/patch/context.h"

namespace script {

/**
 * @brief Runs the events of Sango Studio.
 *
 * An event is a list of steps: show a text, ask a question, give an item,
 * start a battle, send the player to a map... The steps can jump, so an event
 * can test a flag, an item or a variable, and ask a question with choices.
 * Sango Studio (page Events) writes one file for each event:
 * `sdmc:/sango-studio/event/<number>.bin`, where the number is from 0 to 255.
 * The event has the script number 32100 plus its number. A sign, a trigger or
 * a character that has this script number starts the event.
 *
 * The plugin reads the file each time that the event starts. A change of the
 * file is used at once, without a restart.
 *
 * The file is little-endian:
 *
 * | Size | Meaning |
 * | --- | --- |
 * | 4 | The signature `SEVT`. |
 * | 2 | The version (1). |
 * | 2 | The number of steps. |
 *
 * Then, for each step:
 *
 * | Size | Meaning |
 * | --- | --- |
 * | 2 | The action (see Action). |
 * | 2 | The step to go to (a jump), or 0xFFFF to end the event. |
 * | 4 each | The values a, b and c. |
 * | 2 | The number of characters of the text. |
 * | 2 each | The characters of the text (UTF-16), in the readable form of core::TextEditor. |
 */
class StudioEvents {
  MAKE_SINGLETON(StudioEvents)
public:
  /// The number of events, and the script number of the first one.
  static constexpr u32 kMaxEvents = 256;
  static constexpr u32 kFirstScript = 32100;
  /// The most steps of one event, and the most steps that one run executes.
  static constexpr u32 kMaxSteps = 128;
  static constexpr u32 kMaxExecuted = 4096;
  /// The step number that ends the event.
  static constexpr u16 kEnd = 0xFFFF;

  /**
   * @brief What a step does. "Jump" is the field `target` of the step.
   *
   * A step that tests something goes to the jump when the test fails, and to
   * the next step when it is true.
   */
  enum Action : u16 {
    kStop = 0,         ///< Ends the event.
    kSay = 1,          ///< Shows the text. a: 0 sign window, 1 talk window.
    kGiveItem = 2,     ///< Gives the item a, b times (0 is 1).
    kTakeItem = 3,     ///< Takes the item a, b times (0 is 1).
    kMoney = 4,        ///< a: the amount. b: 0 gives it, 1 takes it.
    kSetFlag = 5,      ///< Sets the flag a. b: 0 flag of the game, 1 flag of Sango.
    kResetFlag = 6,    ///< Clears the flag a. b: as kSetFlag.
    kSetVariable = 7,  ///< Gives the value b to the variable a of Sango.
    kSound = 8,        ///< Plays the sound effect a.
    kJingle = 9,       ///< Plays the short music a, then the music of the map.
    kWait = 10,        ///< Waits a frames.
    kRunScript = 11,   ///< Starts the script a of the game.
    kGivePokemon = 12, ///< Gives a Pokémon. a: species, b: level, c: form.
    kWildBattle = 13,  ///< Starts a wild battle. a: species, b: level, c: form.
    kTrainerBattle = 14, ///< Starts a battle against the trainer a. A defeat ends the event.
    kWarp = 15,        ///< Sends the player to the map a, at the tile b (x) and c (z).
    kHeal = 16,        ///< Heals the Pokémon of the party.
    kFade = 17,        ///< a: 0 makes the screens black, 1 shows them again.
    kJump = 18,        ///< Goes to the jump.
    kIfFlag = 19,      ///< Tests the flag a (b: as kSetFlag).
    kIfItem = 20,      ///< Tests that the player has b of the item a (0 is 1).
    kIfMoney = 21,     ///< Tests that the player has a money or more.
    kIfVariable = 22,  ///< Tests the variable a against b. c: 0 equal, 1 different, 2 more, 3 less.
    kAsk = 23,         ///< Asks yes or no. The jump is for no.
    kChoice = 24,      ///< The text is "question|choice 1|choice 2|choice 3". The steps a, b and c are the steps of the choices.
    kActionCount
  };

  /// Registers the scripts 32100 to 32355.
  static void Initialize();

  /// Runs the event that has the script number of the script that runs now.
  static void Run(Context& context);
};

} // namespace script
