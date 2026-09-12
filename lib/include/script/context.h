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
#include "game/constant/item.h"
#include "game/constant/script.h"
#include "game/overworld/character_placement.h"
#include "game/script/native.h"
#include "game/script/engine.h"
#include "system/coroutine.h"

namespace script {
enum class WindowType : u16 {
  kTalk = 0,
  kTalkVariable = 1,
  kJagged = 2,
  kJaggedVariable = 3,
  kFluffyVariable = 4,
  kTown = 5,
  kFacility = 6,
  kBargain = 7,
  kISee = 8,
  kRoadSign = 9,
  kSign = 10,
  kSystem = 11,
  kSmallCircle = 12,
  kMoney = 13,
};

enum TalkOption : u16 {
  kTalkNone = 0,
  kTalkNoZoom = 1 << 0,
  kTalkNoHalfSit = 1 << 1,
  kTalkNoEyeContact = 1 << 2,
  kTalkNoTurn = 1 << 3,
  kTalkMotion = 1 << 4,
  kTalkNoSound = 1 << 5,
  kTalkNoKneel = 1 << 6,
};

enum MessageOption : u32 {
  kMessageNone = 0,
  kMessageCenter = 1 << 0,
  kMessageDoubleSize = 1 << 1,
  kMessageInstant = 1 << 2,
  kMessagePositionUpLeft = 1 << 3,
  kMessagePositionUpRight = 1 << 4,
  kMessagePositionUpCenter = 1 << 5,
  kMessagePositionDownLeft = 1 << 6,
  kMessagePositionDownRight = 1 << 7,
  kMessagePositionDownCenter = 1 << 8,
  kMessageNoTail = 1 << 9,
  kMessageAlignRight = 1 << 14,
};

// Timed actions carry their duration in frames (60 per second). A walk moves
// one tile per step: 8 frames is the normal pace, 4 is running, 2 is cycling,
// 16 or 32 a slow stroll. Turns and waits simply last that many frames.
enum class Action : u16 {
  kNone = 0,
  kFace = 1,
  kFaceRotate32Frames = 2,
  kFaceRotate16Frames = 3,
  kFaceRotate14Frames = 4,
  kFaceRotate12Frames = 5,
  kFaceRotate8Frames = 6,
  kFaceRotate4Frames = 7,
  kWalk32Frames = 11,
  kWalk16Frames = 12,
  kWalk8Frames = 13,
  kWalk4Frames = 14,
  kWalk2Frames = 15,
  kWalk1Frame = 16,
  kWalk12Frames = 17,
  kWalk14Frames = 18,
  kWalk7Frames = 19,
  kWalk6Frames = 20,
  kWalk3Frames = 21,
  kWalk5Frames = 22,
  kDash6Frames = 23,
  kSlowWalk12Frames = 24,
  kStealthyWalk16Frames = 25,
  kTurn2Frames = 26,
  kWalkInPlace32Frames = 27,
  kWalkInPlace16Frames = 28,
  kWalkInPlace8Frames = 29,
  kWalkInPlace4Frames = 30,
  kWalkInPlace2Frames = 31,
  kJumpInPlace16Frames = 32,
  kJumpInPlace8Frames = 33,
  kJump1Tile8Frames = 35,
  kJump1Tile16Frames = 36,
  kJump2Tiles16Frames = 37,
  kJump3Tiles16Frames = 38,
  kWait1Frame = 41,
  kWait2Frames = 42,
  kWait4Frames = 43,
  kWait8Frames = 44,
  kWait15Frames = 45,
  kHalfSitStart = 148,
  kHalfSitWait = 149,
  kHalfSitEnd = 150,
  kKneelStart = 151,
  kKneelWait = 152,
  kKneelEnd = 153,
  kTurn8Frames = 250,
  kTurn12Frames = 251,
};

struct Step {
  Action action;
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

  void TalkStart(s32 object_id = kTalkTarget, u16 options = kTalkMotion) {
    if (object_id == kTalkTarget) object_id = GetTalkTarget();
    SetVariable(ScriptVariable::kSmallMessageTail, 1);

    const s32 direction = Call(natives_.PlayerGetReturnDir);
    bool half_sit = false;
    if (!(options & kTalkNoHalfSit)) {
      half_sit = Call(natives_.MdlIsHalfSitSkelPreset, object_id) == 1;
    }
    options = (u16)Call(natives_._TalkMdlStartInit, object_id, options,
                        direction, half_sit);

    if (!(options & kTalkNoSound)) PlaySound(kSoundMessage);

    if (!(options & kTalkNoTurn)) {
      const s32 facing = Call(natives_.MdlGetDirDisp, object_id);
      const bool can_turn = Call(natives_.MdlCanUseTurnAcmd, object_id) != 0;
      if (can_turn && direction != facing) {
        const Step turn[] = {{Action::kTurn8Frames, (overworld::Facing)direction, 1}};
        Animate(object_id, turn, 1, true);
      } else {
        const Step face[] = {{Action::kFaceRotate4Frames, (overworld::Facing)direction, 1}};
        Animate(object_id, face, 1, false);
      }
      WaitAnimation();
    }

    if (!(options & kTalkNoEyeContact)) {
      Call(natives_.TalkMdlSetEyeToEye, object_id);
    }

    if (!(options & kTalkNoHalfSit) && half_sit) {
      static const Step kHalfSit[] = {
          {Action::kHalfSitStart, overworld::Facing::kInvalid, 1},
          {Action::kHalfSitWait, overworld::Facing::kInvalid, 1},
      };
      Animate(kPlayerObject, kHalfSit, SIZE(kHalfSit), true);
      WaitAnimation();
    }

    if (!(options & kTalkNoKneel) && IsKneelingNpc(object_id)) {
      static const Step kKneel[] = {
          {Action::kKneelStart, overworld::Facing::kInvalid, 1},
          {Action::kKneelWait, overworld::Facing::kInvalid, 1},
      };
      Animate(kPlayerObject, kKneel, SIZE(kKneel), true);
      WaitAnimation();
    }

    if (options & kTalkMotion) Call(natives_.TalkMdlSetTalkMotion, object_id);
  }

  void TalkEnd() {
    u16 options = GetVariable(ScriptVariable::kTalkOptions);
    const s32 object_id = (s16)GetVariable(ScriptVariable::kTalkStartTarget);
    options = (u16)Call(natives_._TalkMdlEndInit, object_id, options);

    if (!(options & kTalkNoEyeContact)) {
      Call(natives_.TalkMdlClearEyeToEye, object_id);
    }

    if (!(options & kTalkNoHalfSit) &&
        Call(natives_.MdlIsHalfSitSkelPreset, object_id) == 1) {
      static const Step kStandUp[] = {
          {Action::kHalfSitEnd, overworld::Facing::kInvalid, 1},
          {Action::kFace, overworld::Facing::kInvalid, 1},
      };
      Animate(kPlayerObject, kStandUp, SIZE(kStandUp), true);
      WaitAnimation();
    }

    if (!(options & kTalkNoKneel) && IsKneelingNpc(object_id)) {
      static const Step kStandUp[] = {
          {Action::kKneelEnd, overworld::Facing::kInvalid, 1},
          {Action::kFace, overworld::Facing::kInvalid, 1},
      };
      Animate(kPlayerObject, kStandUp, SIZE(kStandUp), true);
      WaitAnimation();
    }

    if (options & kTalkMotion) {
      Call(natives_.MdlSetWaitAnimeReq, object_id);
      Yield();
    }
    SetVariable(ScriptVariable::kSmallMessageTail, 0);
  }

  void ShowMessage(const c16* text, WindowType type = WindowType::kTalkVariable,
                   s32 object_id = kTalkTarget, u32 options = kMessageNone,
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
    const Step face[] = {{Action::kFaceRotate4Frames, direction, 1}};
    Animate(object_id, face, 1);
    WaitAnimation();
  }

  void Walk(s32 object_id, overworld::Facing direction, u16 count = 1,
            Action gait = Action::kWalk8Frames) {
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
    struct Header {
      u16 language_count;
      u16 string_count;
      u32 max_language_block_size;
      u32 coded;
      u32 language_block_offset[1];
    } __attribute__((packed));
    struct LanguageBlock {
      u32 size;
      u32 string_offset;
      u16 length;
      u16 user_param;
    } __attribute__((packed));

    u32 length = 0;
    while (text[length] != u'\0') length++;
    const u32 capacity = message_capacity_ -
                         (sizeof(Header) + sizeof(LanguageBlock)) / sizeof(c16);
    if (length >= capacity) length = capacity - 1;

    auto* header = (Header*)message_buffer_;
    auto* block = (LanguageBlock*)(header + 1);
    auto* string = (c16*)(block + 1);

    header->language_count = 1;
    header->string_count = 1;
    header->max_language_block_size = sizeof(LanguageBlock) +
                                      (length + 1) * sizeof(c16);
    header->coded = 1;
    header->language_block_offset[0] = sizeof(Header);
    block->size = header->max_language_block_size;
    block->string_offset = sizeof(LanguageBlock);
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
