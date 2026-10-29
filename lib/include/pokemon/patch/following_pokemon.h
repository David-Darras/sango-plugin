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
 * @file following_pokemon.h
 * @brief The movement of a Pokémon that follows the player in the overworld.
 */

#pragma once

#include "common.h"
#include "pokemon/patch/model_loader.h"

namespace pokemon {

/**
 * @brief Moves a Pokémon model near the player.
 *
 * The Pokémon has its own position. It does not stay at a fixed place behind
 * the player: it chooses a point near the player, walks to it, rests, then
 * chooses a new point. When the player goes far away, the Pokémon walks or
 * runs to catch up with the player. When the player is very far (a new map,
 * a warp), the Pokémon appears behind the player.
 */
class FollowingPokemon {
  MAKE_SINGLETON(FollowingPokemon)

public:
  /// Moves the model one frame. Call it one time for each frame, in the
  /// overworld. It sets the position, the turn and the walk of the model.
  static void Update(LoadedModel* pokemon);

  /// Forgets the position: the Pokémon appears behind the player at the next
  /// frame.
  static void Reset();

private:
  bool is_placed_ = false;
  Vec3 position_;
  Vec3 goal_;
  f32 yaw_ = 0.0f;
  f32 speed_ = 0.0f;
  u32 goal_frames_ = 0;
  u32 still_frames_ = 0;
  // The direction of the face of the model, measured on its bones, and the
  // turn that the code gave to the model. They tell how the model turns.
  f32 last_forward_ = 0.0f;
  f32 last_yaw_ = 0.0f;
  f32 turn_sign_ = 1.0f; ///< 1 or -1: the face turns the same way as the model, or the other way.
  s32 sign_votes_ = 3; ///< Positive: the face turns the same way as the model.
  f32 face_offset_ = 0.0f; ///< The face looks at (turn_sign * turn + offset).
  bool has_offset_ = false;
  bool has_last_ = false;
};

} // namespace pokemon
