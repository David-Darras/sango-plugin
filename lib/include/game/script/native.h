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

#include <cstring>

#include "common.h"
#include "game/script/amx.h"

namespace script {
typedef PawnNativeFunction NativeFunction;

class NativeTable {
public:
  static NativeFunction Find(const c8* name) {
    static const uptr kTables[] = {
        ADDRESS_SCRIPT_NATIVES_FIELD,
        ADDRESS_SCRIPT_NATIVES_STATE,
        ADDRESS_SCRIPT_NATIVES_INTERACTIVE,
        ADDRESS_SCRIPT_NATIVES_POKEMON_CENTER,
        ADDRESS_SCRIPT_NATIVES_MAP_EFFECTS,
        ADDRESS_SCRIPT_NATIVES_BATTLE_FACILITY,
        ADDRESS_SCRIPT_NATIVES_FIELD_SERVICES,
        ADDRESS_SCRIPT_NATIVES_NPC_AI,
    };
    for (u32 i = 0; i < SIZE(kTables); i++) {
      NativeFunction function = FindIn((const PawnNativeBinding*)kTables[i],
                                       name);
      if (function != nullptr) return function;
    }
    return nullptr;
  }

  template <typename... Args>
  STATIC_INLINE s32 Call(NativeFunction function, AmxRuntime* amx,
                         Args... args) {
    PawnCell params[sizeof...(Args) + 1] = {
        (PawnCell)(sizeof...(Args) * sizeof(PawnCell)),
        (PawnCell)(s32)args...
    };
    return (s32)function(amx, params);
  }

private:
  static NativeFunction FindIn(const PawnNativeBinding* table,
                               const c8* name) {
    for (u32 i = 0; i < kMaxEntries; i++) {
      if (table[i].function_name == nullptr) break;
      if (std::strcmp(table[i].function_name, name) == 0) {
        return table[i].function;
      }
    }
    return nullptr;
  }

  static constexpr u32 kMaxEntries = 512;
};

struct Natives {
  // state table
  NativeFunction WorkGet; // (work_no) -> value
  NativeFunction WorkSet; // (work_no, value)
  NativeFunction FlagGet; // (flag_no) -> bool
  NativeFunction FlagSet; // (flag_no)
  NativeFunction FlagReset; // (flag_no)
  NativeFunction ItemAdd; // (item_id, count) -> bool
  NativeFunction ItemAddCheck; // (item_id, count) -> bool
  NativeFunction ItemGetNum; // (item_id, unused) -> count in the bag
  NativeFunction PlayerGetMoney; // () -> money
  NativeFunction PlayerAddMoney; // (money)
  NativeFunction PlayerSubMoney; // (money)
  NativeFunction PlayerGetSex; // () -> 0 male, 1 female
  NativeFunction MdlGetDirDisp; // (obj_id) -> Dir
  NativeFunction MdlGetMoveCode; // (obj_id) -> move code
  NativeFunction MdlSetWaitAnimeReq; // (obj_id)
  // field table
  NativeFunction PlayerGetReturnDir; // () -> Dir facing the player
  NativeFunction MdlIsHalfSitSkelPreset; // (obj_id) -> bool
  NativeFunction MdlCanUseTurnAcmd; // (obj_id) -> bool
  NativeFunction MdlAcmdInit; // (obj_id)
  NativeFunction MdlAcmdSet; // (acmd, dir, count, replace_turn)
  NativeFunction MdlAcmdSetEnd; // ()
  NativeFunction MdlAcmdUpdate; // (obj_id or -1) -> bool finished
  NativeFunction _TalkMdlStartInit; // (obj_id, mode, dir, half_sit) -> mode
  NativeFunction _TalkMdlEndInit; // (obj_id, mode) -> mode
  NativeFunction TalkMdlSetEyeToEye; // (obj_id)
  NativeFunction TalkMdlClearEyeToEye; // (obj_id)
  NativeFunction TalkMdlSetTalkMotion; // (obj_id)
  NativeFunction TalkMdlMsg_Seq; // (19 params, see Context::ShowMessage)
  NativeFunction CheckWinAllSuspend; // () -> bool every window is waiting
  NativeFunction LastKeyWait; // () sleeps until A/B/stick/touch
  NativeFunction _ABKeyWait; // () sleeps until A/B/touch
  NativeFunction MsgWinCloseNo; // (window_id)
  NativeFunction YesNoWin_Seq; // (is_ctrl_str, str1, str2, init_pos) -> done
  NativeFunction SEPlay; // (sound_item_id)
  NativeFunction MEPlay; // (sound_item_id)
  NativeFunction MEIsFinished; // (sound_item_id) -> bool
  NativeFunction MEReturnBGM; // () brings the map music back after a jingle
  NativeFunction GlobalCall; // (script_id) runs a child script

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
    };
    bool complete = true;
    for (u32 i = 0; i < SIZE(entries); i++) {
      *entries[i].slot = NativeTable::Find(entries[i].name);
      if (*entries[i].slot == nullptr) complete = false;
    }
    return complete;
  }
};
} // namespace script