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
 * @file context.h
 * @brief The functions of a C++ script: messages, items, flags, movements...
 *
 * @see docs/concepts/scripts.md
 * @see docs/tutorials/09-write-an-overworld-script.md
 */

#pragma once

#include <initializer_list>

#include "common.h"
#include "battle/constant/trainer.h"
#include "core/constant/event_flag.h"
#include "core/translations.h"
#include "overworld/constant/action_command.h"
#include "overworld/constant/facing.h"
#include "overworld/constant/map.h"
#include "overworld/native/static_encounter.h"
#include "pokemon/constant/item.h"
#include "pokemon/native/gift_pokemon_data.h"
#include "savedata/native/savedata.h"
#include "savedata/native/trainer_status.h"
#include "savedata/patch/hack_save.h"
#include "script/constant/battle_result.h"
#include "script/constant/fade_color.h"
#include "script/constant/message_option.h"
#include "script/constant/script.h"
#include "script/constant/script_variable.h"
#include "script/constant/talk_option.h"
#include "script/constant/trainer_battle_option.h"
#include "script/constant/window_type.h"
#include "script/native/engine.h"
#include "script/native/message_header.h"
#include "script/native/message_language_block.h"
#include "script/native/script_vm.h"
#include "script/patch/natives.h"
#include "system/coroutine.h"
#include "system/native/sound.h"
#include "ui/log.h"
#include "ui/patch/party_select.h"

namespace script {

/// One movement of a character: an action, a direction and a count.
struct Step {
  ActionCommand action;
  overworld::Facing direction;
  u16 count;
};

/// The character that the player talks to.
constexpr s32 kTalkTarget = -1;
/// The player.
constexpr s32 kPlayerObject = 0xFF;

constexpr u32 kPokeSelectSimple = 1;
constexpr u32 kPokeSelectContest = 4;
constexpr s32 kBattleResultWon = 1; ///< The battle result when the player wins.
constexpr s32 kPokeSelectCancel = 6;

/// @name Sounds and short musics for PlaySound() and PlayJingle()
/// @{
constexpr u32 kSoundMessage = (6 << 16) + 0;
constexpr u32 kSoundDecide = (6 << 16) + 1;
constexpr u32 kSoundCancel = (6 << 16) + 3;
constexpr u32 kJingleLevelUp = (5 << 16) + 1;
constexpr u32 kJingleItem = (5 << 16) + 2;
constexpr u32 kJingleKeyItem = (5 << 16) + 3;
constexpr u32 kJinglePokemon = (5 << 16) + 4;
/// @}

/// Choose() returns this value when the player presses B.
constexpr s32 kChoiceCancel = -1;
/// @}

/**
 * @brief The functions of a C++ script.
 *
 * A C++ script receives a Context. The functions that wait (Talk(),
 * AskYesNo()...) give control back to the game: the game draws the next
 * frames, then the script continues.
 */
class Context {
public:
  static constexpr u32 kStarterChoiceFirst = 0;
  static constexpr u16 kStarterChoiceResult = 0x4030;

  Context(const Natives& natives, Coroutine& coroutine, c16* message_buffer,
          u32 message_capacity)
    : natives_(natives), coroutine_(coroutine),
      message_buffer_(message_buffer), message_capacity_(message_capacity) {}

  void Bind(AmxRuntime* amx) { amx_ = amx; }

  /// Waits for the next frame.
  void Yield() {
    Engine::RaiseSleep(amx_);
    coroutine_.Yield();
  }

  /// Waits some frames.
  void Wait(u32 frames) {
    for (u32 i = 0; i < frames; i++) Yield();
  }

  /// Runs a script of the game.
  void CallScript(ScriptId id) {
    Call(natives_.GlobalCall, (s32)id);
    Yield();
  }

  /// Returns the number of Pokémon in the party.
  u32 GetPartyCount() { return (u32)Call(natives_.PokePartyGetCount); }

  /// Gives a gift Pokémon of the gift table. See pokemon::GiftPokemonData.
  bool GivePokemon(u32 gift_table_id, bool add_front = false) {
    return Call(natives_.PokePartyAdd, gift_table_id, add_front) != 0;
  }

  /// Opens the starter selection. Returns the selected starter (0, 1 or 2).
  u32 ChooseStarter() {
    Call(natives_._FieldClose, true, true);
    Yield();
    Call(natives_._CallPoke3Select, kStarterChoiceFirst);
    Yield();
    Call(natives_._FieldOpen, true);
    Yield();
    return GetVariable(kStarterChoiceResult);
  }

  /// Opens the party. Returns the selected slot, or kPokeSelectCancel when
  /// the player cancels.
  s32 SelectPokemon() {
    Call(natives_.CallPokeSelect, kPokeSelectSimple,
         (u32)ScriptVariable::kReturn0, 0, true);
    Yield();
    return (s16)GetVariable(ScriptVariable::kReturn0);
  }

  bool InputPartyNickname(u32 slot) {
    Call(natives_.CallPokePartyNameInput, (u32)ScriptVariable::kReturn0,
         (u32)ScriptVariable::kReturn1, slot, true, false);
    Yield();
    return GetVariable(ScriptVariable::kReturn0) != 0;
  }

  /// Returns true when the player won the last battle.
  bool WonLastBattle() {
    return Call(natives_.WildBattleResultGet) == kBattleResultWon;
  }

  /// Opens the party: the player selects `count` Pokémon. The selected slots
  /// go to `order`. kUnsupported: the game does not support it.
  ui::PartySelect::Status SelectParty(u32 count, u8* order) {
    if (!ui::PartySelect::kIsSupported) return ui::PartySelect::Status::kUnsupported;
    auto& party_select = ui::PartySelect::GetInstance();
    party_select.Arm(count);
    Call(natives_.CallPokeSelect, kPokeSelectContest,
         (u32)ScriptVariable::kReturn0, 0, true);
    Yield();
    return party_select.GetResult(order);
  }

  /// Calls a native of the game with its parameters.
  template <typename... Args>
  INLINE s32 Call(NativeFunction native, Args... args) {
    if (native == nullptr) return 0;
    return NativeTable::Call(native, amx_, args...);
  }

  INLINE AmxRuntime* GetAmx() const { return amx_; }
  INLINE const Natives& GetNatives() const { return natives_; }
  INLINE ScriptVm* GetVm() const { return ScriptVm::FromAmx(amx_); }

  /// Returns a script variable.
  u16 GetVariable(u16 index) { return (u16)Call(natives_.WorkGet, index); }
  u16 GetVariable(ScriptVariable v) { return GetVariable((u16)v); }
  /// Sets a script variable.
  void SetVariable(u16 index, u16 value) {
    Call(natives_.WorkSet, index, value);
  }
  void SetVariable(ScriptVariable v, u16 value) { SetVariable((u16)v, value); }

  /// Returns true when the event flag is set.
  bool GetFlag(u16 flag_no) { return Call(natives_.FlagGet, flag_no) != 0; }
  /// Sets an event flag.
  void SetFlag(u16 flag_no) { Call(natives_.FlagSet, flag_no); }
  /// Clears an event flag.
  void ResetFlag(u16 flag_no) { Call(natives_.FlagReset, flag_no); }
  bool GetFlag(EventFlag f) { return GetFlag((u16)f); }
  void SetFlag(EventFlag f) { SetFlag((u16)f); }
  void ResetFlag(EventFlag f) { ResetFlag((u16)f); }

  /// @name The flags and the variables of the ROM hack
  /// 4096 flags and 512 variables in the save data. See savedata::HackSave.
  /// @{
  bool GetHackFlag(u16 flag) { return savedata::HackSave::GetFlag(flag); }
  void SetHackFlag(u16 flag) { savedata::HackSave::SetFlag(flag); }
  void ResetHackFlag(u16 flag) { savedata::HackSave::ResetFlag(flag); }
  u16 GetHackVariable(u16 variable) {
    return savedata::HackSave::GetVariable(variable);
  }
  void SetHackVariable(u16 variable, u16 value) {
    savedata::HackSave::SetVariable(variable, value);
  }
  /// @}

  /// Returns the character that the player talks to.
  s32 GetTalkTarget() { return (s16)GetVariable(ScriptVariable::kTalkTarget); }

  /// Returns true when the Bag has space for the items.
  bool CanGiveItem(ItemId item, u16 count = 1) {
    return Call(natives_.ItemAddCheck, (u16)item, count) != 0;
  }
  /// Gives items. Returns false when the Bag is full.
  bool GiveItem(ItemId item, u16 count = 1) {
    return Call(natives_.ItemAdd, (u16)item, count) != 0;
  }
  /// Returns the number of an item in the Bag.
  u32 CountItem(ItemId item) {
    return (u32)Call(natives_.ItemGetNum, (u16)item, 0);
  }
  /// Returns true when the Bag has the items.
  bool HasItem(ItemId item, u32 count = 1) { return CountItem(item) >= count; }
  /// Removes items from the Bag. Returns false when the Bag has not enough.
  bool TakeItem(ItemId item, u16 count = 1) {
    if (!HasItem(item, count)) return false;
    return CallNative("ItemSub", (u16)item, count) != 0;
  }
  /// Returns the money of the player.
  u32 GetMoney() { return (u32)Call(natives_.PlayerGetMoney); }
  /// Gives money.
  void AddMoney(u32 amount) { Call(natives_.PlayerAddMoney, amount); }
  /// Takes money.
  void SubMoney(u32 amount) { Call(natives_.PlayerSubMoney, amount); }
  /// Returns true when the player is female.
  bool IsPlayerFemale() { return Call(natives_.PlayerGetSex) != 0; }

  /// Plays a sound effect.
  void PlaySound(u32 sound_id) { Call(natives_.SEPlay, sound_id); }

  /// Plays a short music (for example kJingleItem) and waits for its end.
  void PlayJingle(u32 sound_id) {
    Call(natives_.MEPlay, sound_id);
    while (Call(natives_.MEIsFinished, sound_id) == 0) Yield();
    Call(natives_.MEReturnBGM);
  }

  /// Starts a conversation: the character turns to the player.
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

  /// Ends a conversation: the character goes back to normal.
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

  /// Shows a message. It does not wait for a button.
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

  /// Waits for the player to press a button.
  void WaitKey() {
    Call(natives_.LastKeyWait);
    Yield();
  }

  /// Waits for the player to press A or B.
  void WaitAbKey() {
    Call(natives_._ABKeyWait);
    Yield();
  }

  /// Closes a message window.
  void CloseMessage(u32 window_id = 0) {
    Call(natives_.MsgWinCloseNo, window_id);
  }

  /// Shows a message, waits for a button, then closes the message.
  void Talk(const c16* text, WindowType type = WindowType::kTalkVariable,
            s32 object_id = kTalkTarget) {
    ShowMessage(text, type, object_id);
    WaitKey();
    CloseMessage();
  }

  /// Shows Yes / No. Returns true for Yes.
  bool AskYesNo(bool cursor_on_no = false) {
    while (Call(natives_.YesNoWin_Seq, false, 0, 0, cursor_on_no ? 1 : 0) == 0) {
      Yield();
    }
    return GetVariable(ScriptVariable::kAnswer) != 0;
  }

  /// Starts a list of movements for a character.
  void Animate(s32 object_id, const Step* steps, u32 count,
               bool replace_turn = false) {
    Call(natives_.MdlAcmdInit, object_id);
    for (u32 i = 0; i < count; i++) {
      Call(natives_.MdlAcmdSet, (u16)steps[i].action, (u8)steps[i].direction,
           steps[i].count, replace_turn);
    }
    Call(natives_.MdlAcmdSetEnd);
  }

  /// Waits for the end of the movements.
  void WaitAnimation(s32 object_id = kTalkTarget) {
    while (Call(natives_.MdlAcmdUpdate, object_id) == 0) Yield();
  }

  /// Turns a character.
  void Face(s32 object_id, overworld::Facing direction) {
    const Step face[] = {{ActionCommand::kFaceRotate4Frames, direction, 1}};
    Animate(object_id, face, 1);
    WaitAnimation();
  }

  /// Moves a character `count` tiles.
  void Walk(s32 object_id, overworld::Facing direction, u16 count = 1,
            ActionCommand gait = ActionCommand::kWalk8Frames) {
    const Step walk[] = {{gait, direction, count}};
    Animate(object_id, walk, 1);
    WaitAnimation();
  }

  /// @name Battles
  /// After a lost battle, the player wakes up at the last Pokémon Center and
  /// the game ends the script: the next lines of the script do not run.
  /// @{

  /**
   * @brief Starts a battle against a trainer of the game, and waits for its end.
   *
   * The team of the trainer comes from the game, or from
   * battle::TrainerTeams when the product adds a team for this trainer.
   *
   * @code
   * if (s.TrainerBattle(TrainerId::kRoute103PokemonTrainerBrendan) == BattleResult::kWon) {
   *   s.Talk(u"You are strong!");
   * }
   * @endcode
   *
   * @param trainer The trainer.
   * @param options The options of the battle, for example
   *        TrainerBattleOption::kInverse.
   * @param second_trainer A second trainer for a double battle against two
   *        trainers, or TrainerId::kNone.
   * @return The result. kLost only with TrainerBattleOption::kCannotLose.
   */
  BattleResult TrainerBattle(
      TrainerId trainer,
      TrainerBattleOption options = TrainerBattleOption::kNone,
      TrainerId second_trainer = TrainerId::kNone) {
    if (natives_._CallTrainerBattleCore == nullptr) {
      ui::Log::Print(u"script: no trainer battle in this game");
      return BattleResult::kNotStarted;
    }
    // No partner, and the backgrounds and the effect of the trainer.
    Call(natives_._CallTrainerBattleCore, 0, (u16)trainer,
         (u16)second_trainer, (u32)options, 0, 0, 0);
    Yield();
    Yield();
    const bool can_lose = !HasFlag(options, TrainerBattleOption::kCannotLose);
    return EndBattle(natives_.CallTrainerLose, can_lose);
  }

  /**
   * @brief Starts a battle against a row of the static encounter table (a
   * legendary Pokémon, for example), and waits for its end.
   * @return The result of the battle.
   */
  BattleResult WildBattle(overworld::StaticEncounterId encounter) {
    if (natives_.CallWildBattle == nullptr) {
      ui::Log::Print(u"script: no wild battle in this game");
      return BattleResult::kNotStarted;
    }
    Call(natives_.CallWildBattle, (u32)encounter);
    Yield();
    Yield();
    return EndBattle(natives_._CallWildLose, true);
  }

  /**
   * @brief Starts a battle against a wild Pokémon, and waits for its end.
   *
   * The library lends a row of the static encounter table to the battle.
   * Alpha Sapphire only.
   *
   * @code
   * if (s.WildBattle(SpeciesId::kRayquaza, 70) == BattleResult::kCaught) {
   *   s.Talk(u"You caught it!");
   * }
   * @endcode
   * @return The result of the battle.
   */
  BattleResult WildBattle(SpeciesId species, u8 level,
                          FormId form = FormId::kNormal) {
    if (pokemon::address::kStaticEncounterTable == 0 ||
        natives_.CallWildBattle == nullptr) {
      ui::Log::Print(u"script: no wild battle in this game");
      return BattleResult::kNotStarted;
    }
    auto& row = overworld::StaticEncounter::GetInstance(kLentEncounter);
    const overworld::StaticEncounter backup = row;
    row.species = species;
    row.form = form;
    row.level = level;
    row.is_shiny = ShinyRoll::kRandom;
    row.gender = GenderRoll::kRandom;
    row.ability = AbilitySlot::kRandom;
    row.kind = StaticEncounterKind::kNormal;
    row.animation = static_cast<EncounterAnimationId>(0);

    Call(natives_.CallWildBattle, (u32)kLentEncounter);
    Yield();
    Yield();
    // Give the row back before a loss: the game then ends the script.
    const auto result = (BattleResult)Call(natives_.WildBattleResultGet);
    row = backup;
    return EndBattle(result, natives_._CallWildLose, true);
  }
  /// @}

  /// @name Places and screens
  /// @{

  /**
   * @brief Sends the player to a map, then continues the script there.
   *
   * The screen fades out and in, and the game shows the name of the map.
   * Find the position in the menu: Overworld > Characters (Tile X, Tile Z and
   * Height of the player).
   */
  void Warp(MapId map, u16 tile_x, u16 tile_z, u16 height,
            overworld::Facing facing = overworld::Facing::kDown) {
    if (natives_._MapChangeCore == nullptr) {
      ui::Log::Print(u"script: no warp in this game");
      return;
    }
    // Keep the music: no. Fade: yes. End the script: no. Map name: yes.
    Call(natives_._MapChangeCore, (u32)map, tile_x, height, tile_z,
         (u32)facing, false, true, false, true);
    Yield();
  }

  /// Makes the two screens black (or white), and waits for the end.
  void FadeOut(FadeColor color = FadeColor::kBlack, u32 frames = 15) {
    Call(natives_.FadeRequestOut, (u32)color, kBothScreens, frames);
    WaitFade();
  }

  /// Shows the two screens again after FadeOut(), and waits for the end.
  void FadeIn(u32 frames = 15) {
    Call(natives_.FadeRequestIn, kBothScreens, frames);
    WaitFade();
  }

  /// Heals all the Pokémon of the party: HP, status and PP.
  void HealParty() { Call(natives_.PokePartyRecoverAll); }
  /// @}

  /// @name Music
  /// @{

  /// Plays the music of an event (a cutscene, a boss...) in place of the music
  /// of the map. Call EndEventMusic() before the end of the script.
  /// Not in X and Y.
  void StartEventMusic(BackgroundMusicId music, u32 fade_frames = 10) {
    Call(natives_.BGMStartEvent,
         sys::Sound::kBankBackgroundMusic + (u32)music, fade_frames);
  }

  /// Plays the music of the map again after StartEventMusic().
  void EndEventMusic() { Call(natives_.BGMEndEvent, 90, 60); }
  /// @}

  /// @name Choices and Pokémon
  /// @{

  /**
   * @brief Shows a list of choices and waits for the choice of the player.
   *
   * @code
   * s.ShowMessage(u"Which Pokémon do you want?");
   * const s32 choice = s.Choose({u"Bulbasaur", u"Charmander", u"Squirtle"});
   * s.CloseMessage();
   * @endcode
   *
   * @param choices The texts of the choices. 32 choices at most.
   * @param can_cancel true: the player can press B.
   * @param first_choice The choice of the cursor at the start.
   * @return The index of the choice (0 for the first), or kChoiceCancel.
   */
  s32 Choose(std::initializer_list<const c16*> choices, bool can_cancel = true,
             u32 first_choice = 0) {
    return Choose(choices.begin(), choices.size(), can_cancel, first_choice);
  }

  /// Choose() with a table of texts and its size.
  s32 Choose(const c16* const* choices, u32 count, bool can_cancel = true,
             u32 first_choice = 0) {
    if (natives_.ListMenuInit_Seq == nullptr || natives_.ListMenuAdd == nullptr ||
        natives_.ListMenuStart_Seq == nullptr || count == 0) {
      return kChoiceCancel;
    }
    if (count > kMaxChoices) count = kMaxChoices;

    // The menu reads its texts from the message of the script: each text is
    // the message during one call.
    while (CallWithText(u"", natives_.ListMenuInit_Seq, kChoiceX, kChoiceY,
                        first_choice, can_cancel, false, true) == 0) {
      Yield();
    }
    for (u32 i = 0; i < count; i++) {
      CallWithText(choices[i], natives_.ListMenuAdd, 0, i, true);
    }
    while (Call(natives_.ListMenuStart_Seq) == 0) Yield();

    const u16 answer = GetVariable(ScriptVariable::kAnswer);
    return answer < count ? (s32)answer : kChoiceCancel;
  }

  /**
   * @brief Gives a Pokémon to the player. A full party sends it to the PC.
   *
   * The nature, the IVs, the ability and the gender are random. The library
   * lends a row of the gift table to the game.
   * @return false when the game cannot give the Pokémon.
   */
  bool GivePokemon(SpeciesId species, u8 level, FormId form = FormId::kNormal) {
    using Gift = pokemon::GiftPokemonData;
    auto& gift = Gift::GetInstance(kLentGift);
    const Gift backup = gift;
    gift.species = species;
    gift.form = form;
    gift.level = level;
    gift.shiny = ShinyRoll::kRandom;
    gift.ability_slot = Gift::kRandomAbility;
    gift.nature = Gift::kRandomNature;
    gift.item = Gift::kRandomItem;
    gift.gender = GenderRoll::kRandom;
    gift.egg_place = Gift::kNotAnEgg;
    gift.move = MoveId::kNone;
    for (u32 i = 0; i < SIZE(gift.iv); i++) {
      gift.iv[i] = Gift::kRandomIv;
      gift.contest[i] = 0;
    }
    const bool given = GivePokemon(kLentGift);
    gift = backup;
    return given;
  }
  /// @}

  /**
   * @brief Calls a native of the game with its name. Use it for a native
   * that the Context does not have yet.
   *
   * The names are in the scripts of the game (for example "PlayerGetZoneID").
   * A native with "_Seq" at the end of its name must be called again at each
   * frame, until it returns true. The arguments are numbers only.
   *
   * @code
   * const s32 map = s.CallNative("PlayerGetZoneID");
   * @endcode
   * @return The value of the native, or 0 when the game does not have it.
   */
  template <typename... Args>
  s32 CallNative(const c8* name, Args... args) {
    NativeFunction native = NativeTable::Find(name);
    if (native == nullptr) {
      LogMissingNative(name);
      return 0;
    }
    return Call(native, args...);
  }

private:
  static constexpr u32 kKneelingMoveCode = 60;
  /// The screens of a fade: 0 the top screen, 1 the bottom screen, 2 both.
  static constexpr u32 kBothScreens = 2;
  /// The position of the list of Choose(), the same as in the game.
  static constexpr u32 kChoiceX = 180;
  static constexpr u32 kChoiceY = 112;
  static constexpr u32 kMaxChoices = 32;
  /// The row of the static encounter table that WildBattle() lends: a normal
  /// wild battle of the game.
  static constexpr overworld::StaticEncounterId kLentEncounter =
      overworld::StaticEncounterId::kWurmple;
  /// The row of the gift table that GivePokemon() lends.
  static constexpr u32 kLentGift = 0;

  /// Reads the result of the last battle. After a win, shows the screens
  /// again. After a loss, `lose` makes the player wake up at the Pokémon
  /// Center.
  BattleResult EndBattle(NativeFunction lose, bool can_lose) {
    return EndBattle((BattleResult)Call(natives_.WildBattleResultGet), lose,
                     can_lose);
  }

  BattleResult EndBattle(BattleResult result, NativeFunction lose,
                         bool can_lose) {
    if (result == BattleResult::kLost && can_lose && lose != nullptr) {
      Call(lose);
      // The game ends the script: it does not come back here.
      Yield();
      return result;
    }
    // The same fade as the scripts of the game after a battle.
    Call(natives_.FadeRequestIn, kBothScreens, 4);
    WaitFade();
    return result;
  }

  /// Waits for the end of a fade.
  void WaitFade() {
    if (natives_.FadeIsEnd == nullptr) return;
    while (Call(natives_.FadeIsEnd) == 0) Yield();
  }

  /// Calls a native that reads the message of the script, with `text` as
  /// the message.
  template <typename... Args>
  s32 CallWithText(const c16* text, NativeFunction native, Args... args) {
    SharedWorkspaces* shared = GetSharedWorkspaces();
    if (shared == nullptr) return 0;
    BuildMessage(text);
    LoadedMessage* previous = shared->loaded_message;
    message_.buffer = message_buffer_;
    message_.data = nullptr;
    shared->loaded_message = &message_;
    const s32 value = Call(native, args...);
    shared->loaded_message = previous;
    return value;
  }

  static void LogMissingNative(const c8* name) {
    // The log formats its text: copy the name, without the % of a format.
    c16 text[64] = u"script: no native ";
    u32 length = 0;
    while (text[length] != u'\0') length++;
    for (u32 i = 0; name[i] != '\0' && length + 1 < SIZE(text); i++) {
      text[length++] = name[i] == '%' ? u'?' : (c16)name[i];
    }
    text[length] = u'\0';
    ui::Log::Print(text);
  }

  bool IsKneelingNpc(s32 object_id) {
    return Call(natives_.MdlGetMoveCode, object_id) == kKneelingMoveCode;
  }

  SharedWorkspaces* GetSharedWorkspaces() const {
    Engine* engine = Engine::GetInstance();
    if (engine == nullptr || engine->GetWorkspace() == nullptr) return nullptr;
    return &engine->GetWorkspace()->shared;
  }

  u32 BuildMessage(const c16* text) {
    const u32 capacity =
        message_capacity_ -
        (sizeof(MessageHeader) + sizeof(MessageLanguageBlock)) / sizeof(c16);

    auto* header = (MessageHeader*)message_buffer_;
    auto* block = (MessageLanguageBlock*)(header + 1);
    auto* string = (c16*)(block + 1);
    // The translation of the text, then the values of its {names}.
    const u32 length = ExpandText(core::Translate(text), string, capacity);

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
    return 0;
  }

  /// Copies `text` into `out` and replaces the names between braces:
  /// {player}, {money} and {var:N}. Returns the length of `out`.
  u32 ExpandText(const c16* text, c16* out, u32 capacity) {
    u32 length = 0;
    for (u32 i = 0; text[i] != u'\0' && length + 1 < capacity; i++) {
      if (text[i] == u'{') {
        const u32 used = ExpandName(text + i, out, &length, capacity);
        if (used != 0) {
          i += used - 1;
          continue;
        }
      }
      out[length++] = text[i];
    }
    out[length] = u'\0';
    return length;
  }

  /// Writes the value of the name at the start of `text` ("{player}"...).
  /// Returns the number of characters of the name, or 0 for an unknown name.
  u32 ExpandName(const c16* text, c16* out, u32* length, u32 capacity) {
    if (StartsWith(text, u"{player}")) {
      const c16* name = savedata::TrainerStatus::GetInstance().name;
      for (u32 k = 0; k < savedata::TrainerStatus::kPlayerNameLen &&
                      name[k] != u'\0' && *length + 1 < capacity; k++) {
        out[(*length)++] = name[k];
      }
      return 8;
    }
    if (StartsWith(text, u"{money}")) {
      AppendNumber(GetMoney(), out, length, capacity);
      return 7;
    }
    if (StartsWith(text, u"{var:")) {
      u32 variable = 0;
      u32 i = 5;
      for (; text[i] >= u'0' && text[i] <= u'9'; i++) {
        variable = variable * 10 + (text[i] - u'0');
      }
      if (i == 5 || text[i] != u'}') return 0;
      AppendNumber(savedata::HackSave::GetVariable((u16)variable), out, length,
                   capacity);
      return i + 1;
    }
    return 0;
  }

  static bool StartsWith(const c16* text, const c16* start) {
    for (u32 i = 0; start[i] != u'\0'; i++) {
      if (text[i] != start[i]) return false;
    }
    return true;
  }

  static void AppendNumber(u32 value, c16* out, u32* length, u32 capacity) {
    c16 digits[10];
    u32 count = 0;
    do {
      digits[count++] = (c16)(u'0' + value % 10);
      value /= 10;
    } while (value != 0);
    while (count > 0 && *length + 1 < capacity) {
      out[(*length)++] = digits[--count];
    }
  }

  AmxRuntime* amx_ = nullptr;
  const Natives& natives_;
  Coroutine& coroutine_;
  c16* message_buffer_;
  u32 message_capacity_;
  LoadedMessage message_ = {nullptr, nullptr};
};

/**
 * @brief A conversation with a character: TalkStart() now, and TalkEnd() at
 * the end of the block. A `return` in the block is safe.
 *
 * @code
 * void Greeter(script::Context& s) {
 *   script::Conversation talk(s);
 *   if (s.GetHackFlag(0)) {
 *     s.Talk(u"Hello again!");
 *     return;  // TalkEnd() runs here.
 *   }
 *   s.Talk(u"Hello!");
 * }
 * @endcode
 */
class Conversation {
public:
  explicit Conversation(Context& s, s32 object_id = kTalkTarget,
                        TalkOption options = TalkOption::kMotion)
    : s_(s) {
    s_.TalkStart(object_id, options);
  }
  ~Conversation() { s_.TalkEnd(); }

  Conversation(const Conversation&) = delete;
  Conversation& operator=(const Conversation&) = delete;

private:
  Context& s_;
};

typedef void (*Function)(Context& script);
} // namespace script
