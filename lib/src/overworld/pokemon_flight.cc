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
 * @file pokemon_flight.cc
 * @brief The player flies on the back of a flying Pokémon.
 *
 * The declarations are in overworld/patch/pokemon_flight.h.
 */

#include "overworld/patch/pokemon_flight.h"

#include <cmath>
#include <cstring>

#include "core/native/event_manager.h"
#include "core/native/process_manager.h"
#include "overworld/constant/action_command.h"
#include "overworld/constant/facing.h"
#include "overworld/constant/ride_offset.h"
#include "overworld/native/map_manager.h"
#include "overworld/native/model_manager.h"
#include "overworld/patch/player_cheats.h"
#include "overworld/patch/wild_encounter.h"
#include "pokemon/constant/form.h"
#include "renderer/address.h"
#include "renderer/native/h3d_shader_model.h"
#include "renderer/patch/skeleton_editor.h"
#include "script/native/engine.h"
#include "system/native/controller.h"
#include "system/native/stick.h"
#include "ui/page/pages.h"

namespace overworld {
namespace {
constexpr f32 kPi = 3.14159265f;
constexpr f32 kClimbSmoothing = 0.12f; // The part of the climb speed change in one frame.
constexpr f32 kSpeedSmoothing = 0.06f; // The same, for the speed of the flight.
constexpr f32 kStickDeadZone = 0.15f;
constexpr f32 kWorldPerDraw = 1.0f / 9.0f; // A world unit is 9 draw units.
constexpr f32 kDrawPerTile = 18.0f;
constexpr f32 kTurnSmoothing = 0.2f;
constexpr f32 kRunSpeed = 3.4f; // The speed of the player (units for each frame) that is a run.

f32 WrapAngle(f32 angle) {
  if (!(angle > -1.0e6f && angle < 1.0e6f)) return 0.0f;
  return angle - 2.0f * kPi * std::floor((angle + kPi) / (2.0f * kPi));
}

// The game starts an action of a character: a number of the action and the
// direction.
void PlayAction(Model* model, ActionCommand action, Facing direction) {
  ((u32 (*)(Model*, u32, u32))renderer::address::kModelPlayAnimation)(
      model, static_cast<u32>(action), static_cast<u32>(direction));
}

// One of the eight directions of the game for a direction in radians (atan2 of
// x and z): up is a negative z.
Facing ToFacing(f32 heading) {
  const f32 turn = WrapAngle(heading + kPi) / (kPi / 4.0f);
  const s32 octant = static_cast<s32>(std::floor(turn + 0.5f)) & 7;
  // octant 0: up, then clockwise in the view from above: up right, right...
  static const Facing kOrder[8] = {Facing::kUp,        Facing::kUpLeft,
                                   Facing::kLeft,      Facing::kDownLeft,
                                   Facing::kDown,      Facing::kDownRight,
                                   Facing::kRight,     Facing::kUpRight};
  return kOrder[octant];
}
} // namespace

f32 PokemonFlight::GetRideHeight() const {
  const RideOffset* entry = FindRideOffset(ride_species);
  return entry != nullptr && entry->height != kRideMenu ? entry->height
                                                        : ride_height;
}

f32 PokemonFlight::GetRideBack() const {
  const RideOffset* entry = FindRideOffset(ride_species);
  return entry != nullptr && entry->back != kRideMenu ? entry->back : ride_back;
}

u32 PokemonFlight::GetSitDirection() {
  return static_cast<u32>(ToFacing(GetInstance().heading_));
}

bool PokemonFlight::IsFlying() {
  const auto& self = GetInstance();
  return self.is_active_ && !self.is_riding_ && self.model_.IsLoaded();
}

bool PokemonFlight::IsRiding() {
  const auto& self = GetInstance();
  return self.is_active_ && self.is_riding_ && self.model_.IsLoaded();
}

void PokemonFlight::Apply(void*) {
  auto& self = GetInstance();
  // The menu of the plugin is open over the overworld: the scene is there.
  if (!self.is_enabled && !self.is_ride_enabled && self.is_active_) self.Stop();
}

void PokemonFlight::Stop() {
  renderer::SkeletonEditor::ClearSeatPose();
  if (model_.IsLoaded()) {
    if (core::ProcessManager::IsOverworldActive()) {
      pokemon::ModelLoader::Drop(&model_);
    } else {
      pokemon::ModelLoader::Discard(&model_);
    }
  }
  if (is_active_) {
    if (is_riding_) {
    } else {
      Land();
      PlayerCheats::GetInstance().no_collision = was_no_collision_;
      WildEncounter::GetInstance().mode =
          static_cast<WildEncounter::Mode>(was_encounter_mode_);
    }
    FollowingPokemonSwitch() = was_following_;
    is_active_ = false;
    // The player stands up.
    if (core::ProcessManager::IsOverworldActive() &&
        MapManager::GetInstanceOrNull() != nullptr) {
      PlayAction(&ModelManager::GetInstance().GetPlayer(),
                 ActionCommand::kSitStand, ToFacing(heading_));
    }
  }
  altitude_ = 0.0f;
  climb_ = 0.0f;
  velocity_x_ = 0.0f;
  velocity_z_ = 0.0f;
  has_base_ = false;
  has_seat_ = false;
  has_offset_ = false;
  needs_sit_ = true;
  seat_bone_ = 0xFFFF;
}

void PokemonFlight::LeaveScene(bool is_scene_alive) {
  renderer::SkeletonEditor::ClearSeatPose();
  if (model_.IsLoaded()) {
    if (is_scene_alive) {
      pokemon::ModelLoader::Drop(&model_);
    } else {
      pokemon::ModelLoader::Discard(&model_);
    }
  }
  if (!is_scene_alive) pokemon::ModelLoader::ForgetGameMemory();
  has_seat_ = false;
  has_offset_ = false;
  needs_sit_ = true;
  seat_bone_ = 0xFFFF;
  ready_frame_ = frame_ + kReadyDelayFrames;
}

// The player goes back to the ground: the height is the one that the game gave.
void PokemonFlight::Land() {
  if (!has_base_) return;
  auto& player = ModelManager::GetInstance().GetPlayer();
  DrawModel* draw_model = player.GetDrawModelOrNull();
  if (draw_model == nullptr) return;
  const f32 delta = base_y_ - player.draw_pos.y;
  player.draw_pos.y += delta;
  draw_model->position.y += delta;
  player.world_pos.coords.y += delta * kWorldPerDraw;
  last_y_ = base_y_;
}

bool PokemonFlight::Spawn() {
  auto& player = ModelManager::GetInstance().GetPlayer();
  DrawModel* draw_model = player.GetDrawModelOrNull();
  if (draw_model == nullptr) return false;
  if (!pokemon::ModelLoader::LoadPokemon(
          &model_, is_riding_ ? ride_species : species, FormId::kNormal, false,
          draw_model->position)) {
    return false;
  }
  model_.model->SetScale(Scale(), Scale(), Scale());
  model_.is_walking = false;
  yaw_ = heading_;
  return true;
}

void PokemonFlight::SitDown() {
  const Facing direction = ToFacing(heading_);
  PlayAction(&ModelManager::GetInstance().GetPlayer(), ActionCommand::kSitWait,
             direction);
  needs_sit_ = false;
}

// Puts the model under the player: the seat of the model (a bone of its back)
// is at the place of the player, and the Pokémon looks where the flight goes.
void PokemonFlight::MoveModel(f32 player_x, f32 player_y, f32 player_z) {
  renderer::H3dShaderModel* model = model_.model;
  auto* h3d = reinterpret_cast<renderer::H3dModel*>(model);

  // The seat: a bone of the back, read on the bones of the last frame.
  u32 count = 0;
  const u16* parents = nullptr;
  f32* worlds = renderer::SkeletonEditor::GetWorldMatrices(h3d, &count,
                                                           &parents);
  if (worlds != nullptr) {
    if (seat_bone_ == 0xFFFF) {
      static const c8* const kSeatNames[] = {"Spine2", "Spine1", "Spine",
                                             "Waist", "Hips"};
      for (const c8* wanted : kSeatNames) {
        for (u32 bone = 0; bone < count && seat_bone_ == 0xFFFF; bone++) {
          if (std::strcmp(renderer::SkeletonEditor::GetBoneName(h3d, bone),
                          wanted) == 0) {
            seat_bone_ = static_cast<u16>(bone);
          }
        }
        if (seat_bone_ != 0xFFFF) break;
      }
    }
    if (seat_bone_ < count) {
      const f32* m = worlds + seat_bone_ * 12;
      const f32 relative[3] = {m[3] - model->position.x, m[7] - model->position.y,
                               m[11] - model->position.z};
      for (u32 i = 0; i < 3; i++) {
        seat_[i] = has_seat_ ? seat_[i] + (relative[i] - seat_[i]) * 0.3f
                             : relative[i];
      }
      has_seat_ = true;
    }
  }

  // The direction: the face of the model looks at the direction of the flight.
  if (model_.walk.has_forward) {
    const f32 raw = WrapAngle(model_.walk.forward_angle - yaw_);
    yaw_offset_ = has_offset_ ? WrapAngle(yaw_offset_ +
                                          WrapAngle(raw - yaw_offset_) * 0.15f)
                              : raw;
    has_offset_ = true;
  }
  const f32 step = WrapAngle(heading_ - yaw_offset_ - yaw_) * kTurnSmoothing;
  yaw_ += step > 0.15f ? 0.15f : (step < -0.15f ? -0.15f : step);

  const f32 lift = SeatLift();
  // The ride: the player sits behind the bone, so the model goes forward.
  const f32 back = is_riding_ ? GetRideBack() * Scale() : 0.0f;
  model->SetTranslate(Vec3(player_x - seat_[0] + std::sin(heading_) * back,
                           player_y - seat_[1] - lift,
                           player_z - seat_[2] + std::cos(heading_) * back));
  model->SetRotate(Vec3(0.0f, yaw_, 0.0f));
}

// The ride: the game moves the player as usual (the collisions, the doors and
// the wild Pokémon stay), the player sits on the Pokémon, and the Pokémon
// walks when the player moves.
void PokemonFlight::Ride() {
  auto& player = ModelManager::GetInstance().GetPlayer();
  DrawModel* draw_model = player.GetDrawModelOrNull();
  if (draw_model == nullptr) return;

  const Vec3 facing = player.facing_direction;
  const bool has_facing = facing.x != 0.0f || facing.z != 0.0f;
  if (!has_base_) {
    has_base_ = true;
    if (has_facing) heading_ = std::atan2(facing.x, facing.z);
    yaw_ = heading_ - yaw_offset_;
    last_x_ = player.draw_pos.x;
    last_z_ = player.draw_pos.z;
  }
  if (needs_sit_) SitDown();

  // The Pokémon looks where the player looks.
  if (has_facing) {
    heading_ = WrapAngle(heading_ + WrapAngle(std::atan2(facing.x, facing.z) -
                                              heading_) * 0.3f);
  }

  // The legs swing while the player moves.
  const f32 dx = player.draw_pos.x - last_x_;
  const f32 dz = player.draw_pos.z - last_z_;
  last_x_ = player.draw_pos.x;
  last_z_ = player.draw_pos.z;
  if (dx * dx + dz * dz > 0.01f) {
    still_frames_ = 0;
  } else if (still_frames_ < 1000) {
    still_frames_++;
  }
  model_.is_walking = still_frames_ < 4;
  // The player runs when it moves faster than the walk (2.6 units for each
  // frame): a quadruped gallops.
  speed_ += (std::sqrt(dx * dx + dz * dz) - speed_) * 0.2f;
  model_.walk.is_running = speed_ > kRunSpeed;
  // The player sits on the back. The game keeps the player on the ground (it
  // writes the height again when the player moves): the lift is only drawn,
  // by the bones of the player. The arms of the player hold the mane, and the
  // legs are bent to the sit pose, at rest and when the player moves.
  const f32 seat_height = (has_seat_ ? seat_[1] : 0.0f) + SeatLift();
  renderer::SkeletonEditor::SetSeatPose(
      draw_model, sizeof(DrawModel), &player, sizeof(Model),
      reinterpret_cast<const renderer::H3dModel*>(model_.model),
      std::sin(yaw_ + yaw_offset_), std::cos(yaw_ + yaw_offset_),
      model_.is_walking, seat_height,
      ride_spread, draw_model->position.y);
  MoveModel(draw_model->position.x, draw_model->position.y + seat_height,
            draw_model->position.z);
}

void PokemonFlight::Fly() {
  auto& player = ModelManager::GetInstance().GetPlayer();
  DrawModel* draw_model = player.GetDrawModelOrNull();
  if (draw_model == nullptr) return;
  auto& controller = sys::Controller::GetInstance();

  // The first frame: the turn of the model of the player compared to the
  // direction (a multiple of a quarter of a circle), and the start direction.
  if (!has_base_) {
    const Vec3 facing = player.facing_direction;
    if (facing.x != 0.0f || facing.z != 0.0f) {
      heading_ = std::atan2(facing.x, facing.z);
      const f32 raw = WrapAngle(draw_model->rotation.y - heading_);
      player_offset_ = std::floor(raw / (kPi / 2.0f) + 0.5f) * (kPi / 2.0f);
    }
    yaw_ = heading_ - yaw_offset_;
  }

  // The player sits on the Pokémon, one time. The game cannot start a walk
  // after that (see PlayerCheats): the sit command takes the place of it.
  if (needs_sit_) SitDown();

  // The Circle Pad gives the direction and the speed. The speed grows and
  // falls slowly, so the flight does not shake.
  f32 stick_x = 0.0f;
  f32 stick_y = 0.0f;
  sys::Stick::Read(&stick_x, &stick_y);
  f32 length = std::sqrt(stick_x * stick_x + stick_y * stick_y);
  if (!(length > kStickDeadZone)) {
    stick_x = 0.0f;
    stick_y = 0.0f;
  } else if (length > 1.0f) {
    stick_x /= length;
    stick_y /= length;
  }
  // The D-pad gives the same directions (for a player without a Circle Pad,
  // or when the emulator does not give the Circle Pad).
  if (controller.IsKeyDown(Key::kLeft)) stick_x -= 1.0f;
  if (controller.IsKeyDown(Key::kRight)) stick_x += 1.0f;
  if (controller.IsKeyDown(Key::kUp)) stick_y += 1.0f;
  if (controller.IsKeyDown(Key::kDown)) stick_y -= 1.0f;
  length = std::sqrt(stick_x * stick_x + stick_y * stick_y);
  if (length > 1.0f) {
    stick_x /= length;
    stick_y /= length;
  }
  const f32 top_speed = kBaseSpeed * speed;
  velocity_x_ += (stick_x * top_speed - velocity_x_) * kSpeedSmoothing;
  velocity_z_ += (-stick_y * top_speed - velocity_z_) * kSpeedSmoothing;
  if (std::sqrt(velocity_x_ * velocity_x_ + velocity_z_ * velocity_z_) > 0.4f) {
    heading_ = WrapAngle(heading_ + WrapAngle(std::atan2(velocity_x_, velocity_z_) -
                                              heading_) * 0.12f);
  }

  // The climb: L goes down and R goes up.
  f32 wanted = 0.0f;
  if (controller.IsKeyDown(Key::kR)) wanted += climb_speed;
  if (controller.IsKeyDown(Key::kL)) wanted -= climb_speed;
  climb_ += (wanted - climb_) * kClimbSmoothing;
  altitude_ += climb_;
  // The altitude is from the ground of the take-off. It can be negative: the
  // ground can be much lower in other parts of the map.
  if (altitude_ < -kMaxAltitude) {
    altitude_ = -kMaxAltitude;
    climb_ = 0.0f;
  }
  if (altitude_ > kMaxAltitude) {
    altitude_ = kMaxAltitude;
    climb_ = 0.0f;
  }

  // The height. The game puts the player on the ground when it moves it. When
  // the height changed since the last frame, the game wrote a new ground
  // height: it is the new base, and the altitude goes over it.
  const f32 y_now = player.draw_pos.y;
  if (!has_base_ || std::fabs(y_now - last_y_) > 0.05f) {
    base_y_ = y_now;
    has_base_ = true;
  }
  const f32 delta_y = base_y_ + altitude_ - y_now;
  last_y_ = base_y_ + altitude_;

  // The move: the same change for all the positions of the player, as the
  // character that follows the player does it.
  const f32 delta_x = velocity_x_;
  const f32 delta_z = velocity_z_;
  player.draw_pos.x += delta_x;
  player.draw_pos.y += delta_y;
  player.draw_pos.z += delta_z;
  draw_model->position.x += delta_x;
  draw_model->position.y += delta_y;
  draw_model->position.z += delta_z;
  player.world_pos.coords.x += delta_x * kWorldPerDraw;
  player.world_pos.coords.y += delta_y * kWorldPerDraw;
  player.world_pos.coords.z += delta_z * kWorldPerDraw;
  player.map_pos.coords.x = std::floor(player.draw_pos.x / kDrawPerTile);
  player.map_pos.coords.z = std::floor(player.draw_pos.z / kDrawPerTile);

  MoveModel(draw_model->position.x, draw_model->position.y,
            draw_model->position.z);
}

void PokemonFlight::Update() {
  auto& self = GetInstance();
  self.frame_++;

  if (!self.is_enabled && !self.is_ride_enabled) {
    if (self.is_active_) self.Stop();
    return;
  }
  // The flight has the priority over the ride. A change of the mode stops the
  // active one first.
  const bool wants_ride = !self.is_enabled;
  if (self.is_active_ && self.is_riding_ != wants_ride) self.Stop();
  if (!self.is_active_) {
    // No Pokémon that follows. The flight also has no collision and no wild
    // Pokémon.
    self.is_riding_ = wants_ride;
    self.was_following_ = FollowingPokemonSwitch();
    FollowingPokemonSwitch() = false;
    ApplyFollowingPokemon(nullptr);
    if (!wants_ride) {
      self.was_no_collision_ = PlayerCheats::GetInstance().no_collision;
      PlayerCheats::GetInstance().no_collision = true;
      self.was_encounter_mode_ =
          static_cast<u8>(WildEncounter::GetInstance().mode);
      WildEncounter::GetInstance().mode = WildEncounter::Mode::kNever;
    }
    self.is_active_ = true;
  }

  // The game destroys the scene of the overworld in a menu, in a battle and
  // when it loads a map: the model leaves it as the Pokémon that follows does.
  if (!core::ProcessManager::IsOverworldActive()) {
    self.LeaveScene(false);
    self.last_map_ = 0xFFFFFFFF;
    return;
  }
  MapManager* maps = MapManager::GetInstanceOrNull();
  if (maps == nullptr) return;
  if (maps->GetNextMapId() != MapManager::kNoMap) {
    self.LeaveScene(true);
    return;
  }
  const u32 map = static_cast<u32>(maps->GetMap());
  if (map != self.last_map_) {
    self.last_map_ = map;
    self.LeaveScene(false);
    return;
  }
  auto* engine = script::Engine::GetInstance();
  if ((engine != nullptr && engine->IsScriptRunning()) ||
      core::EventManager::GetInstance().GetGameEvent() != nullptr) {
    self.LeaveScene(true);
    return;
  }

  if (!self.model_.IsLoaded()) {
    if (self.frame_ < self.ready_frame_) return;
    if (!self.Spawn()) {
      self.ready_frame_ = self.frame_ + 60;
      return;
    }
  }
  if (self.is_riding_) {
    self.Ride();
  } else {
    self.Fly();
  }
}

} // namespace overworld
