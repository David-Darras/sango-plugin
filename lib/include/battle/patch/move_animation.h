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

#include <functional>

#include "common.h"
#include "battle/constant/animation_enums.h"
#include "battle/constant/animation_step_kind.h"
#include "battle/constant/effect_model_id.h"
#include "battle/constant/overlay_layout_id.h"
#include "battle/constant/effect_id.h"
#include "pokemon/constant/move.h"

namespace battle {

class MoveAnimation;

enum class EffectLayer : u32 {
  kFirst = 0,
  kSecond = 1,
  kThird = 2,
  kFourth = 3,
  kFifth = 4,
  kSixth = 5,
  kSeventh = 6,
  kEighth = 7,
  kAll = 0xFFFFFFFF,
};

struct AnimationKey {
  s32 frame;
  Vec3 value;
};

struct ChainWindow {
  s32 start_frame;
  s32 end_frame;
};

class AnimationStep {
public:
  AnimationStep() : animation_(nullptr), offset_(0) {}

  INLINE bool IsValid() const { return animation_ != nullptr; }

  AnimationStepKind kind() const;
  s32 start_frame() const;
  s32 end_frame() const;
  u16 group() const;
  u32 value_count() const;

  void SetStartFrame(s32 frame);
  void SetEndFrame(s32 frame);
  void SetGroup(u16 group);

  s32 GetS32(u32 index) const;
  f32 GetF32(u32 index) const;
  Vec3 GetVec3(u32 index) const;

  void SetS32(u32 index, s32 value);
  void SetF32(u32 index, f32 value);
  void SetVec3(u32 index, const Vec3& value);
  void SetText(u32 index, const char* text, u32 length);

private:
  friend class MoveAnimation;

  AnimationStep(MoveAnimation* animation, u32 offset)
    : animation_(animation), offset_(offset) {
  }

  u8* Values() const;
  bool HasValue(u32 index, u32 width) const;

  MoveAnimation* animation_;
  u32 offset_;
};

class MoveAnimation {
public:
  typedef std::function<void(AnimationStep&)> StepVisitor;

  MoveAnimation(u8* data, u32 size, u32 capacity)
    : data_(data), size_(size), capacity_(capacity) {
  }

  INLINE u32 size() const { return size_; }

  static u32 WriteEmpty(u8* buffer);
  void SetDuration(s32 frames);

  u32 step_count() const;
  s32 last_frame() const;

  AnimationStep StepAt(u32 index);
  AnimationStep First(AnimationStepKind kind, u32 skip = 0);
  u32 Count(AnimationStepKind kind) const;
  void ForEach(AnimationStepKind kind, const StepVisitor& visitor);

  AnimationStep Add(AnimationStepKind kind, s32 start_frame, s32 end_frame,
                    u16 group = 0,
                    StepCondition condition = StepCondition::kAlways);
  bool Remove(const AnimationStep& step);
  u32 RemoveAll(AnimationStepKind kind);

  void ScaleEffects(f32 factor);
  void ScaleEffects(const Vec3& factor);
  void TurnEffects(const Vec3& degrees);
  void OffsetEffects(const Vec3& offset);
  void ReplaceEffect(EffectId from, EffectId to);
  void TintEffect(EffectId effect, const Color8& multiplier,
                    EffectLayer layer = EffectLayer::kAll);
  void TintEffects(const Color8& multiplier);
  void ColorEffect(EffectId effect, const Color8& color,
                     EffectLayer layer = EffectLayer::kAll);
  void RecolorEffect(EffectId effect, const Color8& start,
                       const Color8& middle, const Color8& end,
                       EffectLayer layer = EffectLayer::kAll);
  void ColorEffects(const Color8& color);
  void RecolorEffects(const Color8& start, const Color8& middle,
                        const Color8& end);
  void ScaleEffectLife(EffectId effect, f32 factor,
                         EffectLayer layer = EffectLayer::kAll);
  void ScaleEffectDensity(EffectId effect, f32 factor,
                            EffectLayer layer = EffectLayer::kAll);
  void ScaleEffectSize(EffectId effect, f32 factor,
                         EffectLayer layer = EffectLayer::kAll);
  void FadeEffect(EffectId effect, f32 fade_in_end, f32 fade_out_start,
                    f32 peak = 1.0f, EffectLayer layer = EffectLayer::kAll);
  void LoopEffect(EffectId effect,
                    EffectLayer layer = EffectLayer::kAll);
  void DisableEffect(EffectId effect,
                       EffectLayer layer = EffectLayer::kAll);
  void ScaleEffectsLife(f32 factor);
  void ScaleEffectsDensity(f32 factor);
  void ScaleEffectsSize(f32 factor);
  void FadeEffects(f32 fade_in_end, f32 fade_out_start, f32 peak = 1.0f);
  void LoopEffects();
  void ScaleModels(f32 factor);

#include "battle/patch/move_animation_steps.inc"

  static bool Validate(const u8* data, u32 size, u32* used_size);

  enum class LimitProblem : u32 {
    kNone = 0,
    kGroupOutOfRange,
    kGroupStillAlive,
  };

  LimitProblem FindLimitProblem(u32* step_index, u32* limit) const;

private:
  friend class AnimationStep;

  bool NextOffset(u32 offset, u32* next) const;
  u32 InsertionOffset(s32 start_frame) const;
  void WriteMaxFrame(s32 frame);

  u8* data_;
  u32 size_;
  u32 capacity_;
};

class MoveAnimations {
  MAKE_SINGLETON(MoveAnimations)

public:
  typedef void (*Editor)(MoveAnimation& animation);

  static constexpr u32 kMaxEditors = 64;

  static constexpr u32 kMaxDefinitions = 32;

  static bool Edit(pokemon::MoveId move, Editor editor);
  static bool EditFile(u32 file_id, Editor editor);
  static bool Define(pokemon::MoveId move, Editor build);
  static void Clear();

  static bool Resolve(pokemon::MoveId move, u32* id, bool* is_move);

  static u32 FileOf(pokemon::MoveId move);
  static bool HasEditor(u32 file_id);

  static u32 PatchedSize(u32* archive, u32 file_id);
  static bool Serve(u32* archive, u32 file_id, void* buffer, u32* size);
  static void ApplyEditors(u32 file_id, MoveAnimation& animation);
  static void ApplyDefinition(u32 index, MoveAnimation& animation);
  static u32 ScratchOwner(u32 file_id);

  static void LoadEffectsOnDemand(bool enabled = true);
  static bool AreEffectsOnDemand();

private:
  struct EditorEntry {
    u32 file_id;
    Editor editor;
  };

  struct DefinitionEntry {
    pokemon::MoveId move;
    Editor build;
  };

  EditorEntry editors_[kMaxEditors];
  u32 editor_count_ = 0;
  DefinitionEntry definitions_[kMaxDefinitions];
  u32 definition_count_ = 0;
  u32 active_definition_ = kMaxDefinitions;
  bool effects_on_demand_ = false;
};

}
