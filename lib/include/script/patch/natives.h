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
 * @file natives.h
 * @brief The natives that script::Context uses.
 */

#pragma once

#include "script/patch/native_table.h"

namespace script {

/// The natives that script::Context uses. Resolve() finds them by name.
struct Natives {
  // The natives of the state table.
  /// Reads a script variable. Arguments: (variable). Returns: the value.
  NativeFunction WorkGet;
  /// Writes a script variable. Arguments: (variable, value).
  NativeFunction WorkSet;
  /// Reads an event flag. Arguments: (flag). Returns: true when the flag is
  /// set.
  NativeFunction FlagGet;
  NativeFunction FlagSet; ///< Sets an event flag. Arguments: (flag).
  NativeFunction FlagReset; ///< Clears an event flag. Arguments: (flag).
  /// Adds items to the Bag. Arguments: (item, count). Returns: true when the
  /// items go into the Bag.
  NativeFunction ItemAdd;
  /// Checks the space in the Bag. Arguments: (item, count). Returns: true when
  /// the items fit.
  NativeFunction ItemAddCheck;
  /// Counts an item in the Bag. Arguments: (item, 0). Returns: the count.
  NativeFunction ItemGetNum;
  NativeFunction PlayerGetMoney; ///< Returns the money of the player.
  /// Gives money to the player. Arguments: (money).
  NativeFunction PlayerAddMoney;
  /// Removes money from the player. Arguments: (money).
  NativeFunction PlayerSubMoney;
  /// Returns the gender of the player: 0 for male, 1 for female.
  NativeFunction PlayerGetSex;
  /// Arguments: (character). Returns: the direction of the character.
  NativeFunction MdlGetDirDisp;
  /// Arguments: (character). Returns: the movement type of the character.
  NativeFunction MdlGetMoveCode;
  /// Puts a character back in its idle animation. Arguments: (character).
  NativeFunction MdlSetWaitAnimeReq;
  // The natives of the overworld table.
  /// Returns the direction from the player to the character in front.
  NativeFunction PlayerGetReturnDir;
  /// Arguments: (character). Returns: true when the player must crouch to talk
  /// to the character.
  NativeFunction MdlIsHalfSitSkelPreset;
  /// Arguments: (character). Returns: true when the character can turn with an
  /// animation.
  NativeFunction MdlCanUseTurnAcmd;
  /// Starts a list of movements. Arguments: (character).
  NativeFunction MdlAcmdInit;
  /// Adds a movement to the list. Arguments: (movement, direction, count,
  /// replace_turn).
  NativeFunction MdlAcmdSet;
  NativeFunction MdlAcmdSetEnd; ///< Closes the list of movements.
  /// Arguments: (character, or -1). Returns: true when the movements are
  /// complete.
  NativeFunction MdlAcmdUpdate;
  /// Prepares a conversation. Arguments: (character, options, direction,
  /// crouch). Returns: the options.
  NativeFunction _TalkMdlStartInit;
  /// Prepares the end of a conversation. Arguments: (character, options).
  /// Returns: the options.
  NativeFunction _TalkMdlEndInit;
  /// The character looks at the player. Arguments: (character).
  NativeFunction TalkMdlSetEyeToEye;
  /// The character stops to look at the player. Arguments: (character).
  NativeFunction TalkMdlClearEyeToEye;
  /// Starts the talk animation of a character. Arguments: (character).
  NativeFunction TalkMdlSetTalkMotion;
  /// Shows a message. It has 19 arguments. See Context::ShowMessage.
  NativeFunction TalkMdlMsg_Seq;
  /// Returns true when all the message windows wait.
  NativeFunction CheckWinAllSuspend;
  /// Waits for A, B, the Circle Pad or the touch screen.
  NativeFunction LastKeyWait;
  NativeFunction _ABKeyWait; ///< Waits for A, B or the touch screen.
  /// Closes a message window. Arguments: (window).
  NativeFunction MsgWinCloseNo;
  /// Shows Yes / No. Arguments: (is_ctrl_str, str1, str2, init_pos). Returns:
  /// true when the player answers.
  NativeFunction YesNoWin_Seq;
  NativeFunction SEPlay; ///< Plays a sound effect. Arguments: (sound).
  /// Plays a short music (a jingle). Arguments: (sound).
  NativeFunction MEPlay;
  /// Arguments: (sound). Returns: true when the jingle is complete.
  NativeFunction MEIsFinished;
  /// Plays the music of the map again after a jingle.
  NativeFunction MEReturnBGM;
  /// Runs a different script, then comes back. Arguments: (script).
  NativeFunction GlobalCall;
  NativeFunction PokePartyGetCount;
  NativeFunction PokePartyAdd;
  NativeFunction _FieldClose;
  NativeFunction _FieldOpen;
  NativeFunction _CallPoke3Select;
  NativeFunction CallPokeSelect;
  NativeFunction WildBattleResultGet;
  NativeFunction CallPokePartyNameInput;

  bool Resolve() {
    struct Entry {
      const c8* name;
      NativeFunction* slot;
    };
    Entry entries[] = {
        {"WorkGet", &WorkGet},
        {"WorkSet", &WorkSet},
        {"FlagGet", &FlagGet},
        {"FlagSet", &FlagSet},
        {"FlagReset", &FlagReset},
        {"ItemAdd", &ItemAdd},
        {"ItemAddCheck", &ItemAddCheck},
        {"ItemGetNum", &ItemGetNum},
        {"PlayerGetMoney", &PlayerGetMoney},
        {"PlayerAddMoney", &PlayerAddMoney},
        {"PlayerSubMoney", &PlayerSubMoney},
        {"PlayerGetSex", &PlayerGetSex},
        {"MdlGetDirDisp", &MdlGetDirDisp},
        {"MdlGetMoveCode", &MdlGetMoveCode},
        {"MdlSetWaitAnimeReq", &MdlSetWaitAnimeReq},
        {"PlayerGetReturnDir", &PlayerGetReturnDir},
        {"MdlIsHalfSitSkelPreset", &MdlIsHalfSitSkelPreset},
        {"MdlCanUseTurnAcmd", &MdlCanUseTurnAcmd},
        {"MdlAcmdInit", &MdlAcmdInit},
        {"MdlAcmdSet", &MdlAcmdSet},
        {"MdlAcmdSetEnd", &MdlAcmdSetEnd},
        {"MdlAcmdUpdate", &MdlAcmdUpdate},
        {"_TalkMdlStartInit", &_TalkMdlStartInit},
        {"_TalkMdlEndInit", &_TalkMdlEndInit},
        {"TalkMdlSetEyeToEye", &TalkMdlSetEyeToEye},
        {"TalkMdlClearEyeToEye", &TalkMdlClearEyeToEye},
        {"TalkMdlSetTalkMotion", &TalkMdlSetTalkMotion},
        {"TalkMdlMsg_Seq", &TalkMdlMsg_Seq},
        {"CheckWinAllSuspend", &CheckWinAllSuspend},
        {"LastKeyWait", &LastKeyWait},
        {"_ABKeyWait", &_ABKeyWait},
        {"MsgWinCloseNo", &MsgWinCloseNo},
        {"YesNoWin_Seq", &YesNoWin_Seq},
        {"SEPlay", &SEPlay},
        {"MEPlay", &MEPlay},
        {"MEIsFinished", &MEIsFinished},
        {"MEReturnBGM", &MEReturnBGM},
        {"GlobalCall", &GlobalCall},
        {"PokePartyGetCount", &PokePartyGetCount},
        {"PokePartyAdd", &PokePartyAdd},
        {"_FieldClose", &_FieldClose},
        {"_FieldOpen", &_FieldOpen},
        {"_CallPoke3Select", &_CallPoke3Select},
        {"CallPokeSelect", &CallPokeSelect},
        {"WildBattleResultGet", &WildBattleResultGet},
        {"CallPokePartyNameInput", &CallPokePartyNameInput},
    };
    bool complete = true;
    for (const auto & entry : entries) {
      *entry.slot = NativeTable::Find(entry.name);
      if (*entry.slot == nullptr) complete = false;
    }
    return complete;
  }
};

} // namespace script
