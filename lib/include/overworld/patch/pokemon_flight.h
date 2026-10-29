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
 * @file pokemon_flight.h
 * @brief The player flies on the back of a flying Pokémon.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/species.h"
#include "pokemon/patch/model_loader.h"

namespace overworld {

/**
 * @brief The player flies on a flying Pokémon (page Cheats).
 *
 * A Pokémon model is put under the player, and the player sits on it without
 * moving. The plugin moves them both: the Circle Pad gives the direction, and
 * the keys L and R make them go down and up. The speed and the climb are
 * smooth. The collisions are off and the wild Pokémon do not appear. The
 * Pokémon of the team that follows the player leaves during the flight. ORAS
 * only.
 */
class PokemonFlight {
  MAKE_SINGLETON(PokemonFlight)

public:
  bool is_enabled = false; ///< The switch of the page Cheats.
  SpeciesId species = SpeciesId::kStaraptor; ///< The Pokémon that carries the player.
  f32 scale = 0.5f; ///< The size of the model.
  f32 speed = 1.0f; ///< The speed of the flight: 1 is a little faster than the run.
  f32 climb_speed = 2.5f; ///< The speed of the climb, in units for each frame.

  /// Called when the switch changes: lands the player when it goes off.
  static void Apply(void*);

  /// Called at each frame by plugin::UpdateFrame().
  static void Update();

  /// Returns true when the player sits on the Pokémon: the walk of the player
  /// is then replaced by the sit pose (see PlayerCheats).
  static bool IsFlying();

  /// The direction of the flight as a Facing number: the sit order uses it.
  static u32 GetSitDirection();

private:
  static constexpr f32 kMaxAltitude = 1500.0f;
  static constexpr u32 kReadyDelayFrames = 20;
  /// The height of the seat above the middle of the body of the Pokémon, for a
  /// model with the scale 1.
  static constexpr f32 kSeatLift = 10.0f;
  /// The speed of the flight with the Circle Pad at its end, in units for each
  /// frame (a tile is 18 units).
  static constexpr f32 kBaseSpeed = 4.0f;

  void Fly();
  void Land();
  /// Removes the model and gives the player back to the game.
  void Stop();
  /// The model leaves the scene (a map loads, a menu opens...).
  void LeaveScene(bool is_scene_alive);
  bool Spawn();
  void SitDown();
  void MoveModel(f32 player_x, f32 player_y, f32 player_z);

  pokemon::LoadedModel model_;
  bool is_active_ = false; ///< true: the switches of the game are changed.
  bool was_following_ = false;
  bool was_no_collision_ = false;
  u8 was_encounter_mode_ = 0;
  bool has_base_ = false;
  bool has_seat_ = false;
  bool has_offset_ = false;
  bool needs_sit_ = true;
  f32 base_y_ = 0.0f;
  f32 last_y_ = 0.0f;
  f32 altitude_ = 0.0f;
  f32 climb_ = 0.0f; ///< The speed of the climb now.
  f32 velocity_x_ = 0.0f; ///< The speed of the flight now.
  f32 velocity_z_ = 0.0f;
  f32 heading_ = 0.0f; ///< The direction of the flight, in radians (atan2 of x and z).
  f32 player_offset_ = 0.0f; ///< The turn of the model of the player for a direction.
  f32 seat_[3] = {0, 0, 0}; ///< The seat, from the origin of the model.
  f32 yaw_ = 0.0f;
  f32 yaw_offset_ = 0.0f;
  u16 seat_bone_ = 0xFFFF;
  u32 frame_ = 0;
  u32 ready_frame_ = 0;
  u32 last_map_ = 0xFFFFFFFF;
};

} // namespace overworld
