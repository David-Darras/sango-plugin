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
 * @file trainer_class.cc
 * @brief Changes the class of a trainer.
 *
 * The declarations are in battle/patch/trainer_class.h.
 */

#include "battle/patch/trainer_class.h"

#include <cstring>

#include "battle/constant/trainer_class_records.h"
#include "core/translations.h"
#include "system/native/string.h"
#include "ui/log.h"

namespace battle {

namespace {
// The place of the class (a 16-bit number) in the data of a trainer.
constexpr u32 kClassOffset = 2;
// The size of the data of a trainer, at least.
constexpr u32 kMinimumSize = 16;
// The places of the values in the data of a class: the model sequence, the sex
// and the plural (one byte), the group, the battle effect, the 3D model, the
// 2D picture, the character on the map and the music of the sight.
constexpr u32 kSequenceOffset = 0;
constexpr u32 kEffectOffset = 2;
constexpr u32 kModelOffset = 3;
constexpr u32 kPictureOffset = 4;
constexpr u32 kOverworldOffset = 8;
constexpr u32 kMusicOffset = 12;
// The half of the byte of the sequence that is the model sequence.
constexpr u8 kSequenceMask = 0x0F;

void CopyBytes(u8* to, const u8* from, u32 offset, u32 count) {
  for (u32 i = 0; i < count; i++) to[offset + i] = from[offset + i];
}
} // namespace

bool TrainerClasses::Set(TrainerId trainer, TrainerClassId trainer_class) {
  auto& self = GetInstance();
  Entry* same = self.entries_.Find(
      [trainer](const Entry& entry) { return entry.trainer == trainer; });
  if (same != nullptr) {
    same->trainer_class = trainer_class;
    return true;
  }
  return self.entries_.Add({trainer, trainer_class});
}

void TrainerClasses::Remove(TrainerId trainer) {
  GetInstance().entries_.RemoveIf(
      [trainer](const Entry& entry) { return entry.trainer == trainer; });
}

void TrainerClasses::Clear() { GetInstance().entries_.Clear(); }

TrainerClasses::Definition* TrainerClasses::FindDefinition(TrainerClassId id) {
  return GetInstance().definitions_.Find(
      [id](const Definition& item) { return item.id == id; });
}

TrainerClassId TrainerClasses::Define(const c16* title, TrainerClassId from) {
  auto& self = GetInstance();
  const u32 source = static_cast<u32>(from);
  if (self.free_count_ >= kFreeTrainerClassCount ||
      source >= static_cast<u32>(TrainerClassId::kCount) ||
      self.definitions_.IsFull()) {
    return TrainerClassId::kCount;
  }
  Definition definition = {};
  definition.id = static_cast<TrainerClassId>(
      kFreeTrainerClasses[self.free_count_++]);
  std::memcpy(definition.record, kTrainerClassRecords[source],
              sizeof(definition.record));
  SetTitleText(definition.title, title);
  self.definitions_.Add(definition);
  return definition.id;
}

void TrainerClasses::SetTitleText(c16* to, const c16* title) {
  u32 i = 0;
  for (; title != nullptr && title[i] != 0 && i < kMaxTitleLength; i++) {
    to[i] = title[i];
  }
  to[i] = 0;
}

void TrainerClasses::SetTitle(TrainerClassId id, const c16* title) {
  Definition* definition = FindDefinition(id);
  if (definition != nullptr) SetTitleText(definition->title, title);
}

void TrainerClasses::SetOverworld(TrainerClassId id, u32 model) {
  Definition* definition = FindDefinition(id);
  if (definition == nullptr) return;
  for (u32 i = 0; i < 4; i++) {
    definition->record[kOverworldOffset + i] =
        static_cast<u8>(model >> (8 * i));
  }
}

void TrainerClasses::CopyPicture(TrainerClassId id, TrainerClassId from) {
  Definition* definition = FindDefinition(id);
  if (definition == nullptr ||
      static_cast<u32>(from) >= static_cast<u32>(TrainerClassId::kCount)) {
    return;
  }
  CopyBytes(definition->record, kTrainerClassRecords[static_cast<u32>(from)],
            kPictureOffset, 4);
}

void TrainerClasses::CopyIntroduction(TrainerClassId id, TrainerClassId from) {
  Definition* definition = FindDefinition(id);
  if (definition == nullptr ||
      static_cast<u32>(from) >= static_cast<u32>(TrainerClassId::kCount)) {
    return;
  }
  const u8* source = kTrainerClassRecords[static_cast<u32>(from)];
  definition->record[kSequenceOffset] =
      (definition->record[kSequenceOffset] & ~kSequenceMask) |
      (source[kSequenceOffset] & kSequenceMask);
  definition->record[kEffectOffset] = source[kEffectOffset];
  definition->record[kModelOffset] = source[kModelOffset];
}

void TrainerClasses::CopyMusic(TrainerClassId id, TrainerClassId from) {
  Definition* definition = FindDefinition(id);
  if (definition == nullptr ||
      static_cast<u32>(from) >= static_cast<u32>(TrainerClassId::kCount)) {
    return;
  }
  CopyBytes(definition->record, kTrainerClassRecords[static_cast<u32>(from)],
            kMusicOffset, 4);
}

void TrainerClasses::ClearDefinitions() {
  auto& self = GetInstance();
  self.definitions_.Clear();
  self.free_count_ = 0;
}

void TrainerClasses::PatchRecord(u32 file_id, void* data, u32 size) {
  if (data == nullptr || size < kTrainerClassRecordSize) return;
  const Definition* definition =
      FindDefinition(static_cast<TrainerClassId>(file_id));
  if (definition == nullptr) return;
  std::memcpy(data, definition->record, kTrainerClassRecordSize);
}

bool TrainerClasses::PatchTitle(u32 class_id, sys::String* output) {
  const Definition* definition =
      FindDefinition(static_cast<TrainerClassId>(class_id));
  if (definition == nullptr || definition->title[0] == 0) return false;
  output->Set(core::Translate(definition->title));
  return true;
}

void TrainerClasses::Patch(u32 file_id, void* data, u32 size) {
  auto& self = GetInstance();
  if (data == nullptr || size < kMinimumSize) return;
  const Entry* entry = self.entries_.Find([file_id](const Entry& item) {
    return static_cast<u32>(item.trainer) == file_id;
  });
  if (entry == nullptr) return;
  u8* bytes = static_cast<u8*>(data);
  const u16 value = static_cast<u16>(entry->trainer_class);
  bytes[kClassOffset] = static_cast<u8>(value & 0xFF);
  bytes[kClassOffset + 1] = static_cast<u8>(value >> 8);
  ui::Log::Print(u"trainer %u: class %u", file_id, static_cast<u32>(value));
}

} // namespace battle
