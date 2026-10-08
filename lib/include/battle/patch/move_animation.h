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
 * @file move_animation.h
 * @brief Changes the move animations, or makes new ones.
 *
 * @see docs/tutorials/06-make-a-move-animation.md
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

/// A layer of a particle effect. An effect has up to 8 layers.
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

/// A value at a frame.
struct AnimationKey {
  s32 frame;
  Vec3 value;
};

struct ChainWindow {
  s32 start_frame;
  s32 end_frame;
};

/// One step of a move animation. MoveAnimation returns it.
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

/**
 * @brief A move animation: a list of steps.
 *
 * Each step has a start frame, an end frame and a group. The functions of
 * move_animation_steps.inc add the steps (for example EffectSpawn()). The
 * other functions change the effects of the animation.
 */
class MoveAnimation {
public:
  typedef std::function<void(AnimationStep&)> StepVisitor;

  MoveAnimation(u8* data, u32 size, u32 capacity)
    : data_(data), size_(size), capacity_(capacity) {
  }

  INLINE u32 size() const { return size_; }

  static u32 WriteEmpty(u8* buffer);
  void SetDuration(s32 frames);

  /// Returns the number of steps.
  u32 step_count() const;
  /// Returns the last frame of the animation.
  s32 last_frame() const;

  /// Returns a step.
  AnimationStep StepAt(u32 index);
  /// Returns the first step of a type (after `skip` steps of this type).
  AnimationStep First(AnimationStepKind kind, u32 skip = 0);
  /// Returns the number of steps of a type.
  u32 Count(AnimationStepKind kind) const;
  /// Calls a function for each step of a type.
  void ForEach(AnimationStepKind kind, const StepVisitor& visitor);

  /// Adds an empty step. Use the typed functions instead (EffectSpawn()...).
  AnimationStep Add(AnimationStepKind kind, s32 start_frame, s32 end_frame,
                    u16 group = 0,
                    StepCondition condition = StepCondition::kAlways);
  /// Removes a step.
  bool Remove(const AnimationStep& step);
  /// Removes all the steps of a type.
  u32 RemoveAll(AnimationStepKind kind);

  /// Changes the size of all the effects.
  void ScaleEffects(f32 factor);
  void ScaleEffects(const Vec3& factor);
  /// Turns all the effects.
  void TurnEffects(const Vec3& degrees);
  /// Moves all the effects.
  void OffsetEffects(const Vec3& offset);
  /// Uses the effect `to` instead of the effect `from`.
  void ReplaceEffect(EffectId from, EffectId to);
  /// Multiplies the colors of an effect.
  void TintEffect(EffectId effect, const Color8& multiplier,
                    EffectLayer layer = EffectLayer::kAll);
  /// Multiplies the colors of all the effects.
  void TintEffects(const Color8& multiplier);
  /// Gives one color to an effect.
  void ColorEffect(EffectId effect, const Color8& color,
                     EffectLayer layer = EffectLayer::kAll);
  /// Gives three colors to an effect: start, middle and end of the life of each particle.
  void RecolorEffect(EffectId effect, const Color8& start,
                       const Color8& middle, const Color8& end,
                       EffectLayer layer = EffectLayer::kAll);
  /// Gives one color to all the effects.
  void ColorEffects(const Color8& color);
  /// Gives three colors to all the effects.
  void RecolorEffects(const Color8& start, const Color8& middle,
                        const Color8& end);
  /// Changes how long the particles of an effect live.
  void ScaleEffectLife(EffectId effect, f32 factor,
                         EffectLayer layer = EffectLayer::kAll);
  /// Changes the number of particles of an effect.
  void ScaleEffectDensity(EffectId effect, f32 factor,
                            EffectLayer layer = EffectLayer::kAll);
  /// Changes the size of the particles of an effect.
  void ScaleEffectSize(EffectId effect, f32 factor,
                         EffectLayer layer = EffectLayer::kAll);
  /// Makes the particles of an effect appear and disappear slowly (values from 0 to 1 of the life).
  void FadeEffect(EffectId effect, f32 fade_in_end, f32 fade_out_start,
                    f32 peak = 1.0f, EffectLayer layer = EffectLayer::kAll);
  /// Makes an effect emit particles until the end of its step.
  void LoopEffect(EffectId effect,
                    EffectLayer layer = EffectLayer::kAll);
  /// Hides a layer of an effect.
  void DisableEffect(EffectId effect,
                       EffectLayer layer = EffectLayer::kAll);
  /// Changes how long the particles of all the effects live.
  void ScaleEffectsLife(f32 factor);
  /// Changes the number of particles of all the effects.
  void ScaleEffectsDensity(f32 factor);
  /// Changes the size of the particles of all the effects.
  void ScaleEffectsSize(f32 factor);
  /// Makes the particles of all the effects appear and disappear slowly.
  void FadeEffects(f32 fade_in_end, f32 fade_out_start, f32 peak = 1.0f);
  /// Makes all the effects emit until the end of their steps.
  void LoopEffects();
  /// Changes the size of all the models.
  void ScaleModels(f32 factor);

#include "battle/patch/move_animation_steps.inc"

  /// Returns true when the data is a valid animation file.
  static bool Validate(const u8* data, u32 size, u32* used_size);

  /// A limit of the engine that an animation breaks.
  enum class LimitProblem : u32 {
    kNone = 0,
    kGroupOutOfRange,
    kGroupStillAlive,
  };

  /// Checks the limits of the engine (10 effect groups, 10 model groups, 6 sound groups).
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

/**
 * @brief Registers the changed animations and the new animations.
 *
 * @code
 * battle::MoveAnimations::Edit(MoveId::kThunderbolt, EditThunderbolt);
 * battle::MoveAnimations::Define(kMyMove, BuildMyMoveAnimation);
 * @endcode
 */
class MoveAnimations {
  MAKE_SINGLETON(MoveAnimations)

public:
  /// A function that changes or builds an animation.
  typedef void (*Editor)(MoveAnimation& animation);

  static constexpr u32 kMaxEditors = 64;

  static constexpr u32 kMaxDefinitions = 32;

  /// Changes the animation of a move of the game. 64 editors at most.
  static bool Edit(pokemon::MoveId move, Editor editor);
  /// Changes an animation file by its file id.
  static bool EditFile(u32 file_id, Editor editor);
  /// Makes a new animation for a move. 32 animations at most.
  static bool Define(pokemon::MoveId move, Editor build);
  /// Removes all the editors and all the new animations.
  static void Clear();

  /// Selects the file of a move that has a new animation. The library calls it.
  static bool Resolve(pokemon::MoveId move, u32* id, bool* is_move);

  /// Returns the animation file of a move.
  static u32 FileOf(pokemon::MoveId move);
  static bool HasEditor(u32 file_id);

  static u32 PatchedSize(u32* archive, u32 file_id);
  static bool Serve(u32* archive, u32 file_id, void* buffer, u32* size);
  static void ApplyEditors(u32 file_id, MoveAnimation& animation);
  static void ApplyDefinition(u32 index, MoveAnimation& animation);
  static u32 ScratchOwner(u32 file_id);

  /// Loads the effect files only when an animation uses them.
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
