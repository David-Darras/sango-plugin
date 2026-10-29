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
 * @file model_walk.h
 * @brief Moves the legs of a Pokémon model while it walks.
 */

#pragma once

#include "common.h"

namespace renderer {
struct H3dModel;
}

namespace pokemon {

/**
 * @brief The legs of a Pokémon model, and the state of its walk.
 *
 * The Pokémon of the game have no walk animation. The code looks at the
 * skeleton of the model and chooses a walk:
 * - a biped (two feet): the legs swing forward and back, in opposite ways;
 * - a quadruped (the hands are on the ground, in front of the feet): the four
 *   legs swing in diagonal pairs;
 * - a Pokémon with many legs and no feet (a spider): the legs swing in turn;
 * - a Pokémon that flies or floats, or that has no leg: no walk.
 */
struct ModelWalk {
  static constexpr u32 kMaxLegs = 12;

  /// The walk of a skeleton.
  enum class Kind : u8 {
    kUnknown, ///< The skeleton is not read yet.
    kNone, ///< No leg movement.
    kBiped,
    kQuadruped,
    kMultiLegged,
  };

  bool can_walk = true; ///< false: the Pokémon flies or floats. Set by the loader.
  bool is_searched = false; ///< true: the legs are in the list below.
  Kind kind = Kind::kUnknown;
  u32 leg_count = 0;
  u16 leg_bone[kMaxLegs] = {}; ///< The bone that swings: the top of each leg.
  u16 tip_bone[kMaxLegs] = {}; ///< The foot or the hand at the end of each leg.
  u8 leg_side[kMaxLegs] = {}; ///< 0: left, 1: right.
  u8 leg_phase[kMaxLegs] = {}; ///< 0 or 1: the two phases swing in opposite ways.
  f32 phase = 0.0f; ///< The position in the walk cycle, in radians.
  f32 blend = 0.0f; ///< 0 for a Pokémon that stands, 1 for a Pokémon that walks.
  bool has_forward = false; ///< true: forward_angle is the direction of the face.
  f32 forward_angle = 0.0f; ///< The direction that the face looks at, in radians (atan2 of x and z).
  u16 head_bone = 0xFFFF; ///< The head, and a bone in front of the head.
  u16 front_bone = 0xFFFF;
  /// The pairs of bones of the left and the right side (LThigh and RThigh...).
  /// The line from the right bones to the left bones gives the direction of the
  /// body: it does not move like a head does.
  static constexpr u32 kMaxPairs = 8;
  u16 pair_left[kMaxPairs] = {};
  u16 pair_right[kMaxPairs] = {};
  u32 pair_count = 0;

  /// Moves the walk forward one frame. @a is_walking tells if the Pokémon
  /// must walk.
  void Advance(bool is_walking);

  /// Swings the legs and moves the body. Call it after the game wrote the
  /// bones of the model, before the draw. ORAS only.
  void Apply(renderer::H3dModel* model);

private:
  /// Reads the direction of the face from the bones.
  void UpdateForward(const f32* worlds, u32 count);
  /// Finds the face, the legs of the skeleton, and the kind of walk.
  void Search(renderer::H3dModel* model, const f32* worlds, const u16* parents,
              u32 count);
};

} // namespace pokemon
