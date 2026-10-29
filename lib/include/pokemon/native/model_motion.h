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
 * @file model_motion.h
 * @brief The skeleton motion of a Pokémon model.
 */

#pragma once

#include "common.h"
#include "pokemon/address.h"

namespace pokemon {

/**
 * @brief The motion object that moves the bones of a Pokémon model.
 *
 * The motion data is the first resource of the pack section 5 of the
 * Pokémon (the battle animations). Motion 0 of the data is the idle loop.
 * ORAS only.
 */
struct ModelMotion {
  static constexpr u32 kSize = 0x1A8;
  /// The number of the first motion of the data.
  static constexpr s32 kFirstMotion = 2;
  static constexpr u32 kAttributeOffset = 0x140;
  static constexpr u32 kScaleFlagsOffset = 0x14B;
  static constexpr u32 kRootPositionOffset = 0x150;
  static constexpr u32 kRootRotationOffset = 0x15C;
  static constexpr u32 kRootScaleOffset = 0x168;
  static constexpr u32 kLastScaleOffset = 0x19C;
  /// Attribute bits: no leg correction, loop.
  static constexpr u32 kNoCorrection = 0x10;
  static constexpr u32 kLoop = 0x20;

#ifdef GAME_ORAS
  /// Makes a motion for a model. Returns null when the data or the model
  /// are not valid. @a heap_base is the heap of the game for the object.
  static void* Create(void* heap_base, void* motion_data, void* model) {
    void* motion = ((void* (*)(u32, void*))address::kHeapNew)(kSize, heap_base);
    if (motion == nullptr) return nullptr;
    u8* bytes = static_cast<u8*>(motion);
    for (u32 i = 0; i < kSize; ++i) bytes[i] = 0;
    *(uptr*)bytes = address::kMotionVtable;
    *(f32*)(bytes + 0x20) = 1.0f; // The speed of the frames.
    // The scale of the root, and the scale of the last frame: 1 for all axes.
    for (u32 i = 0; i < 3; ++i) {
      *(f32*)(bytes + kRootScaleOffset + i * 4) = 1.0f;
      *(f32*)(bytes + kLastScaleOffset + i * 4) = 1.0f;
    }
    ((void (*)(void*, void*, void*, void*, void*))address::kMotionInitialize)(
        motion, motion_data, heap_base, model, nullptr);
    *(u32*)(bytes + kAttributeOffset) |= kNoCorrection | kLoop;
    bytes[kScaleFlagsOffset] = 1;
    bytes[kScaleFlagsOffset + 1] = 1;
    return motion;
  }

  /// Plays a motion of the data. Returns false when it does not exist.
  static bool Play(void* motion, s32 index) {
    // The change function is the 7th function of the vtable.
    uptr* vtable = *(uptr**)motion;
    return ((bool (*)(void*, s32))vtable[6])(motion, kFirstMotion + index);
  }

  /// Returns true when the data has this motion.
  static bool IsAvailable(void* motion, s32 index) {
    return ((u32 (*)(void*, s32))address::kMotionIsAvailable)(
               motion, kFirstMotion + index) > 0;
  }

  /// Returns true when the motion is at its last frame.
  static bool IsFinished(void* motion) {
    const f32 frame = *(f32*)((u8*)motion + 0x14);
    const f32 count = *(f32*)((u8*)motion + 0x1C);
    return frame >= count - 1.01f;
  }

  /// Chooses if the motion plays again when it ends.
  static void SetLoop(void* motion, bool is_loop) {
    u32& attribute = *(u32*)((u8*)motion + kAttributeOffset);
    attribute = is_loop ? (attribute | kLoop) : (attribute & ~kLoop);
  }

  /// Sets where the motion puts the model: the bones of the motion are in the
  /// world, so the model does not use its own position.
  static void SetRoot(void* motion, const Vec3& position, const Vec3& rotation,
                      const Vec3& scale) {
    u8* bytes = static_cast<u8*>(motion);
    *(Vec3*)(bytes + kRootPositionOffset) = position;
    *(Vec3*)(bytes + kRootRotationOffset) = rotation;
    *(Vec3*)(bytes + kRootScaleOffset) = scale;
  }

  /// Moves the motion forward one frame.
  static void Advance(void* motion) {
    ((void (*)(void*, f32))address::kMotionUpdate)(motion, 1.0f);
  }

  /// Writes the bones of the motion to the model. The game calculates the
  /// bones of a model before it draws it, so call this function after that
  /// calculation and before the draw.
  static void Apply(void* motion, void* model) {
    ((void (*)(void*, void*))address::kMotionApplyToModel)(motion, model);
  }

  static void SetSpeed(void* motion, f32 speed) {
    *(f32*)((u8*)motion + 0x20) = speed;
  }

  /// Destroys a motion that Create made.
  static void Destroy(void* motion) {
    uptr* vtable = *(uptr**)motion;
    ((void (*)(void*))vtable[1])(motion);
  }
#endif
};

} // namespace pokemon
