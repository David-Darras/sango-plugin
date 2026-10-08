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
 * @file move_animation.cc
 * @brief Changes the move animations, or makes new ones.
 *
 * The declarations are in battle/patch/move_animation.h.
 */

#include "battle/patch/move_animation.h"
#include <cstring>
#include "battle/patch/effect_style.h"
#include "core/hook_manager.h"
#include "ui/log_application.h"

namespace battle {

namespace {

constexpr bool kSupported = GAME_CONSTANT(false, true);
constexpr u32 kFirstMoveFile = GAME_CONSTANT(116, 119);
constexpr u32 kScratchFile = GAME_CONSTANT(0, 119);
constexpr u32 kFileMagic = 0x44534553;

constexpr u32 kFileHeaderSize = 16;
constexpr u32 kVersionOffset = 4;
constexpr u32 kMaxFrameOffset = 12;
constexpr u32 kSupportedVersion = 2;
constexpr s32 kEndMarker = -1;
constexpr u32 kEndMarkerSize = 4;

constexpr u32 kStartFrameOffset = 0;
constexpr u32 kEndFrameOffset = 4;
constexpr u32 kGroupOffset = 8;
constexpr u32 kOptionOffset = 10;
constexpr u32 kKindOffset = 12;
constexpr u32 kStepHeaderSize = 14;

constexpr u32 kSlotCount = 2;
constexpr u32 kSlotCapacity = 0x24000;

constexpr u32 kFamilyGroupLimits[] = {0xFFFF, 10, 10, 5, 5, 6};

const u8 kStepFamilies[kAnimationStepKindCount] = {
#include "move_animation_families.inc"
};

bool IsSpawnKind(u16 kind) {
  return kind == (u16)AnimationStepKind::kEffectSpawn ||
         kind == (u16)AnimationStepKind::kEffectSpawnForContest ||
         kind == (u16)AnimationStepKind::kModelSpawn ||
         kind == (u16)AnimationStepKind::kDebrisSpawn ||
         kind == (u16)AnimationStepKind::kOverlayCreate;
}

const u8 kValueBytes[kAnimationStepKindCount] = {
  0, 20, 16, 32, 4, 24, 20, 12, 16, 8, 4, 8, 8, 8, 8, 8, 36, 52, 4, 24, 0,
  36, 36, 48, 8, 56, 12, 4, 16, 8, 16, 24, 20, 28, 4, 8, 8, 4, 4, 4, 12, 8,
  0, 12, 0, 16, 12, 24, 28, 16, 36, 20, 12, 16, 16, 20, 48, 24, 12, 4, 0, 20,
  0, 24, 4, 16, 12, 24, 28, 16, 36, 16, 16, 12, 4, 20, 48, 24, 4, 52, 12, 8,
  8, 8, 8, 4, 16, 4, 0, 20, 8, 0, 0, 28, 8, 8, 8, 8, 4, 24, 24, 4, 16, 16,
  24, 20, 20, 16, 12, 0, 8, 4, 8, 4, 8, 8, 0, 20, 16, 4, 8, 8, 0, 76, 16, 12,
  24, 16, 16, 12, 24, 24, 24, 24, 24, 80, 80, 8, 8, 20, 0, 48, 4, 8, 8, 4, 4,
  4, 0, 4, 0, 16, 16, 8, 8, 12, 8, 4, 12, 0, 20, 0, 4, 4, 8, 8, 28, 28, 24,
  48, 8, 0, 28, 4, 20, 36, 20, 28, 44, 24, 24, 4, 52, 8, 8, 8, 0, 48, 0, 8,
  32, 28, 8, 0, 4, 0, 0, 16, 4, 4, 16, 4, 12, 96, 0, 36, 40, 12, 12, 0, 4, 0,
  0,
};

template <typename T>
T Read(const u8* data, u32 offset) {
  T value;
  std::memcpy(&value, data + offset, sizeof(T));
  return value;
}

template <typename T>
void Write(u8* data, u32 offset, T value) {
  std::memcpy(data + offset, &value, sizeof(T));
}

u32 StepLength(const u8* data, u32 offset) {
  const u16 kind = Read<u16>(data, offset + kKindOffset);
  return kStepHeaderSize + kValueBytes[kind];
}

struct Slot {
  alignas(4) u8 data[kSlotCapacity];
  u32 file_id;
  u32 owner;
  u32 size;
  bool ready;
};

Slot slots[kSlotCount];
u32 next_slot = 0;

void ResetSlots() {
  for (u32 i = 0; i < kSlotCount; i++) slots[i].ready = false;
}

} // namespace

u32 MoveAnimation::WriteEmpty(u8* buffer) {
  Write<u32>(buffer, 0, kFileMagic);
  Write<s32>(buffer, kVersionOffset, (s32)kSupportedVersion);
  Write<s32>(buffer, 8, 0);
  Write<s32>(buffer, kMaxFrameOffset, 0);
  Write<s32>(buffer, kFileHeaderSize, kEndMarker);
  return kFileHeaderSize + kEndMarkerSize;
}

void MoveAnimation::SetDuration(s32 frames) {
  WriteMaxFrame(frames);
}

bool MoveAnimation::Validate(const u8* data, u32 size, u32* used_size) {
  if (size < kFileHeaderSize + kEndMarkerSize) return false;
  if (Read<s32>(data, kVersionOffset) != (s32)kSupportedVersion) return false;

  u32 offset = kFileHeaderSize;
  while (true) {
    if (offset + kEndMarkerSize > size) return false;
    if (Read<s32>(data, offset) == kEndMarker) break;
    if (offset + kStepHeaderSize > size) return false;
    const u16 kind = Read<u16>(data, offset + kKindOffset);
    if (kind >= kAnimationStepKindCount) return false;
    offset += kStepHeaderSize + kValueBytes[kind];
  }
  if (used_size != nullptr) *used_size = offset + kEndMarkerSize;
  return true;
}

MoveAnimation::LimitProblem MoveAnimation::FindLimitProblem(
    u32* step_index, u32* limit) const {
  u32 offset = kFileHeaderSize;
  u32 next = 0;
  u32 index = 0;
  while (NextOffset(offset, &next)) {
    const u16 kind = Read<u16>(data_, offset + kKindOffset);
    const u16 group = Read<u16>(data_, offset + kGroupOffset);
    const u16 option = Read<u16>(data_, offset + kOptionOffset);
    const u8 family = kStepFamilies[kind];
    if (family != 0 && group >= kFamilyGroupLimits[family]) {
      *step_index = index;
      *limit = kFamilyGroupLimits[family];
      return LimitProblem::kGroupOutOfRange;
    }
    if (IsSpawnKind(kind) && option == 0) {
      const s32 start = Read<s32>(data_, offset + kStartFrameOffset);
      const s32 end = Read<s32>(data_, offset + kEndFrameOffset);
      u32 earlier = kFileHeaderSize;
      u32 earlier_next = 0;
      while (earlier < offset && NextOffset(earlier, &earlier_next)) {
        const u16 other_kind = Read<u16>(data_, earlier + kKindOffset);
        if (kStepFamilies[other_kind] == family && IsSpawnKind(other_kind) &&
            Read<u16>(data_, earlier + kGroupOffset) == group &&
            Read<u16>(data_, earlier + kOptionOffset) == 0 &&
            start < Read<s32>(data_, earlier + kEndFrameOffset) &&
            Read<s32>(data_, earlier + kStartFrameOffset) < end) {
          *step_index = index;
          *limit = group;
          return LimitProblem::kGroupStillAlive;
        }
        earlier = earlier_next;
      }
    }
    offset = next;
    index++;
  }
  return LimitProblem::kNone;
}

bool MoveAnimation::NextOffset(u32 offset, u32* next) const {
  if (offset + kEndMarkerSize > size_) return false;
  if (Read<s32>(data_, offset) == kEndMarker) return false;
  *next = offset + StepLength(data_, offset);
  return true;
}

u32 MoveAnimation::step_count() const {
  u32 count = 0;
  u32 offset = kFileHeaderSize;
  while (NextOffset(offset, &offset)) count++;
  return count;
}

s32 MoveAnimation::last_frame() const {
  return Read<s32>(data_, kMaxFrameOffset);
}

void MoveAnimation::WriteMaxFrame(s32 frame) {
  Write<s32>(data_, kMaxFrameOffset, frame);
}

AnimationStep MoveAnimation::StepAt(u32 index) {
  u32 offset = kFileHeaderSize;
  u32 next = 0;
  while (NextOffset(offset, &next)) {
    if (index == 0) return AnimationStep(this, offset);
    index--;
    offset = next;
  }
  return AnimationStep();
}

AnimationStep MoveAnimation::First(AnimationStepKind kind, u32 skip) {
  u32 offset = kFileHeaderSize;
  u32 next = 0;
  while (NextOffset(offset, &next)) {
    if (Read<u16>(data_, offset + kKindOffset) == (u16)kind) {
      if (skip == 0) return AnimationStep(this, offset);
      skip--;
    }
    offset = next;
  }
  return AnimationStep();
}

u32 MoveAnimation::Count(AnimationStepKind kind) const {
  u32 count = 0;
  u32 offset = kFileHeaderSize;
  u32 next = 0;
  while (NextOffset(offset, &next)) {
    if (Read<u16>(data_, offset + kKindOffset) == (u16)kind) count++;
    offset = next;
  }
  return count;
}

void MoveAnimation::ForEach(AnimationStepKind kind,
                            const StepVisitor& visitor) {
  u32 offset = kFileHeaderSize;
  u32 next = 0;
  while (NextOffset(offset, &next)) {
    if (Read<u16>(data_, offset + kKindOffset) == (u16)kind) {
      AnimationStep step(this, offset);
      visitor(step);
    }
    offset = next;
  }
}

u32 MoveAnimation::InsertionOffset(s32 start_frame) const {
  u32 offset = kFileHeaderSize;
  u32 next = 0;
  while (NextOffset(offset, &next)) {
    if (Read<s32>(data_, offset + kStartFrameOffset) > start_frame) {
      return offset;
    }
    offset = next;
  }
  return offset;
}

AnimationStep MoveAnimation::Add(AnimationStepKind kind, s32 start_frame,
                                 s32 end_frame, u16 group,
                                 StepCondition condition) {
  if ((u32)kind >= kAnimationStepKindCount) return AnimationStep();
  if (end_frame < start_frame) end_frame = start_frame;

  const u32 length = kStepHeaderSize + kValueBytes[(u32)kind];
  if (size_ + length > capacity_) return AnimationStep();

  const u32 offset = InsertionOffset(start_frame);
  std::memmove(data_ + offset + length, data_ + offset, size_ - offset);
  size_ += length;

  std::memset(data_ + offset, 0, length);
  Write<s32>(data_, offset + kStartFrameOffset, start_frame);
  Write<s32>(data_, offset + kEndFrameOffset, end_frame);
  Write<u16>(data_, offset + kGroupOffset, group);
  Write<u16>(data_, offset + kOptionOffset, (u16)condition);
  Write<u16>(data_, offset + kKindOffset, (u16)kind);

  if (end_frame > last_frame()) WriteMaxFrame(end_frame);
  return AnimationStep(this, offset);
}

bool MoveAnimation::Remove(const AnimationStep& step) {
  if (!step.IsValid() || step.animation_ != this) return false;
  const u32 length = StepLength(data_, step.offset_);
  if (step.offset_ + length > size_) return false;
  std::memmove(data_ + step.offset_, data_ + step.offset_ + length,
               size_ - step.offset_ - length);
  size_ -= length;
  return true;
}

u32 MoveAnimation::RemoveAll(AnimationStepKind kind) {
  u32 removed = 0;
  AnimationStep step = First(kind);
  while (step.IsValid()) {
    Remove(step);
    removed++;
    step = First(kind);
  }
  return removed;
}

void MoveAnimation::ScaleEffects(f32 factor) {
  ScaleEffects(Vec3(factor, factor, factor));
}

void MoveAnimation::ScaleEffects(const Vec3& factor) {
  ForEach(AnimationStepKind::kEffectResize, [&factor](AnimationStep& step) {
    const Vec3 value = step.GetVec3(0);
    step.SetVec3(0, Vec3(value.x * factor.x, value.y * factor.y,
                         value.z * factor.z));
  });
}

void MoveAnimation::TurnEffects(const Vec3& degrees) {
  ForEach(AnimationStepKind::kEffectTurn, [&degrees](AnimationStep& step) {
    const Vec3 value = step.GetVec3(0);
    step.SetVec3(0, Vec3(value.x + degrees.x, value.y + degrees.y,
                         value.z + degrees.z));
  });
}

void MoveAnimation::OffsetEffects(const Vec3& offset) {
  ForEach(AnimationStepKind::kEffectGlideToPokemon,
          [&offset](AnimationStep& step) {
    const Vec3 value = step.GetVec3(2);
    step.SetVec3(2, Vec3(value.x + offset.x, value.y + offset.y,
                         value.z + offset.z));
  });
}

void MoveAnimation::ReplaceEffect(EffectId from, EffectId to) {
  ForEach(AnimationStepKind::kEffectSpawn, [from, to](AnimationStep& step) {
    if (step.GetS32(0) == static_cast<s32>(from)) {
      step.SetS32(0, static_cast<s32>(to));
    }
  });
}

void MoveAnimation::TintEffect(EffectId effect,
                                 const Color8& multiplier, EffectLayer layer) {
  EffectStyles::Tint((u32)effect, multiplier, (u32)layer);
}

void MoveAnimation::TintEffects(const Color8& multiplier) {
  ForEach(AnimationStepKind::kEffectSpawn,
          [&multiplier](AnimationStep& step) {
    EffectStyles::Tint((u32)step.GetS32(0), multiplier);
  });
}

void MoveAnimation::ColorEffect(EffectId effect, const Color8& color,
                                  EffectLayer layer) {
  EffectStyles::Recolor((u32)effect, color, color, color, (u32)layer);
}

void MoveAnimation::ColorEffects(const Color8& color) {
  ForEach(AnimationStepKind::kEffectSpawn, [&color](AnimationStep& step) {
    EffectStyles::Recolor((u32)step.GetS32(0), color, color, color);
  });
}

void MoveAnimation::RecolorEffect(EffectId effect, const Color8& start,
                                    const Color8& middle, const Color8& end,
                                    EffectLayer layer) {
  EffectStyles::Recolor((u32)effect, start, middle, end, (u32)layer);
}

void MoveAnimation::RecolorEffects(const Color8& start, const Color8& middle,
                                     const Color8& end) {
  ForEach(AnimationStepKind::kEffectSpawn,
          [&start, &middle, &end](AnimationStep& step) {
    EffectStyles::Recolor((u32)step.GetS32(0), start, middle, end);
  });
}

void MoveAnimation::ScaleEffectLife(EffectId effect, f32 factor,
                                      EffectLayer layer) {
  EffectStyles::ScaleLife((u32)effect, factor, (u32)layer);
}

void MoveAnimation::ScaleEffectDensity(EffectId effect, f32 factor,
                                         EffectLayer layer) {
  EffectStyles::ScaleDensity((u32)effect, factor, (u32)layer);
}

void MoveAnimation::ScaleEffectSize(EffectId effect, f32 factor,
                                      EffectLayer layer) {
  EffectStyles::ScaleSize((u32)effect, factor, (u32)layer);
}

void MoveAnimation::FadeEffect(EffectId effect, f32 fade_in_end,
                                 f32 fade_out_start, f32 peak,
                                 EffectLayer layer) {
  EffectStyles::Fade((u32)effect, fade_in_end, fade_out_start, peak,
                       (u32)layer);
}

void MoveAnimation::LoopEffect(EffectId effect, EffectLayer layer) {
  EffectStyles::Loop((u32)effect, (u32)layer);
}

void MoveAnimation::DisableEffect(EffectId effect, EffectLayer layer) {
  EffectStyles::Disable((u32)effect, (u32)layer);
}

void MoveAnimation::LoopEffects() {
  ForEach(AnimationStepKind::kEffectSpawn, [](AnimationStep& step) {
    EffectStyles::Loop((u32)step.GetS32(0), EffectStyles::kAllLayers);
  });
}

void MoveAnimation::ScaleEffectsLife(f32 factor) {
  ForEach(AnimationStepKind::kEffectSpawn, [factor](AnimationStep& step) {
    EffectStyles::ScaleLife((u32)step.GetS32(0), factor);
  });
}

void MoveAnimation::ScaleEffectsDensity(f32 factor) {
  ForEach(AnimationStepKind::kEffectSpawn, [factor](AnimationStep& step) {
    EffectStyles::ScaleDensity((u32)step.GetS32(0), factor);
  });
}

void MoveAnimation::ScaleEffectsSize(f32 factor) {
  ForEach(AnimationStepKind::kEffectSpawn, [factor](AnimationStep& step) {
    EffectStyles::ScaleSize((u32)step.GetS32(0), factor);
  });
}

void MoveAnimation::FadeEffects(f32 fade_in_end, f32 fade_out_start,
                                  f32 peak) {
  ForEach(AnimationStepKind::kEffectSpawn,
          [fade_in_end, fade_out_start, peak](AnimationStep& step) {
    EffectStyles::Fade((u32)step.GetS32(0), fade_in_end, fade_out_start,
                         peak);
  });
}

void MoveAnimation::ScaleModels(f32 factor) {
  ForEach(AnimationStepKind::kModelResize, [factor](AnimationStep& step) {
    const Vec3 value = step.GetVec3(0);
    step.SetVec3(0, Vec3(value.x * factor, value.y * factor,
                         value.z * factor));
  });
}

AnimationStepKind AnimationStep::kind() const {
  return (AnimationStepKind)Read<u16>(animation_->data_, offset_ + kKindOffset);
}

s32 AnimationStep::start_frame() const {
  return Read<s32>(animation_->data_, offset_ + kStartFrameOffset);
}

s32 AnimationStep::end_frame() const {
  return Read<s32>(animation_->data_, offset_ + kEndFrameOffset);
}

u16 AnimationStep::group() const {
  return Read<u16>(animation_->data_, offset_ + kGroupOffset);
}

u32 AnimationStep::value_count() const {
  return kValueBytes[(u32)kind()] / 4;
}

void AnimationStep::SetStartFrame(s32 frame) {
  Write<s32>(animation_->data_, offset_ + kStartFrameOffset, frame);
}

void AnimationStep::SetEndFrame(s32 frame) {
  Write<s32>(animation_->data_, offset_ + kEndFrameOffset, frame);
  if (frame > animation_->last_frame()) animation_->WriteMaxFrame(frame);
}

void AnimationStep::SetGroup(u16 group) {
  Write<u16>(animation_->data_, offset_ + kGroupOffset, group);
}

u8* AnimationStep::Values() const {
  return animation_->data_ + offset_ + kStepHeaderSize;
}

bool AnimationStep::HasValue(u32 index, u32 width) const {
  return (index * 4 + width) <= kValueBytes[(u32)kind()];
}

s32 AnimationStep::GetS32(u32 index) const {
  return HasValue(index, 4) ? Read<s32>(Values(), index * 4) : 0;
}

f32 AnimationStep::GetF32(u32 index) const {
  return HasValue(index, 4) ? Read<f32>(Values(), index * 4) : 0.0f;
}

Vec3 AnimationStep::GetVec3(u32 index) const {
  if (!HasValue(index, 12)) return Vec3();
  return Vec3(Read<f32>(Values(), index * 4),
              Read<f32>(Values(), index * 4 + 4),
              Read<f32>(Values(), index * 4 + 8));
}

void AnimationStep::SetS32(u32 index, s32 value) {
  if (HasValue(index, 4)) Write<s32>(Values(), index * 4, value);
}

void AnimationStep::SetF32(u32 index, f32 value) {
  if (HasValue(index, 4)) Write<f32>(Values(), index * 4, value);
}

void AnimationStep::SetText(u32 index, const char* text, u32 length) {
  if (!HasValue(index, length)) return;
  u8* target = Values() + index * 4;
  std::memset(target, 0, length);
  for (u32 i = 0; i + 1 < length && text != nullptr && text[i] != 0; i++) {
    target[i] = (u8)text[i];
  }
}

void AnimationStep::SetVec3(u32 index, const Vec3& value) {
  if (!HasValue(index, 12)) return;
  Write<f32>(Values(), index * 4, value.x);
  Write<f32>(Values(), index * 4 + 4, value.y);
  Write<f32>(Values(), index * 4 + 8, value.z);
}

u32 MoveAnimations::FileOf(pokemon::MoveId move) {
  return kFirstMoveFile + (u32)move;
}

bool MoveAnimations::Edit(pokemon::MoveId move, Editor editor) {
  if (move == pokemon::MoveId::kNone) return false;
  return EditFile(FileOf(move), editor);
}

bool MoveAnimations::EditFile(u32 file_id, Editor editor) {
  auto& ctx = GetInstance();
  if (!kSupported || editor == nullptr) return false;
  if (ctx.editor_count_ >= kMaxEditors) return false;
  ctx.editors_[ctx.editor_count_].file_id = file_id;
  ctx.editors_[ctx.editor_count_].editor = editor;
  ctx.editor_count_++;
  ResetSlots();
  return true;
}

bool MoveAnimations::Define(pokemon::MoveId move, Editor build) {
  auto& ctx = GetInstance();
  if (!kSupported || build == nullptr || move == pokemon::MoveId::kNone) {
    return false;
  }
  for (u32 i = 0; i < ctx.definition_count_; i++) {
    if (ctx.definitions_[i].move == move) {
      ctx.definitions_[i].build = build;
      ResetSlots();
      return true;
    }
  }
  if (ctx.definition_count_ >= kMaxDefinitions) return false;
  ctx.definitions_[ctx.definition_count_].move = move;
  ctx.definitions_[ctx.definition_count_].build = build;
  ctx.definition_count_++;
  ResetSlots();
  return true;
}

void MoveAnimations::Clear() {
  auto& ctx = GetInstance();
  ctx.editor_count_ = 0;
  ctx.definition_count_ = 0;
  ctx.active_definition_ = kMaxDefinitions;
  ResetSlots();
}

bool MoveAnimations::Resolve(pokemon::MoveId move, u32* id, bool* is_move) {
  auto& ctx = GetInstance();
  ctx.active_definition_ = kMaxDefinitions;
  if (!kSupported) return false;
  for (u32 i = 0; i < ctx.definition_count_; i++) {
    if (ctx.definitions_[i].move == move) {
      ctx.active_definition_ = i;
      ResetSlots();
      *id = kScratchFile;
      *is_move = false;
      return true;
    }
  }
  return false;
}

void MoveAnimations::LoadEffectsOnDemand(bool enabled) {
  GetInstance().effects_on_demand_ = enabled;
}

bool MoveAnimations::AreEffectsOnDemand() {
  return GetInstance().effects_on_demand_;
}

bool MoveAnimations::HasEditor(u32 file_id) {
  auto& ctx = GetInstance();
  if (ScratchOwner(file_id) != kMaxDefinitions) return true;
  for (u32 i = 0; i < ctx.editor_count_; i++) {
    if (ctx.editors_[i].file_id == file_id) return true;
  }
  return false;
}

u32 MoveAnimations::ScratchOwner(u32 file_id) {
  auto& ctx = GetInstance();
  if (!kSupported || file_id != kScratchFile) return kMaxDefinitions;
  return ctx.active_definition_;
}

void MoveAnimations::ApplyEditors(u32 file_id, MoveAnimation& animation) {
  auto& ctx = GetInstance();
  for (u32 i = 0; i < ctx.editor_count_; i++) {
    if (ctx.editors_[i].file_id == file_id) {
      ctx.editors_[i].editor(animation);
    }
  }
}

void MoveAnimations::ApplyDefinition(u32 index, MoveAnimation& animation) {
  auto& ctx = GetInstance();
  if (index < ctx.definition_count_) ctx.definitions_[index].build(animation);
}

static Slot* PrepareSlot(u32* archive, u32 file_id) {
  const u32 owner = MoveAnimations::ScratchOwner(file_id);
  for (u32 i = 0; i < kSlotCount; i++) {
    if (slots[i].ready && slots[i].file_id == file_id &&
        slots[i].owner == owner) {
      return &slots[i];
    }
  }

  Slot& slot = slots[next_slot];
  next_slot = (next_slot + 1) % kSlotCount;
  slot.ready = false;

  u32 used = 0;
  if (owner != MoveAnimations::kMaxDefinitions) {
    used = MoveAnimation::WriteEmpty(slot.data);
  } else {
    const u32 original =
        core::HookManager::Call<u32>(HookId::kArchiveGetFileSize, archive,
                                     file_id);
    if (original > kSlotCapacity) return nullptr;

    core::HookManager::Call<u32>(HookId::kArchiveLoadData2, archive, file_id,
                                 (void*)slot.data);

    if (!MoveAnimation::Validate(slot.data, original, &used)) return nullptr;
  }

  MoveAnimation animation(slot.data, used, kSlotCapacity);
  if (owner != MoveAnimations::kMaxDefinitions) {
    MoveAnimations::ApplyDefinition(owner, animation);
  } else {
    MoveAnimations::ApplyEditors(file_id, animation);
  }

  u32 final_size = 0;
  if (!MoveAnimation::Validate(slot.data, animation.size(), &final_size) ||
      final_size != animation.size()) {
    return nullptr;
  }

  u32 problem_step = 0;
  u32 problem_limit = 0;
  const MoveAnimation::LimitProblem problem =
      animation.FindLimitProblem(&problem_step, &problem_limit);
  if (problem == MoveAnimation::LimitProblem::kGroupOutOfRange) {
    ui::LogApplication::Print(u"Animation %d: step %d uses a group >= %d",
                              (s32)file_id, (s32)problem_step,
                              (s32)problem_limit);
    return nullptr;
  }
  if (problem == MoveAnimation::LimitProblem::kGroupStillAlive) {
    ui::LogApplication::Print(u"Animation %d: step %d reuses group %d too early",
                              (s32)file_id, (s32)problem_step,
                              (s32)problem_limit);
    return nullptr;
  }

  slot.file_id = file_id;
  slot.owner = owner;
  slot.size = final_size;
  slot.ready = true;
  return &slot;
}

u32 MoveAnimations::PatchedSize(u32* archive, u32 file_id) {
  if (!HasEditor(file_id)) return 0;
  Slot* slot = PrepareSlot(archive, file_id);
  return slot != nullptr ? slot->size : 0;
}

bool MoveAnimations::Serve(u32* archive, u32 file_id, void* buffer,
                           u32* size) {
  if (!HasEditor(file_id)) return false;
  Slot* slot = PrepareSlot(archive, file_id);
  if (slot == nullptr) return false;
  std::memcpy(buffer, slot->data, slot->size);
  if (size != nullptr) *size = slot->size;
  return true;
}

}
