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
 * @file data_scripts.cc
 * @brief Scripts that the player writes in the menu, without code.
 *
 * The declarations are in script/patch/data_scripts.h.
 */

#include "script/patch/data_scripts.h"

#include <cstdio>
#include <cstring>

#include "core/patch/text_editor.h"
#include "script/patch/native_script.h"
#include "system/native/file.h"

namespace script {

namespace {
const c16 kFolder[] = u"sdmc:/sango";
const c16 kPath[] = u"sdmc:/sango/scripts.txt";
constexpr u32 kLineSize = 320;
constexpr u32 kFileLimit = 12288;
constexpr u32 kPollFrames = 45;

template <u32 Slot>
void Runner(Context& context) {
  DataScripts::Run(context, Slot);
}

const Function kRunners[DataScripts::kMaxScripts] = {
    Runner<0>, Runner<1>, Runner<2>,  Runner<3>, Runner<4>,  Runner<5>,
    Runner<6>, Runner<7>, Runner<8>,  Runner<9>, Runner<10>, Runner<11>,
};

bool ReadInt(const c8*& p, s32& value) {
  while (*p == ' ') p++;
  if (*p < '0' || *p > '9') return false;
  s32 result = 0;
  while (*p >= '0' && *p <= '9') result = result * 10 + (*p++ - '0');
  value = result;
  return true;
}
} // namespace

void DataScripts::Initialize() {
  for (u32 i = 0; i < kMaxScripts; i++) {
    NativeScript::Register(static_cast<ScriptId>(kFirstScript + i), kRunners[i]);
  }
  Load();
}

DataScripts::Step* DataScripts::GetStep(u32 script, u32 step) {
  if (script >= kMaxScripts || step >= kMaxSteps) return nullptr;
  return &GetInstance().steps_[script][step];
}

bool DataScripts::IsUsed(u32 script) {
  if (script >= kMaxScripts) return false;
  for (const Step& step : GetInstance().steps_[script]) {
    if (step.action != kNone) return true;
  }
  return false;
}

bool DataScripts::LoadLine(const c8* line) {
  const c8* p = line;
  s32 script;
  s32 step;
  s32 action;
  s32 parameter_1;
  s32 parameter_2;
  if (!ReadInt(p, script) || !ReadInt(p, step) || !ReadInt(p, action) ||
      !ReadInt(p, parameter_1) || !ReadInt(p, parameter_2)) {
    return false;
  }
  Step* target = GetStep(static_cast<u32>(script), static_cast<u32>(step));
  if (target == nullptr || action < 0 || action >= kActionCount) return false;
  if (*p == ' ') p++;
  c16 joined[2 * kPartLength];
  const u32 length = core::TextEditor::FromUtf8(
      p, static_cast<u32>(strlen(p)), joined, sizeof(joined) / sizeof(joined[0]));
  *target = Step();
  target->action = static_cast<u8>(action);
  target->parameter_1 = static_cast<u16>(parameter_1);
  target->parameter_2 = static_cast<u16>(parameter_2);
  // The first 31 characters are the first part.
  for (u32 i = 0; i < length; i++) {
    if (i < kPartLength - 1) {
      target->text[i] = joined[i];
    } else if (i - (kPartLength - 1) < kPartLength - 1) {
      target->text_2[i - (kPartLength - 1)] = joined[i];
    }
  }
  return true;
}

bool DataScripts::Load() {
  auto& self = GetInstance();
  for (auto& script : self.steps_) {
    for (Step& step : script) step = Step();
  }
  sys::File file;
  file.Open(kPath, sys::File::kRead);
  if (!file.IsOpen()) return false;
  c8 chunk[128];
  c8 line[kLineSize];
  u32 length = 0;
  for (;;) {
    const s32 read = file.Read(chunk, sizeof(chunk));
    if (read <= 0) break;
    for (s32 i = 0; i < read; i++) {
      const c8 c = chunk[i];
      if (c == '\n') {
        line[length] = '\0';
        if (length != 0 && line[0] != '#') LoadLine(line);
        length = 0;
      } else if (c != '\r' && length < kLineSize - 1) {
        line[length++] = c;
      }
    }
    if (static_cast<u32>(read) < sizeof(chunk)) break;
  }
  line[length] = '\0';
  if (length != 0 && line[0] != '#') LoadLine(line);
  return true;
}

bool DataScripts::Save() {
  auto& self = GetInstance();
  bool has_step = false;
  for (u32 s = 0; s < kMaxScripts; s++) {
    if (IsUsed(s)) has_step = true;
  }
  if (!has_step) {
    sys::File::Delete(kPath);
    self.has_baseline_ = false;
    return true;
  }
  sys::File::CreateDirectory(kFolder);
  sys::File file(kPath, true);
  if (!file.IsOpen()) return false;
  static const c8 kHeader[] =
      "# Sango scripts. script step action parameter1 parameter2 text\n";
  file.Write(kHeader, sizeof(kHeader) - 1);
  for (u32 s = 0; s < kMaxScripts; s++) {
    for (u32 i = 0; i < kMaxSteps; i++) {
      const Step& step = self.steps_[s][i];
      if (step.action == kNone) continue;
      c16 joined[2 * kPartLength];
      u32 length = 0;
      for (u32 k = 0; step.text[k] != 0 && k < kPartLength; k++) {
        joined[length++] = step.text[k];
      }
      for (u32 k = 0; step.text_2[k] != 0 && k < kPartLength; k++) {
        joined[length++] = step.text_2[k];
      }
      joined[length] = 0;
      c8 line[kLineSize];
      int used = snprintf(line, 40, "%u %u %u %u %u ", (unsigned)s, (unsigned)i,
                          (unsigned)step.action, (unsigned)step.parameter_1,
                          (unsigned)step.parameter_2);
      if (used < 0) continue;
      used += static_cast<int>(core::TextEditor::ToUtf8(
          joined, line + used, kLineSize - static_cast<u32>(used) - 2));
      line[used++] = '\n';
      file.Write(line, static_cast<u32>(used));
    }
  }
  self.has_baseline_ = false;
  return true;
}

u32 DataScripts::HashFile() {
  static u8 buffer[kFileLimit];
  const u32 size = sys::File::ReadAll(kPath, buffer, kFileLimit);
  u32 hash = 2166136261u ^ size;
  for (u32 i = 0; i < size; i++) {
    hash ^= buffer[i];
    hash *= 16777619u;
  }
  return hash;
}

void DataScripts::Update() {
  auto& self = GetInstance();
  if (!self.is_live) return;
  if (++self.frame_ % kPollFrames != 0) return;
  const u32 hash = HashFile();
  if (!self.has_baseline_) {
    self.file_hash_ = hash;
    self.has_baseline_ = true;
    return;
  }
  if (hash != self.file_hash_) {
    self.file_hash_ = hash;
    Load();
  }
}

void DataScripts::Run(Context& context, u32 script) {
  if (script >= kMaxScripts) return;
  auto& self = GetInstance();
  for (u32 i = 0; i < kMaxSteps; i++) {
    const Step& step = self.steps_[script][i];
    switch (step.action) {
      case kNone:
        return;
      case kSay: {
        c16 marked[2 * kPartLength];
        u32 length = 0;
        for (u32 k = 0; step.text[k] != 0 && k < kPartLength; k++) {
          marked[length++] = step.text[k];
        }
        for (u32 k = 0; step.text_2[k] != 0 && k < kPartLength; k++) {
          marked[length++] = step.text_2[k];
        }
        marked[length] = 0;
        c16 raw[core::TextEditor::kMaxLength + 1];
        u32 raw_length = 0;
        const c8* error = nullptr;
        if (core::TextEditor::Decode(marked, raw, core::TextEditor::kMaxLength + 1,
                                     raw_length, &error)) {
          context.Talk(raw, step.parameter_1 != 0 ? WindowType::kTalkVariable
                                                  : WindowType::kSign);
        }
        break;
      }
      case kAsk:
        if (!context.AskYesNo()) return;
        break;
      case kGiveItem: {
        const ItemId item = static_cast<ItemId>(step.parameter_1);
        const u16 count = step.parameter_2 == 0 ? 1 : step.parameter_2;
        if (context.CanGiveItem(item, count)) context.GiveItem(item, count);
        break;
      }
      case kAddMoney:
        if (step.parameter_2 == 0) {
          context.AddMoney(step.parameter_1);
        } else {
          context.SubMoney(step.parameter_1);
        }
        break;
      case kSetFlag:
        context.SetFlag(step.parameter_1);
        break;
      case kResetFlag:
        context.ResetFlag(step.parameter_1);
        break;
      case kRequireFlag:
        if (!context.GetFlag(step.parameter_1)) return;
        break;
      case kSound:
        context.PlaySound(step.parameter_1);
        break;
      case kWait:
        context.Wait(step.parameter_1);
        break;
      case kRunScript:
        context.CallScript(static_cast<ScriptId>(step.parameter_1));
        break;
      case kGivePokemon:
        context.GivePokemon(step.parameter_1);
        break;
      default:
        break;
    }
  }
}

} // namespace script
