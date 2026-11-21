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
 * @file studio_events.cc
 * @brief Runs the events that the user made in Sango Studio.
 *
 * The declarations and the format of the files are in
 * script/patch/studio_events.h.
 */

#include "script/patch/studio_events.h"

#include <cstring>

#include "core/heap_buffer.h"
#include "core/patch/text_editor.h"
#include "script/patch/native_script.h"
#include "system/native/file.h"
#include "ui/log.h"

namespace script {

namespace {

constexpr u32 kFileLimit = 16 * 1024;
constexpr u32 kStepHeaderSize = 2 + 2 + 4 + 4 + 4 + 2;
constexpr u32 kTextCapacity = 256;
constexpr u32 kChoiceCount = 3;

/// One step that was read from the file.
struct EventStep {
  u16 action;
  u16 target;
  u32 a;
  u32 b;
  u32 c;
  const u8* text; ///< The characters (UTF-16), in the file.
  u32 text_length;
};

u16 ReadU16(const u8* p) { return static_cast<u16>(p[0] | (p[1] << 8)); }

u32 ReadU32(const u8* p) {
  return static_cast<u32>(p[0]) | (static_cast<u32>(p[1]) << 8) |
         (static_cast<u32>(p[2]) << 16) | (static_cast<u32>(p[3]) << 24);
}

/// Writes the path of the file of an event.
void MakePath(u32 number, c16* path) {
  static const c16 kPrefix[] = u"sdmc:/sango-studio/event/";
  u32 n = 0;
  for (; kPrefix[n] != 0; n++) path[n] = kPrefix[n];
  c16 digits[4];
  u32 count = 0;
  do {
    digits[count++] = static_cast<c16>(u'0' + number % 10);
    number /= 10;
  } while (number != 0);
  while (count != 0) path[n++] = digits[--count];
  static const c16 kExtension[] = u".bin";
  for (u32 i = 0; kExtension[i] != 0; i++) path[n++] = kExtension[i];
  path[n] = 0;
}

/// Reads the steps of the file in `buffer`. Returns the number of steps, or 0.
u32 ReadSteps(const u8* buffer, u32 size, EventStep* steps) {
  if (size < 8 || std::memcmp(buffer, "SEVT", 4) != 0) return 0;
  if (ReadU16(buffer + 4) != 1) return 0;
  u32 count = ReadU16(buffer + 6);
  if (count > StudioEvents::kMaxSteps) count = StudioEvents::kMaxSteps;
  u32 at = 8;
  for (u32 i = 0; i < count; i++) {
    if (at + kStepHeaderSize > size) return i;
    EventStep& step = steps[i];
    step.action = ReadU16(buffer + at);
    step.target = ReadU16(buffer + at + 2);
    step.a = ReadU32(buffer + at + 4);
    step.b = ReadU32(buffer + at + 8);
    step.c = ReadU32(buffer + at + 12);
    step.text_length = ReadU16(buffer + at + 16);
    at += kStepHeaderSize;
    if (at + 2 * step.text_length > size) return i;
    step.text = buffer + at;
    at += 2 * step.text_length;
  }
  return count;
}

/// Copies the characters of a step (the part from `first` to `last`) and
/// converts the readable form into the codes of the game.
bool ReadText(const EventStep& step, u32 first, u32 last, c16* raw) {
  c16 marked[kTextCapacity];
  u32 length = 0;
  for (u32 i = first; i < last && i < step.text_length && length + 1 < kTextCapacity; i++) {
    marked[length++] = static_cast<c16>(ReadU16(step.text + 2 * i));
  }
  marked[length] = 0;
  u32 raw_length = 0;
  const c8* error = nullptr;
  return core::TextEditor::Decode(marked, raw, kTextCapacity, raw_length, &error);
}

bool TestFlag(Context& context, const EventStep& step) {
  return step.b == 0 ? context.GetFlag(static_cast<u16>(step.a))
                     : context.GetHackFlag(static_cast<u16>(step.a));
}

bool TestVariable(Context& context, const EventStep& step) {
  const u16 value = context.GetHackVariable(static_cast<u16>(step.a));
  const u16 wanted = static_cast<u16>(step.b);
  switch (step.c) {
    case 1: return value != wanted;
    case 2: return value > wanted;
    case 3: return value < wanted;
    default: return value == wanted;
  }
}

/// Shows the question and the choices. Returns the step to go to.
u16 RunChoice(Context& context, const EventStep& step) {
  // The text is cut at each bar.
  u32 starts[kChoiceCount + 1];
  u32 ends[kChoiceCount + 1];
  u32 parts = 0;
  u32 begin = 0;
  for (u32 i = 0; i <= step.text_length && parts < kChoiceCount + 1; i++) {
    if (i == step.text_length || ReadU16(step.text + 2 * i) == u'|') {
      starts[parts] = begin;
      ends[parts] = i;
      parts++;
      begin = i + 1;
    }
  }
  if (parts < 2) return step.target;
  c16 question[kTextCapacity];
  c16 options[kChoiceCount][kTextCapacity];
  const c16* list[kChoiceCount];
  const u32 count = parts - 1;
  if (!ReadText(step, starts[0], ends[0], question)) return step.target;
  for (u32 i = 0; i < count; i++) {
    if (!ReadText(step, starts[i + 1], ends[i + 1], options[i])) {
      return step.target;
    }
    list[i] = options[i];
  }
  context.ShowMessage(question);
  const s32 chosen = context.Choose(list, count, false);
  context.CloseMessage();
  if (chosen < 0) return step.target;
  const u32 targets[kChoiceCount] = {step.a, step.b, step.c};
  return static_cast<u16>(targets[chosen]);
}

} // namespace

void StudioEvents::Initialize() {
  for (u32 i = 0; i < kMaxEvents; i++) {
    NativeScript::Register(static_cast<ScriptId>(kFirstScript + i), Run);
  }
}

void StudioEvents::Run(Context& context) {
  const u32 id = static_cast<u32>(NativeScript::GetCurrentId());
  if (id < kFirstScript || id >= kFirstScript + kMaxEvents) return;
  c16 path[64];
  MakePath(id - kFirstScript, path);
  core::HeapBuffer file(kFileLimit);
  if (!file.IsValid()) return;
  const u32 size = sys::File::ReadAll(path, file.Get(), file.Size());
  EventStep steps[kMaxSteps];
  const u32 count = ReadSteps(file.Get(), size, steps);
  if (count == 0) {
    ui::Log::Print(u"event %u: no file or no step", id - kFirstScript);
    return;
  }

  u32 at = 0;
  for (u32 executed = 0; at < count && executed < kMaxExecuted; executed++) {
    const EventStep& step = steps[at];
    u32 next = at + 1;
    bool jump = false;
    switch (step.action) {
      case kStop:
        return;
      case kSay: {
        c16 raw[kTextCapacity];
        if (ReadText(step, 0, step.text_length, raw)) {
          context.Talk(raw, step.a != 0 ? WindowType::kTalkVariable
                                        : WindowType::kSign);
        }
        break;
      }
      case kGiveItem: {
        const u16 number = step.b == 0 ? 1 : static_cast<u16>(step.b);
        const ItemId item = static_cast<ItemId>(step.a);
        if (context.CanGiveItem(item, number)) context.GiveItem(item, number);
        break;
      }
      case kTakeItem:
        context.TakeItem(static_cast<ItemId>(step.a),
                         step.b == 0 ? 1 : static_cast<u16>(step.b));
        break;
      case kMoney:
        if (step.b == 0) {
          context.AddMoney(step.a);
        } else {
          context.SubMoney(step.a);
        }
        break;
      case kSetFlag:
        if (step.b == 0) {
          context.SetFlag(static_cast<u16>(step.a));
        } else {
          context.SetHackFlag(static_cast<u16>(step.a));
        }
        break;
      case kResetFlag:
        if (step.b == 0) {
          context.ResetFlag(static_cast<u16>(step.a));
        } else {
          context.ResetHackFlag(static_cast<u16>(step.a));
        }
        break;
      case kSetVariable:
        context.SetHackVariable(static_cast<u16>(step.a), static_cast<u16>(step.b));
        break;
      case kSound:
        context.PlaySound(step.a);
        break;
      case kJingle:
        context.PlayJingle(step.a);
        break;
      case kWait:
        context.Wait(step.a);
        break;
      case kRunScript:
        context.CallScript(static_cast<ScriptId>(step.a));
        break;
      case kGivePokemon:
        context.GivePokemon(static_cast<SpeciesId>(step.a), static_cast<u8>(step.b),
                            static_cast<FormId>(step.c));
        break;
      case kWildBattle:
        context.WildBattle(static_cast<SpeciesId>(step.a), static_cast<u8>(step.b),
                           static_cast<FormId>(step.c));
        break;
      case kTrainerBattle:
        if (context.TrainerBattle(static_cast<TrainerId>(step.a)) ==
            BattleResult::kLost) {
          return;
        }
        break;
      case kWarp:
        context.Warp(static_cast<MapId>(step.a), static_cast<u16>(step.b),
                     static_cast<u16>(step.c), 0);
        break;
      case kHeal:
        context.HealParty();
        break;
      case kFade:
        if (step.a == 0) {
          context.FadeOut();
        } else {
          context.FadeIn();
        }
        break;
      case kJump:
        jump = true;
        break;
      case kIfFlag:
        jump = !TestFlag(context, step);
        break;
      case kIfItem:
        jump = !context.HasItem(static_cast<ItemId>(step.a), step.b == 0 ? 1 : step.b);
        break;
      case kIfMoney:
        jump = context.GetMoney() < step.a;
        break;
      case kIfVariable:
        jump = !TestVariable(context, step);
        break;
      case kAsk:
        jump = !context.AskYesNo();
        break;
      case kChoice: {
        const u16 to = RunChoice(context, step);
        if (to == kEnd) return;
        next = to;
        break;
      }
      default:
        break;
    }
    if (jump) {
      if (step.target == kEnd) return;
      next = step.target;
    }
    at = next;
    context.Yield();
  }
}

} // namespace script
