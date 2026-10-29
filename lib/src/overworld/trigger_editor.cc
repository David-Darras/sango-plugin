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
 * @brief Adds and changes the signs, the triggers, the warps and the
 *        characters of the map.
 *
 * The declarations are in overworld/patch/trigger_editor.h.
 */

#include "overworld/patch/trigger_editor.h"

#include <cstdio>
#include <cstring>

#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/native/world_layout.h"
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

void TriggerEditor::CopyCharacterFields(CharacterPlacement& to,
                                        const CharacterPlacement& from) {
  to.position.tile_x = from.position.tile_x;
  to.position.tile_z = from.position.tile_z;
  to.position.height = from.position.height;
  to.facing = from.facing;
  to.script_id = from.script_id;
  to.movement_id = from.movement_id;
  to.interaction_kind = from.interaction_kind;
  to.param0 = from.param0;
  to.param1 = from.param1;
  to.param2 = from.param2;
  to.hide_when_flag_set = from.hide_when_flag_set;
  to.model_id = from.model_id;
  to.trigger_x = from.trigger_x;
  to.trigger_z = from.trigger_z;
  to.trigger_width = from.trigger_width;
  to.trigger_depth = from.trigger_depth;
}

bool TriggerEditor::LoadLine(const c8* line) {
  const c8* p = line;
  s32 map;
  s32 index;
  if (!ReadInt(p, map)) return false;
  while (*p == ' ') p++;
  const c8 type = p[0];
  const c8 mode = p[1];
  const bool is_known =
      ((type == 'S' || type == 'T' || type == 'W') &&
       (mode == '+' || mode == '=')) ||
      (type == 'C' && mode == '=');
  if (!is_known) return false;
  p += 2;
  if (!ReadInt(p, index)) return false;
  s32 f[16] = {};
  const u32 count = type == 'S' ? 8 : (type == 'C' ? 16 : 9);
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
  } else if (type == 'T') {
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
  } else if (type == 'W') {
    record->kind = mode == '+' ? kAddedWarp : kEditedWarp;
    WarpEvent& warp = record->warp;
    warp.world.x = static_cast<s16>(f[0]);
    warp.world.y = static_cast<s16>(f[1]);
    warp.world.z = static_cast<s16>(f[2]);
    warp.world.width = static_cast<u16>(f[3]);
    warp.world.depth = static_cast<u16>(f[4]);
    warp.map_id = static_cast<MapId>(f[5]);
    warp.exit_id = static_cast<u16>(f[6]);
    warp.exit_facing = static_cast<Facing>(f[7]);
    warp.exit_kind = static_cast<u8>(f[8]);
    warp.position_kind = 0;
  } else {
    record->kind = kEditedCharacter;
    CharacterPlacement& c = record->character;
    c.position.tile_x = static_cast<u16>(f[0]);
    c.position.tile_z = static_cast<u16>(f[1]);
    c.position.height = static_cast<f32>(f[2]);
    c.facing = static_cast<Facing>(f[3]);
    c.script_id = static_cast<script::ScriptId>(f[4]);
    c.movement_id = static_cast<u16>(f[5]);
    c.interaction_kind = static_cast<u16>(f[6]);
    c.param0 = static_cast<u16>(f[7]);
    c.param1 = static_cast<u16>(f[8]);
    c.param2 = static_cast<u16>(f[9]);
    c.hide_when_flag_set = static_cast<u16>(f[10]);
    c.model_id = static_cast<ModelId>(f[11]);
    c.trigger_x = static_cast<s16>(f[12]);
    c.trigger_z = static_cast<s16>(f[13]);
    c.trigger_width = static_cast<s16>(f[14]);
    c.trigger_depth = static_cast<s16>(f[15]);
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
  shipped_warp_count_ = warp_count_;
  memcpy(shipped_warps_, warps_, sizeof(warps_));
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
      case kEditedWarp:
        if (record.index < shipped_warp_count_) warps_[record.index] = record.warp;
        break;
      case kAddedWarp:
        if (warp_count_ < kMaxWarps) warps_[warp_count_++] = record.warp;
        break;
      case kEditedCharacter:
        if (events_ != nullptr && events_->characters != nullptr &&
            record.index < character_count_) {
          CopyCharacterFields(events_->characters[record.index], record.character);
        }
        break;
    }
  }
}

void TriggerEditor::OnMapEventsLoaded(MapEventData* events) {
  auto& self = GetInstance();
  MapEventData* previous = self.events_;
  self.events_ = nullptr;
  if (events == nullptr) return;
  // The previous map is still loaded: it keeps a copy of its tables, because
  // the working tables now belong to the new map.
  if (previous != nullptr && previous != events &&
      previous->map_id != events->map_id && previous->signs == self.signs_ &&
      previous->triggers == self.triggers_ && previous->warps == self.warps_) {
    memcpy(self.frozen_signs_, self.signs_, sizeof(self.signs_));
    memcpy(self.frozen_triggers_, self.triggers_, sizeof(self.triggers_));
    memcpy(self.frozen_warps_, self.warps_, sizeof(self.warps_));
    previous->signs = self.frozen_signs_;
    previous->triggers = self.frozen_triggers_;
    previous->warps = self.frozen_warps_;
  }
  // A map with more events than the tables: the editor leaves it alone.
  if (events->sign_count > kMaxSigns || events->trigger_count > kMaxTriggers ||
      events->warp_count > kMaxWarps ||
      events->character_count > kMaxCharactersPerMap) {
    return;
  }
  if ((events->sign_count != 0 && events->signs == nullptr) ||
      (events->trigger_count != 0 && events->triggers == nullptr) ||
      (events->warp_count != 0 && events->warps == nullptr) ||
      (events->character_count != 0 && events->characters == nullptr)) {
    return;
  }
  memset(self.signs_, 0, sizeof(self.signs_));
  memset(self.triggers_, 0, sizeof(self.triggers_));
  memset(self.warps_, 0, sizeof(self.warps_));
  memset(self.shipped_characters_, 0, sizeof(self.shipped_characters_));
  for (u32 i = 0; i < events->sign_count; i++) self.signs_[i] = events->signs[i];
  for (u32 i = 0; i < events->trigger_count; i++) {
    self.triggers_[i] = events->triggers[i];
  }
  for (u32 i = 0; i < events->warp_count; i++) self.warps_[i] = events->warps[i];
  for (u32 i = 0; i < events->character_count; i++) {
    self.shipped_characters_[i] = events->characters[i];
  }
  self.sign_count_ = events->sign_count;
  self.trigger_count_ = events->trigger_count;
  self.warp_count_ = events->warp_count;
  self.character_count_ = events->character_count;
  self.events_ = events;
  self.map_ = static_cast<u16>(events->map_id);
  self.TakeTables();
  self.ApplyRecords();
  events->signs = self.signs_;
  events->sign_count = self.sign_count_;
  events->triggers = self.triggers_;
  events->trigger_count = self.trigger_count_;
  events->warps = self.warps_;
  events->warp_count = self.warp_count_;
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

u32 TriggerEditor::GetWarpCount() { return GetInstance().warp_count_; }
u32 TriggerEditor::GetShippedWarpCount() {
  return GetInstance().shipped_warp_count_;
}
u32 TriggerEditor::GetCharacterCount() { return GetInstance().character_count_; }

WarpEvent* TriggerEditor::GetWarp(u32 index) {
  auto& self = GetInstance();
  return index < self.warp_count_ ? &self.warps_[index] : nullptr;
}

CharacterPlacement* TriggerEditor::GetCharacter(u32 index) {
  auto& self = GetInstance();
  if (self.events_ == nullptr || self.events_->characters == nullptr ||
      index >= self.character_count_) {
    return nullptr;
  }
  return &self.events_->characters[index];
}

WarpEvent* TriggerEditor::AddWarp() {
  auto& self = GetInstance();
  if (!IsActive() || self.warp_count_ >= kMaxWarps) return nullptr;
  WarpEvent* warp = &self.warps_[self.warp_count_];
  *warp = WarpEvent();
  warp->map_id = static_cast<MapId>(self.map_);
  warp->exit_id = 0;
  warp->exit_facing = static_cast<Facing>(1);
  warp->exit_kind = 0;
  warp->position_kind = 0;
  warp->world.width = WorldLayout::kUnitsPerTile;
  warp->world.depth = WorldLayout::kUnitsPerTile;
  MoveToPlayer(warp);
  self.warp_count_++;
  self.events_->warp_count = self.warp_count_;
  return warp;
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

void TriggerEditor::MoveToPlayer(WarpEvent* warp) {
  auto& player = ModelManager::GetInstance().GetPlayer();
  const s32 unit = static_cast<s32>(WorldLayout::kUnitsPerTile);
  warp->world.x = static_cast<s16>(static_cast<s32>(player.map_pos.coords.x) * unit);
  warp->world.z = static_cast<s16>(static_cast<s32>(player.map_pos.coords.z) * unit);
  warp->world.y = static_cast<s16>(player.GetDrawModel().position.y);
}

void TriggerEditor::MoveToPlayer(CharacterPlacement* character) {
  auto& player = ModelManager::GetInstance().GetPlayer();
  character->position.tile_x =
      static_cast<u16>(static_cast<s32>(player.map_pos.coords.x));
  character->position.tile_z =
      static_cast<u16>(static_cast<s32>(player.map_pos.coords.z));
  character->position.height = player.GetDrawModel().position.y;
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

bool TriggerEditor::RemoveWarp(u32 index) {
  auto& self = GetInstance();
  if (!IsActive() || index < self.shipped_warp_count_ ||
      index >= self.warp_count_) {
    return false;
  }
  for (u32 i = index; i + 1 < self.warp_count_; i++) {
    self.warps_[i] = self.warps_[i + 1];
  }
  self.warp_count_--;
  self.events_->warp_count = self.warp_count_;
  return true;
}

bool TriggerEditor::IsWarpChanged(u32 index) {
  auto& self = GetInstance();
  if (index >= self.warp_count_) return false;
  if (index >= self.shipped_warp_count_) return true;
  return !IsSame(&self.warps_[index], &self.shipped_warps_[index],
                 sizeof(WarpEvent));
}

bool TriggerEditor::IsCharacterChanged(u32 index) {
  auto& self = GetInstance();
  if (index >= self.character_count_ || self.events_ == nullptr ||
      self.events_->characters == nullptr) {
    return false;
  }
  return !IsSame(&self.events_->characters[index], &self.shipped_characters_[index],
                 sizeof(CharacterPlacement));
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
  for (u32 i = 0; i < self.warp_count_; i++) {
    if (IsWarpChanged(i)) needed++;
  }
  for (u32 i = 0; i < self.character_count_; i++) {
    if (IsCharacterChanged(i)) needed++;
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
  for (u32 i = 0; i < self.warp_count_; i++) {
    if (!IsWarpChanged(i)) continue;
    Record* record = self.NewRecord();
    record->kind = i < self.shipped_warp_count_ ? kEditedWarp : kAddedWarp;
    record->map = self.map_;
    record->index = static_cast<u16>(i);
    record->warp = self.warps_[i];
  }
  for (u32 i = 0; i < self.character_count_; i++) {
    if (!IsCharacterChanged(i)) continue;
    Record* record = self.NewRecord();
    record->kind = kEditedCharacter;
    record->map = self.map_;
    record->index = static_cast<u16>(i);
    record->character = self.events_->characters[i];
  }

  self.has_baseline_ = false;
  if (GetRecordCount() == 0) {
    sys::File::Delete(kPath);
    return true;
  }
  sys::File::CreateDirectory(kFolder);
  sys::File file(kPath, true);
  if (!file.IsOpen()) return false;
  static const c8 kHeader[] =
      "# Sango map events. map S+|S=|T+|T=|W+|W=|C= index fields. See the docs.\n";
  file.Write(kHeader, sizeof(kHeader) - 1);
  for (const Record& record : self.records_) {
    if (!record.is_used) continue;
    c8 line[kLineSize];
    int length;
    const bool is_sign =
        record.kind == kAddedSign || record.kind == kEditedSign;
    const c8 mode = (record.kind == kAddedSign || record.kind == kAddedTrigger ||
                     record.kind == kAddedWarp) ? '+' : '=';
    if (record.kind == kAddedWarp || record.kind == kEditedWarp) {
      const WarpEvent& w = record.warp;
      length = snprintf(line, sizeof(line), "%u W%c %u %d %d %d %u %u %u %u %d %u\n",
                        (unsigned)record.map, mode, (unsigned)record.index,
                        (int)w.world.x, (int)w.world.y, (int)w.world.z,
                        (unsigned)w.world.width, (unsigned)w.world.depth,
                        (unsigned)static_cast<u16>(w.map_id), (unsigned)w.exit_id,
                        (int)static_cast<u8>(w.exit_facing), (unsigned)w.exit_kind);
    } else if (record.kind == kEditedCharacter) {
      const CharacterPlacement& c = record.character;
      length = snprintf(line, sizeof(line),
                        "%u C= %u %u %u %d %d %u %u %u %u %u %u %u %u %d %d %d %d\n",
                        (unsigned)record.map, (unsigned)record.index,
                        (unsigned)c.position.tile_x, (unsigned)c.position.tile_z,
                        (int)c.position.height, (int)static_cast<u8>(c.facing),
                        (unsigned)static_cast<u32>(c.script_id), (unsigned)c.movement_id,
                        (unsigned)c.interaction_kind, (unsigned)c.param0,
                        (unsigned)c.param1, (unsigned)c.param2,
                        (unsigned)c.hide_when_flag_set,
                        (unsigned)static_cast<u32>(c.model_id), (int)c.trigger_x,
                        (int)c.trigger_z, (int)c.trigger_width, (int)c.trigger_depth);
    } else if (is_sign) {
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

void TriggerEditor::RestoreShipped() {
  memset(signs_, 0, sizeof(signs_));
  memset(triggers_, 0, sizeof(triggers_));
  memcpy(signs_, shipped_signs_, shipped_sign_count_ * sizeof(SignEvent));
  memcpy(triggers_, shipped_triggers_,
         shipped_trigger_count_ * sizeof(TriggerEvent));
  sign_count_ = shipped_sign_count_;
  trigger_count_ = shipped_trigger_count_;
  events_->sign_count = sign_count_;
  events_->trigger_count = trigger_count_;

  memset(warps_, 0, sizeof(warps_));
  memcpy(warps_, shipped_warps_, shipped_warp_count_ * sizeof(WarpEvent));
  warp_count_ = shipped_warp_count_;
  events_->warp_count = warp_count_;
  // The characters go back to the values of the game: the map shows them
  // when it loads again.
  for (u32 i = 0; i < character_count_ && events_->characters != nullptr; i++) {
    events_->characters[i] = shipped_characters_[i];
  }
}

void TriggerEditor::ResetMap() {
  auto& self = GetInstance();
  if (!IsActive()) return;
  for (Record& record : self.records_) {
    if (record.is_used && record.map == self.map_) record.is_used = false;
  }
  self.RestoreShipped();
}

void TriggerEditor::ReloadFromFile() {
  for (Record& record : records_) record.is_used = false;
  Initialize();
  if (!IsActive()) return;
  RestoreShipped();
  ApplyRecords();
  events_->sign_count = sign_count_;
  events_->trigger_count = trigger_count_;
  events_->warp_count = warp_count_;
}

void TriggerEditor::Update() {
  auto& self = GetInstance();
  if (!self.is_live) return;
  if (++self.frame_ % 45 != 0) return;
  static u8 buffer[8192];
  const u32 size = sys::File::ReadAll(kPath, buffer, sizeof(buffer));
  u32 hash = 2166136261u ^ size;
  for (u32 i = 0; i < size; i++) {
    hash ^= buffer[i];
    hash *= 16777619u;
  }
  if (!self.has_baseline_) {
    self.file_hash_ = hash;
    self.has_baseline_ = true;
    return;
  }
  if (hash != self.file_hash_) {
    self.file_hash_ = hash;
    self.ReloadFromFile();
  }
}

} // namespace overworld
