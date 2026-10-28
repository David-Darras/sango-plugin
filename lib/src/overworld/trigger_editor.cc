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
 * @file trigger_editor.cc
 * @brief Adds and changes the signs and the triggers of the map.
 *
 * The declarations are in overworld/patch/trigger_editor.h.
 */

#include "overworld/patch/trigger_editor.h"

#include <cstdio>
#include <cstring>

#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "system/native/file.h"

namespace overworld {

namespace {
const c16 kFolder[] = u"sdmc:/sango";
const c16 kPath[] = u"sdmc:/sango/map_events.txt";

// The talk direction "all sides" of a sign.
constexpr u8 kTalkAllSides = 4;
constexpr u32 kLineSize = 160;

bool ReadInt(const c8*& p, s32& value) {
  while (*p == ' ') p++;
  bool is_negative = false;
  if (*p == '-') {
    is_negative = true;
    p++;
  }
  if (*p < '0' || *p > '9') return false;
  s32 result = 0;
  while (*p >= '0' && *p <= '9') result = result * 10 + (*p++ - '0');
  value = is_negative ? -result : result;
  return true;
}

bool IsSame(const void* a, const void* b, u32 size) {
  return memcmp(a, b, size) == 0;
}
} // namespace

TriggerEditor::Record* TriggerEditor::NewRecord() {
  for (Record& record : records_) {
    if (!record.is_used) {
      record = Record();
      record.is_used = true;
      return &record;
    }
  }
  return nullptr;
}

bool TriggerEditor::LoadLine(const c8* line) {
  const c8* p = line;
  s32 map;
  s32 index;
  if (!ReadInt(p, map)) return false;
  while (*p == ' ') p++;
  const c8 type = p[0];
  const c8 mode = p[1];
  if ((type != 'S' && type != 'T') || (mode != '+' && mode != '=')) return false;
  p += 2;
  if (!ReadInt(p, index)) return false;
  s32 f[9] = {};
  const u32 count = type == 'S' ? 8 : 9;
  for (u32 i = 0; i < count; i++) {
    if (!ReadInt(p, f[i])) return false;
  }
  Record* record = GetInstance().NewRecord();
  if (record == nullptr) return false;
  record->map = static_cast<u16>(map);
  record->index = static_cast<u16>(index);
  if (type == 'S') {
    record->kind = mode == '+' ? kAddedSign : kEditedSign;
    SignEvent& sign = record->sign;
    sign.tiles.tile_x = static_cast<s16>(f[0]);
    sign.tiles.tile_z = static_cast<s16>(f[1]);
    sign.tiles.width = static_cast<u16>(f[2]);
    sign.tiles.depth = static_cast<u16>(f[3]);
    sign.tiles.height = f[4];
    sign.id = static_cast<u16>(f[5]);
    sign.talk_facing = static_cast<Facing>(f[6]);
    sign.kind = static_cast<u16>(f[7]);
    sign.position_kind = 0;
  } else {
    record->kind = mode == '+' ? kAddedTrigger : kEditedTrigger;
    TriggerEvent& trigger = record->trigger;
    trigger.tiles.tile_x = static_cast<u16>(f[0]);
    trigger.tiles.tile_z = static_cast<u16>(f[1]);
    trigger.tiles.width = static_cast<u16>(f[2]);
    trigger.tiles.depth = static_cast<u16>(f[3]);
    trigger.tiles.height = static_cast<s16>(f[4]);
    trigger.id = static_cast<u16>(f[5]);
    trigger.work_id = static_cast<u16>(f[6]);
    trigger.param = static_cast<u16>(f[7]);
    trigger.check_kind = static_cast<u16>(f[8]);
    trigger.position_kind = 0;
  }
  return true;
}

void TriggerEditor::Initialize() {
  sys::File file;
  file.Open(kPath, sys::File::kRead);
  if (!file.IsOpen()) return;
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
}

void TriggerEditor::TakeTables() {
  shipped_sign_count_ = sign_count_;
  shipped_trigger_count_ = trigger_count_;
  memcpy(shipped_signs_, signs_, sizeof(signs_));
  memcpy(shipped_triggers_, triggers_, sizeof(triggers_));
}

void TriggerEditor::ApplyRecords() {
  for (const Record& record : records_) {
    if (!record.is_used || record.map != map_) continue;
    switch (record.kind) {
      case kEditedSign:
        if (record.index < shipped_sign_count_) signs_[record.index] = record.sign;
        break;
      case kEditedTrigger:
        if (record.index < shipped_trigger_count_) {
          triggers_[record.index] = record.trigger;
        }
        break;
      case kAddedSign:
        if (sign_count_ < kMaxSigns) signs_[sign_count_++] = record.sign;
        break;
      case kAddedTrigger:
        if (trigger_count_ < kMaxTriggers) {
          triggers_[trigger_count_++] = record.trigger;
        }
        break;
    }
  }
}

void TriggerEditor::OnMapEventsLoaded(MapEventData* events) {
  auto& self = GetInstance();
  self.events_ = nullptr;
  if (events == nullptr) return;
  // A map with more events than the tables: the editor leaves it alone.
  if (events->sign_count > kMaxSigns || events->trigger_count > kMaxTriggers) {
    return;
  }
  if ((events->sign_count != 0 && events->signs == nullptr) ||
      (events->trigger_count != 0 && events->triggers == nullptr)) {
    return;
  }
  memset(self.signs_, 0, sizeof(self.signs_));
  memset(self.triggers_, 0, sizeof(self.triggers_));
  for (u32 i = 0; i < events->sign_count; i++) self.signs_[i] = events->signs[i];
  for (u32 i = 0; i < events->trigger_count; i++) {
    self.triggers_[i] = events->triggers[i];
  }
  self.sign_count_ = events->sign_count;
  self.trigger_count_ = events->trigger_count;
  self.map_ = static_cast<u16>(events->map_id);
  self.TakeTables();
  self.ApplyRecords();
  events->signs = self.signs_;
  events->sign_count = self.sign_count_;
  events->triggers = self.triggers_;
  events->trigger_count = self.trigger_count_;
  self.events_ = events;
}

bool TriggerEditor::IsActive() {
  const auto& self = GetInstance();
  return self.events_ != nullptr &&
         static_cast<u16>(self.events_->map_id) ==
             static_cast<u16>(MapManager::GetInstance().GetMap());
}

const MapEventData* TriggerEditor::GetEvents() {
  return IsActive() ? GetInstance().events_ : nullptr;
}

u32 TriggerEditor::GetMap() { return GetInstance().map_; }
u32 TriggerEditor::GetSignCount() { return GetInstance().sign_count_; }
u32 TriggerEditor::GetShippedSignCount() {
  return GetInstance().shipped_sign_count_;
}
u32 TriggerEditor::GetTriggerCount() { return GetInstance().trigger_count_; }
u32 TriggerEditor::GetShippedTriggerCount() {
  return GetInstance().shipped_trigger_count_;
}

SignEvent* TriggerEditor::GetSign(u32 index) {
  auto& self = GetInstance();
  return index < self.sign_count_ ? &self.signs_[index] : nullptr;
}

TriggerEvent* TriggerEditor::GetTrigger(u32 index) {
  auto& self = GetInstance();
  return index < self.trigger_count_ ? &self.triggers_[index] : nullptr;
}

SignEvent* TriggerEditor::AddSign() {
  auto& self = GetInstance();
  if (!IsActive() || self.sign_count_ >= kMaxSigns) return nullptr;
  SignEvent* sign = &self.signs_[self.sign_count_];
  *sign = SignEvent();
  sign->id = kNoScript;
  sign->kind = 0;
  sign->talk_facing = static_cast<Facing>(kTalkAllSides);
  sign->position_kind = 0;
  sign->tiles.width = 1;
  sign->tiles.depth = 1;
  MoveToPlayer(sign);
  self.sign_count_++;
  self.events_->sign_count = self.sign_count_;
  return sign;
}

TriggerEvent* TriggerEditor::AddTrigger() {
  auto& self = GetInstance();
  if (!IsActive() || self.trigger_count_ >= kMaxTriggers) return nullptr;
  TriggerEvent* trigger = &self.triggers_[self.trigger_count_];
  *trigger = TriggerEvent();
  trigger->id = kNoScript;
  trigger->position_kind = 0;
  trigger->tiles.width = 1;
  trigger->tiles.depth = 1;
  MoveToPlayer(trigger);
  self.trigger_count_++;
  self.events_->trigger_count = self.trigger_count_;
  return trigger;
}

void TriggerEditor::MoveToPlayer(SignEvent* sign) {
  auto& player = ModelManager::GetInstance().GetPlayer();
  sign->tiles.tile_x = static_cast<s16>(static_cast<s32>(player.map_pos.coords.x));
  sign->tiles.tile_z = static_cast<s16>(static_cast<s32>(player.map_pos.coords.z));
  sign->tiles.height = static_cast<s32>(player.GetDrawModel().position.y);
}

void TriggerEditor::MoveToPlayer(TriggerEvent* trigger) {
  auto& player = ModelManager::GetInstance().GetPlayer();
  trigger->tiles.tile_x = static_cast<u16>(static_cast<s32>(player.map_pos.coords.x));
  trigger->tiles.tile_z = static_cast<u16>(static_cast<s32>(player.map_pos.coords.z));
  trigger->tiles.height = static_cast<s16>(player.GetDrawModel().position.y);
}

bool TriggerEditor::RemoveSign(u32 index) {
  auto& self = GetInstance();
  if (!IsActive() || index < self.shipped_sign_count_ ||
      index >= self.sign_count_) {
    return false;
  }
  for (u32 i = index; i + 1 < self.sign_count_; i++) {
    self.signs_[i] = self.signs_[i + 1];
  }
  self.sign_count_--;
  self.events_->sign_count = self.sign_count_;
  return true;
}

bool TriggerEditor::RemoveTrigger(u32 index) {
  auto& self = GetInstance();
  if (!IsActive() || index < self.shipped_trigger_count_ ||
      index >= self.trigger_count_) {
    return false;
  }
  for (u32 i = index; i + 1 < self.trigger_count_; i++) {
    self.triggers_[i] = self.triggers_[i + 1];
  }
  self.trigger_count_--;
  self.events_->trigger_count = self.trigger_count_;
  return true;
}

bool TriggerEditor::IsSignChanged(u32 index) {
  auto& self = GetInstance();
  if (index >= self.sign_count_) return false;
  if (index >= self.shipped_sign_count_) return true;
  return !IsSame(&self.signs_[index], &self.shipped_signs_[index],
                 sizeof(SignEvent));
}

bool TriggerEditor::IsTriggerChanged(u32 index) {
  auto& self = GetInstance();
  if (index >= self.trigger_count_) return false;
  if (index >= self.shipped_trigger_count_) return true;
  return !IsSame(&self.triggers_[index], &self.shipped_triggers_[index],
                 sizeof(TriggerEvent));
}

u32 TriggerEditor::GetRecordCount() {
  u32 count = 0;
  for (const Record& record : GetInstance().records_) {
    if (record.is_used) count++;
  }
  return count;
}

bool TriggerEditor::Save() {
  auto& self = GetInstance();
  if (!IsActive()) return false;

  // The changes of this map need free records: check before any change.
  u32 needed = 0;
  for (u32 i = 0; i < self.sign_count_; i++) {
    if (IsSignChanged(i)) needed++;
  }
  for (u32 i = 0; i < self.trigger_count_; i++) {
    if (IsTriggerChanged(i)) needed++;
  }
  u32 others = 0;
  for (const Record& record : self.records_) {
    if (record.is_used && record.map != self.map_) others++;
  }
  if (others + needed > kMaxRecords) return false;

  for (Record& record : self.records_) {
    if (record.is_used && record.map == self.map_) record.is_used = false;
  }
  for (u32 i = 0; i < self.sign_count_; i++) {
    if (!IsSignChanged(i)) continue;
    Record* record = self.NewRecord();
    record->kind = i < self.shipped_sign_count_ ? kEditedSign : kAddedSign;
    record->map = self.map_;
    record->index = static_cast<u16>(i);
    record->sign = self.signs_[i];
  }
  for (u32 i = 0; i < self.trigger_count_; i++) {
    if (!IsTriggerChanged(i)) continue;
    Record* record = self.NewRecord();
    record->kind =
        i < self.shipped_trigger_count_ ? kEditedTrigger : kAddedTrigger;
    record->map = self.map_;
    record->index = static_cast<u16>(i);
    record->trigger = self.triggers_[i];
  }

  if (GetRecordCount() == 0) {
    sys::File::Delete(kPath);
    return true;
  }
  sys::File::CreateDirectory(kFolder);
  sys::File file(kPath, true);
  if (!file.IsOpen()) return false;
  static const c8 kHeader[] =
      "# Sango map events. map S+|S=|T+|T= index fields. See the docs.\n";
  file.Write(kHeader, sizeof(kHeader) - 1);
  for (const Record& record : self.records_) {
    if (!record.is_used) continue;
    c8 line[kLineSize];
    int length;
    const bool is_sign =
        record.kind == kAddedSign || record.kind == kEditedSign;
    const c8 mode =
        (record.kind == kAddedSign || record.kind == kAddedTrigger) ? '+' : '=';
    if (is_sign) {
      const SignEvent& s = record.sign;
      length = snprintf(line, sizeof(line), "%u S%c %u %d %d %u %u %d %u %d %u\n",
                        (unsigned)record.map, mode, (unsigned)record.index,
                        (int)s.tiles.tile_x, (int)s.tiles.tile_z,
                        (unsigned)s.tiles.width, (unsigned)s.tiles.depth,
                        (int)s.tiles.height, (unsigned)s.id,
                        (int)static_cast<u8>(s.talk_facing), (unsigned)s.kind);
    } else {
      const TriggerEvent& t = record.trigger;
      length = snprintf(line, sizeof(line),
                        "%u T%c %u %u %u %u %u %d %u %u %u %u\n",
                        (unsigned)record.map, mode, (unsigned)record.index,
                        (unsigned)t.tiles.tile_x, (unsigned)t.tiles.tile_z,
                        (unsigned)t.tiles.width, (unsigned)t.tiles.depth,
                        (int)t.tiles.height, (unsigned)t.id,
                        (unsigned)t.work_id, (unsigned)t.param,
                        (unsigned)t.check_kind);
    }
    if (length > 0) file.Write(line, static_cast<u32>(length));
  }
  return true;
}

void TriggerEditor::ResetMap() {
  auto& self = GetInstance();
  if (!IsActive()) return;
  for (Record& record : self.records_) {
    if (record.is_used && record.map == self.map_) record.is_used = false;
  }
  memset(self.signs_, 0, sizeof(self.signs_));
  memset(self.triggers_, 0, sizeof(self.triggers_));
  memcpy(self.signs_, self.shipped_signs_,
         self.shipped_sign_count_ * sizeof(SignEvent));
  memcpy(self.triggers_, self.shipped_triggers_,
         self.shipped_trigger_count_ * sizeof(TriggerEvent));
  self.sign_count_ = self.shipped_sign_count_;
  self.trigger_count_ = self.shipped_trigger_count_;
  self.events_->sign_count = self.sign_count_;
  self.events_->trigger_count = self.trigger_count_;
}

} // namespace overworld
