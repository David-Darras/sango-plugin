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
 * @file following_pokemon.cc
 * @brief The movement of a Pokémon that follows the player in the overworld.
 *
 * The declarations are in pokemon/patch/following_pokemon.h.
 */

#include "pokemon/patch/following_pokemon.h"

#include <cmath>

#include "core/utils.h"
#include "overworld/native/model_manager.h"
#include "renderer/native/h3d_shader_model.h"

namespace pokemon {
namespace {
// The distances are in world units: a tile has 18 units.
constexpr f32 kMinDistance = 16.0f; // The Pokémon goes away from the player when it is nearer.
constexpr f32 kWanderMin = 24.0f; // The rest points are between these two distances.
constexpr f32 kWanderMax = 48.0f;
constexpr f32 kFarDistance = 72.0f; // After this distance, the Pokémon catches up.
constexpr f32 kTeleportDistance = 240.0f; // After this distance, the Pokémon appears again.
constexpr f32 kArriveDistance = 3.0f;
constexpr f32 kWalkSpeed = 1.3f; // Units for each frame.
constexpr f32 kRunSpeed = 3.6f;
constexpr f32 kMovingSpeed = 0.35f; // Above this speed, the Pokémon walks.
constexpr u32 kStillFramesBeforeStop = 6;
constexpr u32 kGoalFramesMin = 150;
constexpr u32 kGoalFramesRandom = 250;
constexpr f32 kTurnWalking = 0.18f; // The part of the turn that the Pokémon does in one frame.
constexpr f32 kTurnResting = 0.05f;
constexpr f32 kMaxTurnStep = 0.12f; // Radians for each frame.
constexpr f32 kPi = 3.14159265f;

// Returns true for a number that a position or a turn can have. The game gives
// wrong numbers when it loads a map.
bool IsReasonable(f32 value) { return value > -1.0e6f && value < 1.0e6f; }

// Brings an angle to the range from -pi to pi. A loop would never end for a
// very large number: the code calculates the turns instead.
f32 WrapAngle(f32 angle) {
  if (!IsReasonable(angle)) return 0.0f;
  return angle - 2.0f * kPi * std::floor((angle + kPi) / (2.0f * kPi));
}

f32 RandomUnit() {
  return static_cast<f32>(core::Utils::GetRandomValue(1000)) / 1000.0f;
}
} // namespace

void FollowingPokemon::Reset() { GetInstance().is_placed_ = false; }

void FollowingPokemon::Update(LoadedModel* pokemon) {
  auto& self = GetInstance();
  if (pokemon == nullptr || pokemon->model == nullptr) {
    self.is_placed_ = false;
    return;
  }

  auto& player = overworld::ModelManager::GetInstance().GetPlayer();
  // The player has no model while the game loads a map.
  overworld::DrawModel* player_draw = player.GetDrawModelOrNull();
  if (player_draw == nullptr) {
    self.is_placed_ = false;
    return;
  }
  overworld::DrawModel& player_model = *player_draw;
  if (!IsReasonable(player_model.position.x) ||
      !IsReasonable(player_model.position.y) ||
      !IsReasonable(player_model.position.z) ||
      !IsReasonable(player_model.rotation.y)) {
    self.is_placed_ = false;
    return;
  }
  const Vec3 center = player_model.position;
  const Vec3 facing = player.facing_direction;

  // The turn of the models of the game compared to the direction: it is the
  // same for the player and for the Pokémon.
  // The model of the player turns slowly after a change of direction, so the
  // code reads the turn only when the model does not move. The turn is a
  // multiple of a quarter of a circle.
  static f32 yaw_offset = 0.0f;
  static f32 last_player_yaw = 0.0f;
  const bool is_player_turning =
      std::fabs(WrapAngle(player_model.rotation.y - last_player_yaw)) > 0.001f;
  last_player_yaw = player_model.rotation.y;
  if (!is_player_turning && (facing.x != 0.0f || facing.z != 0.0f)) {
    const f32 raw =
        WrapAngle(player_model.rotation.y - std::atan2(facing.x, facing.z));
    yaw_offset = std::floor(raw / (kPi / 2.0f) + 0.5f) * (kPi / 2.0f);
  }

  // The face of the model: the code reads where it looks at on the bones, and
  // how the model turns when the code changes the turn. The face looks at the
  // direction (turn_sign * turn + offset). Without bones, the turn of the
  // player gives the offset.
  const bool has_face = pokemon->walk.has_forward && self.is_placed_;
  f32 face_offset = yaw_offset;
  if (has_face) {
    const f32 measured = pokemon->walk.forward_angle;
    if (self.has_last_) {
      // How the face moved when the code turned the model: only a large turn
      // counts, the animations move the head a little. Some votes decide.
      const f32 turned = WrapAngle(self.yaw_ - self.last_yaw_);
      const f32 moved_face = WrapAngle(measured - self.last_forward_);
      if (std::fabs(turned) > 0.08f && std::fabs(moved_face) > 0.05f) {
        self.sign_votes_ += turned * moved_face >= 0.0f ? 1 : -1;
        if (self.sign_votes_ > 6) self.sign_votes_ = 6;
        if (self.sign_votes_ < -6) self.sign_votes_ = -6;
        self.turn_sign_ = self.sign_votes_ >= 0 ? 1.0f : -1.0f;
      }
    }
    self.last_forward_ = measured;
    self.last_yaw_ = self.yaw_;
    self.has_last_ = true;
    // The offset changes slowly: the idle animation moves the head.
    const f32 raw = WrapAngle(measured - self.turn_sign_ * self.yaw_);
    if (!self.has_offset_) {
      self.face_offset_ = raw;
      self.has_offset_ = true;
    } else {
      self.face_offset_ = WrapAngle(self.face_offset_ +
                                    WrapAngle(raw - self.face_offset_) * 0.15f);
    }
    face_offset = self.face_offset_;
  } else {
    self.has_last_ = false;
    self.has_offset_ = false;
  }
  // The turn of the model that makes the face look at a direction.
  auto turn_for = [&](f32 direction) {
    return has_face ? self.turn_sign_ * WrapAngle(direction - face_offset)
                    : direction + yaw_offset;
  };

  f32 away_x = self.position_.x - center.x;
  f32 away_z = self.position_.z - center.z;
  f32 distance = std::sqrt(away_x * away_x + away_z * away_z);

  if (!self.is_placed_ || distance > kTeleportDistance) {
    self.position_ = Vec3(center.x - facing.x * kWanderMin, center.y,
                          center.z - facing.z * kWanderMin);
    self.goal_ = self.position_;
    self.yaw_ = player_model.rotation.y;
    self.speed_ = 0.0f;
    self.goal_frames_ = 0;
    self.is_placed_ = true;
    away_x = self.position_.x - center.x;
    away_z = self.position_.z - center.z;
    distance = std::sqrt(away_x * away_x + away_z * away_z);
  }
  if (distance < 0.001f) {
    away_x = 1.0f;
    away_z = 0.0f;
    distance = 1.0f;
  }

  // The goal of the Pokémon.
  const bool is_far = distance > kFarDistance;
  if (is_far) {
    // The player left: go to the player in a straight line.
    self.goal_ = Vec3(center.x + away_x / distance * kWanderMin, center.y,
                      center.z + away_z / distance * kWanderMin);
    self.goal_frames_ = 0;
  } else if (distance < kMinDistance) {
    // The player is near: go away from the player.
    self.goal_ = Vec3(center.x + away_x / distance * kWanderMin, center.y,
                      center.z + away_z / distance * kWanderMin);
    self.goal_frames_ = kGoalFramesMin;
  } else if (self.goal_frames_ == 0) {
    // A new place to rest, near the player.
    const f32 angle = RandomUnit() * 2.0f * kPi;
    const f32 radius = kWanderMin + RandomUnit() * (kWanderMax - kWanderMin);
    self.goal_ = Vec3(center.x + std::sin(angle) * radius, center.y,
                      center.z + std::cos(angle) * radius);
    self.goal_frames_ = kGoalFramesMin + core::Utils::GetRandomValue(kGoalFramesRandom);
  } else {
    self.goal_frames_--;
  }

  // The walk to the goal.
  const f32 to_goal_x = self.goal_.x - self.position_.x;
  const f32 to_goal_z = self.goal_.z - self.position_.z;
  const f32 goal_distance = std::sqrt(to_goal_x * to_goal_x + to_goal_z * to_goal_z);
  f32 wanted_speed = 0.0f;
  f32 turn_to_goal = 0.0f;
  if (goal_distance > kArriveDistance) {
    wanted_speed = goal_distance * 0.12f;
    const f32 top = is_far ? kRunSpeed : kWalkSpeed;
    if (wanted_speed > top) wanted_speed = top;
    if (wanted_speed < 0.6f) wanted_speed = 0.6f;
    // The Pokémon walks forward: it turns first, then it goes.
    turn_to_goal = WrapAngle(turn_for(std::atan2(to_goal_x, to_goal_z)) -
                             self.yaw_);
    const f32 alignment = std::cos(turn_to_goal);
    wanted_speed = alignment > 0.5f ? wanted_speed * alignment : 0.0f;
  }
  self.speed_ += (wanted_speed - self.speed_) * 0.2f;
  if (self.speed_ < 0.02f) self.speed_ = 0.0f;

  f32 target_yaw;
  f32 turn;
  if (goal_distance > kArriveDistance) {
    // The Pokémon goes to the goal, and it looks at it.
    if (self.speed_ > 0.0f) {
      const f32 step = self.speed_ < goal_distance ? self.speed_ : goal_distance;
      self.position_.x += to_goal_x / goal_distance * step;
      self.position_.z += to_goal_z / goal_distance * step;
    }
    target_yaw = turn_for(std::atan2(to_goal_x, to_goal_z));
    turn = kTurnWalking;
  } else {
    // The Pokémon rests: it looks at the player.
    target_yaw = turn_for(std::atan2(-away_x, -away_z));
    turn = kTurnResting;
  }
  // The turn is smooth, and not too fast: a fast turn confuses the reading
  // of the face.
  f32 turn_step = WrapAngle(target_yaw - self.yaw_) * turn;
  if (turn_step > kMaxTurnStep) turn_step = kMaxTurnStep;
  if (turn_step < -kMaxTurnStep) turn_step = -kMaxTurnStep;
  self.yaw_ += turn_step;
  self.position_.y += (center.y - self.position_.y) * 0.2f;

  // The Pokémon walks while it moves, and a few frames more.
  self.still_frames_ = self.speed_ > kMovingSpeed ? 0 : self.still_frames_ + 1;
  pokemon->is_walking = self.still_frames_ < kStillFramesBeforeStop;
  // A fast follower gallops.
  pokemon->walk.is_running = self.speed_ > (kWalkSpeed + kRunSpeed) * 0.5f;

  pokemon->model->SetTranslate(self.position_);
  pokemon->model->SetRotate(Vec3(0.0f, self.yaw_, 0.0f));
}

} // namespace pokemon
