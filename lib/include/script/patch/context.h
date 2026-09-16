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

#include "common.h"
#include "core/constant/event_flag.h"
#include "overworld/constant/action_command.h"
#include "overworld/constant/facing.h"
#include "pokemon/constant/item.h"
#include "script/constant/message_option.h"
#include "script/constant/script.h"
#include "script/constant/script_variable.h"
#include "script/constant/talk_option.h"
#include "script/constant/window_type.h"
#include "script/native/engine.h"
#include "script/native/message_header.h"
#include "script/native/message_language_block.h"
#include "script/native/script_vm.h"
#include "script/patch/natives.h"
#include "system/coroutine.h"

namespace script {

struct Step {
  ActionCommand action;
  overworld::Facing direction;
  u16 count;
};

constexpr s32 kTalkTarget = -1;
constexpr s32 kPlayerObject = 0xFF;

constexpr u32 kSoundMessage = (6 << 16) + 0;
constexpr u32 kSoundDecide = (6 << 16) + 1;
constexpr u32 kSoundCancel = (6 << 16) + 3;
constexpr u32 kJingleLevelUp = (5 << 16) + 1;
constexpr u32 kJingleItem = (5 << 16) + 2;
constexpr u32 kJingleKeyItem = (5 << 16) + 3;
constexpr u32 kJinglePokemon = (5 << 16) + 4;

class Context {
public:
  static constexpr u32 kStarterChoiceFirst = 0;
  static constexpr u16 kStarterChoiceResult = 0x4030;

  Context(const Natives& natives, Coroutine& coroutine, c16* message_buffer,
          u32 message_capacity)
    : natives_(natives), coroutine_(coroutine),
      message_buffer_(message_buffer), message_capacity_(message_capacity) {}

  void Bind(AmxRuntime* amx) { amx_ = amx; }

  void Yield() {
    Engine::RaiseSleep(amx_);
    coroutine_.Yield();
  }

  void Wait(u32 frames) {
    for (u32 i = 0; i < frames; i++) Yield();
  }

  void CallScript(ScriptId id) {
    Call(natives_.GlobalCall, (s32)id);
    Yield();
  }

  u32 GetPartyCount() { return (u32)Call(natives_.PokePartyGetCount); }

  bool GivePokemon(u32 gift_table_id, bool add_front = false) {
    return Call(natives_.PokePartyAdd, gift_table_id, add_front) != 0;
  }

  u32 ChooseStarter() {
    Call(natives_._FieldClose, true, true);
    Yield();
    Call(natives_._CallPoke3Select, kStarterChoiceFirst);
    Yield();
    Call(natives_._FieldOpen, true);
    Yield();
    return GetVariable(kStarterChoiceResult);
  }

  template <typename... Args>
  INLINE s32 Call(NativeFunction native, Args... args) {
    if (native == nullptr) return 0;
    return NativeTable::Call(native, amx_, args...);
  }

  INLINE AmxRuntime* GetAmx() const { return amx_; }
  INLINE const Natives& GetNatives() const { return natives_; }
  INLINE ScriptVm* GetVm() const { return ScriptVm::FromAmx(amx_); }

  u16 GetVariable(u16 index) { return (u16)Call(natives_.WorkGet, index); }
  u16 GetVariable(ScriptVariable v) { return GetVariable((u16)v); }
  void SetVariable(u16 index, u16 value) {
    Call(natives_.WorkSet, index, value);
  }
  void SetVariable(ScriptVariable v, u16 value) { SetVariable((u16)v, value); }

  bool GetFlag(u16 flag_no) { return Call(natives_.FlagGet, flag_no) != 0; }
  void SetFlag(u16 flag_no) { Call(natives_.FlagSet, flag_no); }
  void ResetFlag(u16 flag_no) { Call(natives_.FlagReset, flag_no); }
  bool GetFlag(EventFlag f) { return GetFlag((u16)f); }
  void SetFlag(EventFlag f) { SetFlag((u16)f); }
  void ResetFlag(EventFlag f) { ResetFlag((u16)f); }

  s32 GetTalkTarget() { return (s16)GetVariable(ScriptVariable::kTalkTarget); }

  bool CanGiveItem(ItemId item, u16 count = 1) {
    return Call(natives_.ItemAddCheck, (u16)item, count) != 0;
  }
  bool GiveItem(ItemId item, u16 count = 1) {
    return Call(natives_.ItemAdd, (u16)item, count) != 0;
  }
  u32 CountItem(ItemId item) {
    return (u32)Call(natives_.ItemGetNum, (u16)item, 0);
  }
  bool HasItem(ItemId item, u32 count = 1) { return CountItem(item) >= count; }
  u32 GetMoney() { return (u32)Call(natives_.PlayerGetMoney); }
  void AddMoney(u32 amount) { Call(natives_.PlayerAddMoney, amount); }
  void SubMoney(u32 amount) { Call(natives_.PlayerSubMoney, amount); }
  bool IsPlayerFemale() { return Call(natives_.PlayerGetSex) != 0; }

  void PlaySound(u32 sound_id) { Call(natives_.SEPlay, sound_id); }

  void PlayJingle(u32 sound_id) {
    Call(natives_.MEPlay, sound_id);
    while (Call(natives_.MEIsFinished, sound_id) == 0) Yield();
    Call(natives_.MEReturnBGM);
  }

  void TalkStart(s32 object_id = kTalkTarget,
                 TalkOption options = TalkOption::kMotion) {
    if (object_id == kTalkTarget) object_id = GetTalkTarget();
    SetVariable(ScriptVariable::kSmallMessageTail, 1);

    const s32 direction = Call(natives_.PlayerGetReturnDir);
    bool half_sit = false;
    if (!HasFlag(options, TalkOption::kNoHalfSit)) {
      half_sit = Call(natives_.MdlIsHalfSitSkelPreset, object_id) == 1;
    }
    options = static_cast<TalkOption>(Call(
        natives_._TalkMdlStartInit, object_id, options, direction, half_sit));

    if (!HasFlag(options, TalkOption::kNoSound)) PlaySound(kSoundMessage);

    if (!HasFlag(options, TalkOption::kNoTurn)) {
      const s32 facing = Call(natives_.MdlGetDirDisp, object_id);
      const bool can_turn = Call(natives_.MdlCanUseTurnAcmd, object_id) != 0;
      if (can_turn && direction != facing) {
        const Step turn[] = {
            {ActionCommand::kTurn8Frames, (overworld::Facing)direction, 1}};
        Animate(object_id, turn, 1, true);
      } else {
        const Step face[] = {
            {ActionCommand::kFaceRotate4Frames, (overworld::Facing)direction,
             1}};
        Animate(object_id, face, 1, false);
      }
      WaitAnimation();
    }

    if (!HasFlag(options, TalkOption::kNoEyeContact)) {
      Call(natives_.TalkMdlSetEyeToEye, object_id);
    }

    if (!HasFlag(options, TalkOption::kNoHalfSit) && half_sit) {
      static const Step kHalfSit[] = {
          {ActionCommand::kHalfSitStart, overworld::Facing::kInvalid, 1},
          {ActionCommand::kHalfSitWait, overworld::Facing::kInvalid, 1},
      };
      Animate(kPlayerObject, kHalfSit, SIZE(kHalfSit), true);
      WaitAnimation();
    }

    if (!HasFlag(options, TalkOption::kNoKneel) && IsKneelingNpc(object_id)) {
      static const Step kKneel[] = {
          {ActionCommand::kKneelStart, overworld::Facing::kInvalid, 1},
          {ActionCommand::kKneelWait, overworld::Facing::kInvalid, 1},
      };
      Animate(kPlayerObject, kKneel, SIZE(kKneel), true);
      WaitAnimation();
    }

    if (HasFlag(options, TalkOption::kMotion)) {
      Call(natives_.TalkMdlSetTalkMotion, object_id);
    }
  }

  void TalkEnd() {
    auto options =
        static_cast<TalkOption>(GetVariable(ScriptVariable::kTalkOptions));
    const s32 object_id = (s16)GetVariable(ScriptVariable::kTalkStartTarget);
    options = static_cast<TalkOption>(
        Call(natives_._TalkMdlEndInit, object_id, options));

    if (!HasFlag(options, TalkOption::kNoEyeContact)) {
      Call(natives_.TalkMdlClearEyeToEye, object_id);
    }

    if (!HasFlag(options, TalkOption::kNoHalfSit) &&
        Call(natives_.MdlIsHalfSitSkelPreset, object_id) == 1) {
      static const Step kStandUp[] = {
          {ActionCommand::kHalfSitEnd, overworld::Facing::kInvalid, 1},
          {ActionCommand::kFace, overworld::Facing::kInvalid, 1},
      };
      Animate(kPlayerObject, kStandUp, SIZE(kStandUp), true);
      WaitAnimation();
    }

    if (!HasFlag(options, TalkOption::kNoKneel) && IsKneelingNpc(object_id)) {
      static const Step kStandUp[] = {
          {ActionCommand::kKneelEnd, overworld::Facing::kInvalid, 1},
          {ActionCommand::kFace, overworld::Facing::kInvalid, 1},
      };
      Animate(kPlayerObject, kStandUp, SIZE(kStandUp), true);
      WaitAnimation();
    }

    if (HasFlag(options, TalkOption::kMotion)) {
      Call(natives_.MdlSetWaitAnimeReq, object_id);
      Yield();
    }
    SetVariable(ScriptVariable::kSmallMessageTail, 0);
  }

  void ShowMessage(const c16* text, WindowType type = WindowType::kTalkVariable,
                   s32 object_id = kTalkTarget,
                   MessageOption options = MessageOption::kNone,
                   u32 window_id = 0) {
    SharedWorkspaces* shared = GetSharedWorkspaces();
    if (shared == nullptr) return;
    const u32 string_id = BuildMessage(text);
    LoadedMessage* previous = shared->loaded_message;
    message_.buffer = message_buffer_;
    message_.data = nullptr;
    shared->loaded_message = &message_;

    Call(natives_.TalkMdlMsg_Seq, string_id, window_id, object_id, (u16)type,
         0, 0, 0, 0, 0, 0, 0, 0, true, false, false, true, -1, true,
         options);

    shared->loaded_message = previous;

    while (Call(natives_.CheckWinAllSuspend) == 0) Yield();
  }

  void WaitKey() {
    Call(natives_.LastKeyWait);
    Yield();
  }

  void WaitAbKey() {
    Call(natives_._ABKeyWait);
    Yield();
  }

  void CloseMessage(u32 window_id = 0) {
    Call(natives_.MsgWinCloseNo, window_id);
  }

  void Talk(const c16* text, WindowType type = WindowType::kTalkVariable,
            s32 object_id = kTalkTarget) {
    ShowMessage(text, type, object_id);
    WaitKey();
    CloseMessage();
  }

  bool AskYesNo(bool cursor_on_no = false) {
    while (Call(natives_.YesNoWin_Seq, false, 0, 0, cursor_on_no ? 1 : 0) == 0) {
      Yield();
    }
    return GetVariable(ScriptVariable::kAnswer) != 0;
  }

  void Animate(s32 object_id, const Step* steps, u32 count,
               bool replace_turn = false) {
    Call(natives_.MdlAcmdInit, object_id);
    for (u32 i = 0; i < count; i++) {
      Call(natives_.MdlAcmdSet, (u16)steps[i].action, (u8)steps[i].direction,
           steps[i].count, replace_turn);
    }
    Call(natives_.MdlAcmdSetEnd);
  }

  void WaitAnimation(s32 object_id = kTalkTarget) {
    while (Call(natives_.MdlAcmdUpdate, object_id) == 0) Yield();
  }

  void Face(s32 object_id, overworld::Facing direction) {
    const Step face[] = {{ActionCommand::kFaceRotate4Frames, direction, 1}};
    Animate(object_id, face, 1);
    WaitAnimation();
  }

  void Walk(s32 object_id, overworld::Facing direction, u16 count = 1,
            ActionCommand gait = ActionCommand::kWalk8Frames) {
    const Step walk[] = {{gait, direction, count}};
    Animate(object_id, walk, 1);
    WaitAnimation();
  }

private:
  static constexpr u32 kKneelingMoveCode = 60;

  bool IsKneelingNpc(s32 object_id) {
    return Call(natives_.MdlGetMoveCode, object_id) == kKneelingMoveCode;
  }

  SharedWorkspaces* GetSharedWorkspaces() const {
    Engine* engine = Engine::GetInstance();
    if (engine == nullptr || engine->GetWorkspace() == nullptr) return nullptr;
    return &engine->GetWorkspace()->shared;
  }

  u32 BuildMessage(const c16* text) {

    u32 length = 0;
    while (text[length] != u'\0') length++;
    const u32 capacity =
        message_capacity_ -
        (sizeof(MessageHeader) + sizeof(MessageLanguageBlock)) / sizeof(c16);
    if (length >= capacity) length = capacity - 1;

    auto* header = (MessageHeader*)message_buffer_;
    auto* block = (MessageLanguageBlock*)(header + 1);
    auto* string = (c16*)(block + 1);

    header->language_count = 1;
    header->string_count = 1;
    header->max_language_block_size = sizeof(MessageLanguageBlock) +
                                      (length + 1) * sizeof(c16);
    header->coded = 1;
    header->language_block_offset[0] = sizeof(MessageHeader);
    block->size = header->max_language_block_size;
    block->string_offset = sizeof(MessageLanguageBlock);
    block->length = (u16)(length + 1);
    block->user_param = 0;
    for (u32 i = 0; i < length; i++) string[i] = text[i];
    string[length] = u'\0';
    return 0;
  }

  AmxRuntime* amx_ = nullptr;
  const Natives& natives_;
  Coroutine& coroutine_;
  c16* message_buffer_;
  u32 message_capacity_;
  LoadedMessage message_ = {nullptr, nullptr};
};

typedef void (*Function)(Context& script);
} // namespace script
