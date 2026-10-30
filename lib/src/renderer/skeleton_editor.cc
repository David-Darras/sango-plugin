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
 * @file skeleton_editor.cc
 * @brief Moves, turns and scales the bones of the 3D models.
 *
 * The declarations are in renderer/patch/skeleton_editor.h.
 */

#include "renderer/patch/skeleton_editor.h"
#include "pokemon/patch/model_loader.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "core/hook.h"
#include "core/ini.h"
#include "renderer/address.h"
#include "renderer/native/h3d_model.h"
#include "system/native/file.h"
#include "system/native/graphics.h"

namespace renderer {

namespace {
core::Hook<void(void*, void*, void*, void*)> mesh_draw_hook;
core::Ini ini;

const c16* const kPath = u"sdmc:/sango/skeleton.ini";

constexpr u32 kMaxBones = 512;

// The state of the skeleton starts at the byte 0x38 of the model. The matrices
// of the bones in the world are an array of 48 bytes (12 floats) for each bone.
// The parents of the bones (16 bits each) are at the byte 0x3C of the model,
// the number of bones is at 0x40, and the matrices at 0x4C.
constexpr u32 kStateOffset = 0x38;
constexpr u32 kParentsOffset = 0x3C;
constexpr u32 kBoneCountOffset = 0x40;
constexpr u32 kWorldMatricesOffset = 0x4C;

// A pointer to an array of the game: aligned, in the heap or in the linear
// memory.
bool IsReadable(uptr address) {
  return (address & 3) == 0 && sys::Graphics::IsDataAddress(address);
}

u32 HashText(const c8* text, u32 hash) {
  for (; *text != '\0'; text++) {
    hash = (hash ^ static_cast<u8>(*text)) * 16777619u;
  }
  return (hash ^ 0xFF) * 16777619u;
}

// The matrix of a change: the scale, then the turn (X, then Y, then Z), then
// the move. 3 rows and 4 columns, as the matrices of the game.
void MakeMatrix(const BoneChange& change, f32 out[12]) {
  const f32 to_radians = 3.14159265f / 180.0f;
  const f32 sx = sinf(change.rotate[0] * to_radians);
  const f32 cx = cosf(change.rotate[0] * to_radians);
  const f32 sy = sinf(change.rotate[1] * to_radians);
  const f32 cy = cosf(change.rotate[1] * to_radians);
  const f32 sz = sinf(change.rotate[2] * to_radians);
  const f32 cz = cosf(change.rotate[2] * to_radians);
  // R = Rz * Ry * Rx
  const f32 r[3][3] = {
      {cz * cy, cz * sy * sx - sz * cx, cz * sy * cx + sz * sx},
      {sz * cy, sz * sy * sx + cz * cx, sz * sy * cx - cz * sx},
      {-sy, cy * sx, cy * cx},
  };
  for (u32 i = 0; i < 3; i++) {
    for (u32 j = 0; j < 3; j++) out[i * 4 + j] = r[i][j] * change.scale[j];
    out[i * 4 + 3] = change.translate[i];
  }
}

// Returns the inverse of a matrix of 3 rows and 4 columns, or false when the
// matrix has no inverse (a scale of 0).
bool Invert(const f32* m, f32* out) {
  const f32 a = m[0], b = m[1], c = m[2];
  const f32 d = m[4], e = m[5], f = m[6];
  const f32 g = m[8], h = m[9], i = m[10];
  const f32 cof_a = e * i - f * h;
  const f32 cof_b = f * g - d * i;
  const f32 cof_c = d * h - e * g;
  const f32 det = a * cof_a + b * cof_b + c * cof_c;
  if (fabsf(det) < 1e-18f) return false;
  const f32 inv = 1.0f / det;
  const f32 r[9] = {cof_a * inv,
                    (c * h - b * i) * inv,
                    (b * f - c * e) * inv,
                    cof_b * inv,
                    (a * i - c * g) * inv,
                    (c * d - a * f) * inv,
                    cof_c * inv,
                    (b * g - a * h) * inv,
                    (a * e - b * d) * inv};
  for (u32 row = 0; row < 3; row++) {
    for (u32 col = 0; col < 3; col++) out[row * 4 + col] = r[row * 3 + col];
    out[row * 4 + 3] = -(r[row * 3 + 0] * m[3] + r[row * 3 + 1] * m[7] +
                         r[row * 3 + 2] * m[11]);
  }
  return true;
}

// out = a * b, for matrices of 3 rows and 4 columns (the 4th row is 0 0 0 1).
void Multiply(const f32* a, const f32* b, f32* out) {
  f32 result[12];
  for (u32 i = 0; i < 3; i++) {
    for (u32 j = 0; j < 4; j++) {
      f32 sum = a[i * 4 + 0] * b[0 * 4 + j] + a[i * 4 + 1] * b[1 * 4 + j] +
                a[i * 4 + 2] * b[2 * 4 + j];
      if (j == 3) sum += a[i * 4 + 3];
      result[i * 4 + j] = sum;
    }
  }
  memcpy(out, result, sizeof(result));
}

struct Vector {
  f32 x, y, z;
};

Vector Subtract(const Vector& a, const Vector& b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}

f32 Length(const Vector& v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

Vector Normalize(const Vector& v) {
  const f32 length = Length(v);
  if (length < 1e-9f) return {0.0f, -1.0f, 0.0f};
  return {v.x / length, v.y / length, v.z / length};
}

Vector BonePosition(const f32* worlds, u32 bone) {
  return {worlds[bone * 12 + 3], worlds[bone * 12 + 7], worlds[bone * 12 + 11]};
}

// The turn (3 rows and 3 columns) that takes the direction @a from to the
// direction @a to. Both are unit vectors.
void MakeTurn(const Vector& from, const Vector& to, f32 out[9]) {
  const f32 dot = from.x * to.x + from.y * to.y + from.z * to.z;
  Vector axis = {from.y * to.z - from.z * to.y, from.z * to.x - from.x * to.z,
                 from.x * to.y - from.y * to.x};
  f32 sine = Length(axis);
  f32 cosine = dot;
  if (sine < 1e-6f) {
    if (dot > 0.0f) {
      const f32 identity[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
      memcpy(out, identity, sizeof(identity));
      return;
    }
    // The opposite direction: a half turn around any direction that is
    // perpendicular.
    axis = fabsf(from.x) < 0.9f ? Vector{0, -from.z, from.y}
                                : Vector{-from.z, 0, from.x};
    axis = Normalize(axis);
    sine = 0.0f;
    cosine = -1.0f;
  } else {
    axis = {axis.x / sine, axis.y / sine, axis.z / sine};
  }
  const f32 t = 1.0f - cosine;
  out[0] = cosine + axis.x * axis.x * t;
  out[1] = axis.x * axis.y * t - axis.z * sine;
  out[2] = axis.x * axis.z * t + axis.y * sine;
  out[3] = axis.y * axis.x * t + axis.z * sine;
  out[4] = cosine + axis.y * axis.y * t;
  out[5] = axis.y * axis.z * t - axis.x * sine;
  out[6] = axis.z * axis.x * t - axis.y * sine;
  out[7] = axis.z * axis.y * t + axis.x * sine;
  out[8] = cosine + axis.z * axis.z * t;
}

// Turns a bone and the bones under it around the place of the bone.
void TurnBranch(f32* worlds, const u16* parents, u32 count, u32 bone,
                const f32 turn[9]) {
  bool is_in_tree[128] = {};
  if (count > 128) return;
  is_in_tree[bone] = true;
  for (u32 pass = 0; pass < 16; pass++) {
    bool is_changed = false;
    for (u32 i = 0; i < count; i++) {
      const u32 parent = parents[i];
      if (!is_in_tree[i] && parent < count && parent != i && is_in_tree[parent]) {
        is_in_tree[i] = true;
        is_changed = true;
      }
    }
    if (!is_changed) break;
  }
  const Vector pivot = BonePosition(worlds, bone);
  for (u32 i = 0; i < count; i++) {
    if (!is_in_tree[i]) continue;
    f32* m = worlds + i * 12;
    f32 result[12];
    const f32 px = m[3] - pivot.x;
    const f32 py = m[7] - pivot.y;
    const f32 pz = m[11] - pivot.z;
    const f32 center[3] = {pivot.x, pivot.y, pivot.z};
    for (u32 row = 0; row < 3; row++) {
      for (u32 col = 0; col < 3; col++) {
        result[row * 4 + col] = turn[row * 3 + 0] * m[0 * 4 + col] +
                                turn[row * 3 + 1] * m[1 * 4 + col] +
                                turn[row * 3 + 2] * m[2 * 4 + col];
      }
      result[row * 4 + 3] = turn[row * 3 + 0] * px + turn[row * 3 + 1] * py +
                            turn[row * 3 + 2] * pz + center[row];
    }
    memcpy(m, result, sizeof(result));
  }
}

// The bones from @a bone up to the root: the length of the path.
u32 GetPath(const u16* parents, u32 count, u32 bone, u16* path) {
  u32 length = 0;
  while (bone < count && length < count) {
    path[length++] = static_cast<u16>(bone);
    const u32 parent = parents[bone];
    if (parent == bone || parent >= count) break;
    bone = parent;
  }
  return length;
}
} // namespace

// A pointer of the character objects to the model, one or two levels deep.
bool SkeletonEditor::IsSeatModel(const H3dModel* model, u32 count) {
  if (!has_seat_pose_ || model == seat_excluded_ || count < 20) return false;
  if (model == seat_model_) return true;
  for (u32 i = 0; i < seat_checked_count_; i++) {
    if (seat_checked_[i] == model) return false;
  }
  const uptr wanted = reinterpret_cast<uptr>(model);
  bool is_found = false;
  for (u32 owner = 0; owner < 2 && !is_found; owner++) {
    const uptr* words = static_cast<const uptr*>(seat_owner_[owner]);
    if (words == nullptr) continue;
    const u32 word_count = seat_owner_size_[owner] / sizeof(uptr);
    for (u32 i = 0; i < word_count && !is_found; i++) {
      if (words[i] == wanted) {
        is_found = true;
        break;
      }
      const uptr inner = words[i];
      if ((inner & 3) != 0 || !sys::Graphics::IsDataAddress(inner) ||
          !sys::Graphics::IsDataAddress(inner + 0x100)) {
        continue;
      }
      const uptr* deep = reinterpret_cast<const uptr*>(inner);
      for (u32 j = 0; j < 64; j++) {
        if (deep[j] == wanted) {
          is_found = true;
          break;
        }
      }
    }
  }
  if (is_found) {
    seat_model_ = model;
  } else if (seat_checked_count_ < 8) {
    seat_checked_[seat_checked_count_++] = model;
  }
  return is_found;
}

bool SkeletonEditor::ApplySeatPose(const H3dModel* model, f32* worlds,
                                   const u16* parents, u32 count) {
  if (count < 8 || count > 128) return false;

  // The legs of the character: a thigh, a lower leg, a foot and the toes, on
  // each side. The names are the ones of the skeleton of the characters.
  static const c8* const kLegNames[2][4] = {
      {"LThigh", "LLeg", "LFoot", "LToe"}, {"RThigh", "RLeg", "RFoot", "RToe"}};
  u32 legs[2][4];
  for (u32 side = 0; side < 2; side++) {
    for (u32 part = 0; part < 4; part++) {
      legs[side][part] = count;
      for (u32 bone = 0; bone < count; bone++) {
        if (strcmp(GetBoneName(model, bone), kLegNames[side][part]) == 0) {
          legs[side][part] = bone;
          break;
        }
      }
      if (legs[side][part] == count) return false;
    }
  }

  // The character turns as the Pokémon does: the game turns the player at
  // once, the Pokémon turns slowly. The direction of the character is the line
  // across the thighs (left to right) turned by a quarter.
  {
    const Vector left_thigh = BonePosition(worlds, legs[0][0]);
    const Vector right_thigh = BonePosition(worlds, legs[1][0]);
    const f32 across_x = left_thigh.x - right_thigh.x;
    const f32 across_z = left_thigh.z - right_thigh.z;
    if (across_x * across_x + across_z * across_z > 1e-6f) {
      const f32 actual = atan2f(-across_z, across_x);
      const f32 wanted = atan2f(seat_forward_[0], seat_forward_[1]);
      f32 turn = wanted - actual;
      if (turn > -100.0f && turn < 100.0f) {
        while (turn > 3.14159265f) turn -= 6.2831853f;
        while (turn < -3.14159265f) turn += 6.2831853f;
        const f32 c = cosf(turn);
        const f32 s = sinf(turn);
        const f32 pivot_x = (left_thigh.x + right_thigh.x) * 0.5f;
        const f32 pivot_z = (left_thigh.z + right_thigh.z) * 0.5f;
        for (u32 i = 0; i < count; i++) {
          f32* m = worlds + i * 12;
          f32 result[12];
          for (u32 col = 0; col < 3; col++) {
            result[0 * 4 + col] = c * m[0 * 4 + col] + s * m[2 * 4 + col];
            result[1 * 4 + col] = m[1 * 4 + col];
            result[2 * 4 + col] = -s * m[0 * 4 + col] + c * m[2 * 4 + col];
          }
          const f32 dx = m[3] - pivot_x;
          const f32 dz = m[11] - pivot_z;
          result[3] = c * dx + s * dz + pivot_x;
          result[7] = m[7];
          result[11] = -s * dx + c * dz + pivot_z;
          memcpy(m, result, sizeof(result));
        }
      }
    }
  }

  // The hips stay at the height of the sit pose: the walk animation makes the
  // body go up and down, and the character would hop on the Pokémon.
  u32 hips = count;
  for (u32 bone = 0; bone < count; bone++) {
    if (strcmp(GetBoneName(model, bone), "Hips") == 0) {
      hips = bone;
      break;
    }
  }
  if (hips < count) {
    const f32 current = worlds[hips * 12 + 7] - seat_ground_y_;
    if (!seat_moving_) {
      seat_rest_height_ = seat_rest_height_ < 0.0f
                              ? current
                              : seat_rest_height_ + (current - seat_rest_height_) * 0.1f;
    } else {
      seat_walk_height_ = seat_walk_height_ < 0.0f
                              ? current
                              : seat_walk_height_ + (current - seat_walk_height_) * 0.1f;
      const f32 wanted = seat_rest_height_ >= 0.0f ? seat_rest_height_
                                                   : seat_walk_height_;
      for (u32 i = 0; i < count; i++) worlds[i * 12 + 7] += wanted - current;
    }
  }

  // The front of the character: the direction of the ride.
  const f32 length = sqrtf(seat_forward_[0] * seat_forward_[0] +
                           seat_forward_[1] * seat_forward_[1]);
  if (length < 1e-6f) return false;
  const Vector forward = {seat_forward_[0] / length, 0.0f,
                          seat_forward_[1] / length};
  const Vector left = BonePosition(worlds, legs[0][0]);
  const Vector right = BonePosition(worlds, legs[1][0]);
  const Vector side_direction =
      Normalize({left.x - right.x, 0.0f, left.z - right.z});

  // The arms: forward, bent, the hands in front of the chest.
  static const c8* const kArmNames[2][3] = {{"LArmA", "LForeArm", "LHand"},
                                            {"RArmA", "RForeArm", "RHand"}};
  for (u32 side = 0; side < 2; side++) {
    u32 arm[3] = {count, count, count};
    for (u32 part = 0; part < 3; part++) {
      for (u32 bone = 0; bone < count; bone++) {
        if (strcmp(GetBoneName(model, bone), kArmNames[side][part]) == 0) {
          arm[part] = bone;
          break;
        }
      }
    }
    if (arm[0] == count || arm[1] == count || arm[2] == count) continue;
    const f32 inward = side == 0 ? -0.12f : 0.12f;
    // The upper arm: forward and down.
    const Vector upper = Normalize(Subtract(BonePosition(worlds, arm[1]),
                                            BonePosition(worlds, arm[0])));
    const Vector upper_wanted = Normalize(
        {forward.x * 0.7f + side_direction.x * inward, -0.55f,
         forward.z * 0.7f + side_direction.z * inward});
    f32 turn[9];
    MakeTurn(upper, upper_wanted, turn);
    TurnBranch(worlds, parents, count, arm[0], turn);
    // The forearm: forward, a little down, the elbow is bent.
    const Vector fore = Normalize(Subtract(BonePosition(worlds, arm[2]),
                                           BonePosition(worlds, arm[1])));
    const Vector fore_wanted = Normalize(
        {forward.x * 0.95f + side_direction.x * inward * 1.5f, -0.2f,
         forward.z * 0.95f + side_direction.z * inward * 1.5f});
    MakeTurn(fore, fore_wanted, turn);
    TurnBranch(worlds, parents, count, arm[1], turn);
  }

  for (u32 side = 0; side < 2; side++) {
    const u32 thigh = legs[side][0];
    const u32 lower = legs[side][1];
    const u32 foot = legs[side][2];
    const f32 splay = side == 0 ? seat_spread_ : -seat_spread_;

    // The thigh goes forward, a little down and a little to the side.
    const Vector now = Normalize(Subtract(BonePosition(worlds, lower),
                                          BonePosition(worlds, thigh)));
    const Vector wanted = Normalize(
        {forward.x * 0.9f + side_direction.x * splay, -0.35f,
         forward.z * 0.9f + side_direction.z * splay});
    f32 turn[9];
    MakeTurn(now, wanted, turn);
    TurnBranch(worlds, parents, count, thigh, turn);

    // The lower leg goes down, a little back.
    const Vector shin = Normalize(Subtract(BonePosition(worlds, foot),
                                           BonePosition(worlds, lower)));
    const Vector down = Normalize({-forward.x * 0.2f, -1.0f, -forward.z * 0.2f});
    MakeTurn(shin, down, turn);
    TurnBranch(worlds, parents, count, lower, turn);
  }
  // The drawn character goes up: the game keeps it on the ground.
  for (u32 i = 0; i < count; i++) worlds[i * 12 + 7] += seat_raise_;
  return true;
}

void SkeletonEditor::SetSeatPose(const void* draw_model, u32 draw_model_size,
                                 const void* character, u32 character_size,
                                 const H3dModel* excluded, f32 forward_x,
                                 f32 forward_z, bool is_moving, f32 raise,
                                 f32 leg_spread, f32 ground_y) {
  auto& self = GetInstance();
  self.seat_ground_y_ = ground_y;
  if (fabsf(leg_spread - self.seat_spread_) > 0.001f) self.version_++;
  self.seat_spread_ = leg_spread;
  if (fabsf(raise - self.seat_raise_) > 0.01f) self.version_++;
  self.seat_raise_ = raise;
  if (fabsf(forward_x - self.seat_forward_[0]) > 0.001f ||
      fabsf(forward_z - self.seat_forward_[1]) > 0.001f ||
      is_moving != self.seat_moving_) {
    self.version_++;
  }
  self.seat_forward_[0] = forward_x;
  self.seat_forward_[1] = forward_z;
  self.seat_moving_ = is_moving;
  if (!self.has_seat_pose_) {
    self.has_seat_pose_ = true;
    self.seat_model_ = nullptr;
    self.seat_checked_count_ = 0;
    self.version_++;
  }
  self.seat_owner_[0] = draw_model;
  self.seat_owner_size_[0] = draw_model_size;
  self.seat_owner_[1] = character;
  self.seat_owner_size_[1] = character_size;
  self.seat_excluded_ = excluded;
}

void SkeletonEditor::ClearSeatPose() {
  auto& self = GetInstance();
  if (!self.has_seat_pose_) return;
  self.has_seat_pose_ = false;
  self.seat_model_ = nullptr;
  self.seat_rest_height_ = -1.0f;
  self.seat_walk_height_ = -1.0f;
  self.version_++;
}

void SkeletonEditor::Initialize() {
#ifdef GAME_ORAS
  mesh_draw_hook.Install(address::kMeshDrawCommand, MeshDrawHook);
  Load();
#endif
}

// The game calls this function to write the draw commands of a mesh. Its third
// argument is the state of the skeleton of the model.
void SkeletonEditor::MeshDrawHook(void* mesh, void* helper, void* state,
                                  void* extra) {
  auto& self = GetInstance();
  // The animation of a Pokémon model goes first: the changes of the editor come after.
  pokemon::ModelLoader::ApplyMotion(
      reinterpret_cast<const void*>(reinterpret_cast<uptr>(state) - kStateOffset));
  if (self.edit_count_ != 0 || self.has_runtime_ || self.has_seat_pose_) {
    self.ApplyToModel(reinterpret_cast<const H3dModel*>(
        reinterpret_cast<uptr>(state) - kStateOffset));
  }
  mesh_draw_hook(mesh, helper, state, extra);
}

f32* SkeletonEditor::GetWorldMatrices(const H3dModel* model, u32* count,
                                      const u16** parents) {
#ifdef GAME_ORAS
  const u32 bones = GetBoneCount(model);
  if (bones == 0) return nullptr;
  const uptr base = reinterpret_cast<uptr>(model);
  const uptr parents_address =
      *reinterpret_cast<const uptr*>(base + kParentsOffset);
  const uptr worlds_address =
      *reinterpret_cast<const uptr*>(base + kWorldMatricesOffset);
  const u32 state_count = *reinterpret_cast<const u32*>(base + kBoneCountOffset);
  if (!IsReadable(worlds_address) || (parents_address & 1) != 0 ||
      !sys::Graphics::IsDataAddress(parents_address) || state_count < bones) {
    return nullptr;
  }
  *count = bones;
  *parents = reinterpret_cast<const u16*>(parents_address);
  return reinterpret_cast<f32*>(worlds_address);
#else
  (void)model;
  (void)count;
  (void)parents;
  return nullptr;
#endif
}

u32 SkeletonEditor::GetBoneCount(const H3dModel* model) {
#ifdef GAME_ORAS
  const s32 count = ((s32(*)(const H3dModel*))address::kH3dModelGetBonesCount)(
      model);
  return count > 0 && count <= static_cast<s32>(kMaxBones)
             ? static_cast<u32>(count)
             : 0;
#else
  (void)model;
  return 0;
#endif
}

const c8* SkeletonEditor::GetBoneName(const H3dModel* model, u32 bone) {
#ifdef GAME_ORAS
  if (bone >= GetBoneCount(model)) return "";
  const c8* name = ((const c8* (*)(const H3dModel*, s32))
                        address::kH3dModelGetBonesName)(model, bone);
  if (name != nullptr &&
      sys::Graphics::IsDataAddress(reinterpret_cast<uptr>(name))) {
    return name;
  }
#else
  (void)model;
  (void)bone;
#endif
  return "";
}

// The hash of the names of all the bones: it tells the skeletons apart. The
// editor keeps it for each model that it saw. 0: the names are not readable.
u32 SkeletonEditor::GetSkeleton(const H3dModel* model) {
  const u32 count = GetBoneCount(model);
  for (ModelInfo& info : models_) {
    if (info.model == model && info.bone_count == count) return info.skeleton;
  }
  u32 hash = 2166136261u;
  u32 readable_names = 0;
  for (u32 bone = 0; bone < count; bone++) {
    const c8* name = GetBoneName(model, bone);
    if (name[0] != '\0') readable_names++;
    hash = HashText(name, hash);
  }
  if (readable_names == 0) hash = 0;
  ModelInfo& slot = models_[next_model_];
  next_model_ = (next_model_ + 1) % kMaxModels;
  slot.model = model;
  slot.skeleton = hash;
  slot.bone_count = count;
  return hash;
}

void SkeletonEditor::OnModelRemoved(const H3dModel* model) {
  auto& self = GetInstance();
  for (ModelInfo& info : self.models_) {
    if (info.model == model) info.model = nullptr;
  }
  for (Runtime& runtime : self.runtimes_) {
    if (runtime.model == model) {
      runtime.model = nullptr;
      runtime.is_valid = false;
    }
  }
}

SkeletonEditor::Edit* SkeletonEditor::Find(u32 skeleton, u32 bone) {
  for (Edit& edit : edits_) {
    if (edit.is_used && edit.skeleton == skeleton && edit.bone == bone) {
      return &edit;
    }
  }
  return nullptr;
}

const BoneChange* SkeletonEditor::GetChange(const H3dModel* model, u32 bone) {
  auto& self = GetInstance();
  const Edit* edit = self.Find(self.GetSkeleton(model), bone);
  return edit != nullptr ? &edit->change : nullptr;
}

bool SkeletonEditor::SetChange(const H3dModel* model, u32 bone,
                               const BoneChange& change) {
  auto& self = GetInstance();
  const u32 skeleton = self.GetSkeleton(model);
  if (skeleton == 0) return false;
  Edit* edit = self.Find(skeleton, bone);
  if (edit == nullptr) {
    for (Edit& slot : self.edits_) {
      if (!slot.is_used) {
        edit = &slot;
        break;
      }
    }
    if (edit == nullptr) return false;
    edit->is_used = true;
    edit->skeleton = skeleton;
    edit->bone = bone;
    strncpy(edit->name, GetBoneName(model, bone), kMaxNameLength - 1);
    edit->name[kMaxNameLength - 1] = '\0';
    self.edit_count_++;
  }
  edit->change = change;
  self.version_++;
  return true;
}

void SkeletonEditor::RemoveChange(const H3dModel* model, u32 bone) {
  auto& self = GetInstance();
  Edit* edit = self.Find(self.GetSkeleton(model), bone);
  if (edit == nullptr) return;
  edit->is_used = false;
  self.edit_count_--;
  self.version_++;
}

void SkeletonEditor::RemoveAll(const H3dModel* model) {
  auto& self = GetInstance();
  const u32 skeleton = self.GetSkeleton(model);
  for (Edit& edit : self.edits_) {
    if (edit.is_used && edit.skeleton == skeleton) {
      edit.is_used = false;
      self.edit_count_--;
    }
  }
  self.version_++;
}

u32 SkeletonEditor::GetChangeCount() { return GetInstance().edit_count_; }

// Gives the changes to the matrices of a model. The game draws a model many
// times in a frame (each mesh, each eye): the editor changes the matrices only
// when the game wrote new ones, or when the changes changed.
void SkeletonEditor::ApplyToModel(const H3dModel* model) {
  const u32 count = GetBoneCount(model);
  if (count == 0 || count > kMaxRuntimeBones) return;
  const uptr base = reinterpret_cast<uptr>(model);
  const uptr parents_address =
      *reinterpret_cast<const uptr*>(base + kParentsOffset);
  const uptr worlds_address =
      *reinterpret_cast<const uptr*>(base + kWorldMatricesOffset);
  const u32 state_count = *reinterpret_cast<const u32*>(base + kBoneCountOffset);
  if (!IsReadable(worlds_address) || (parents_address & 1) != 0 ||
      !sys::Graphics::IsDataAddress(parents_address) || state_count < count) {
    return;
  }
  const u16* parents = reinterpret_cast<const u16*>(parents_address);
  f32* worlds = reinterpret_cast<f32*>(worlds_address);
  const u32 bytes = count * 12 * sizeof(f32);
  const bool is_seat_model = IsSeatModel(model, count);
  // The names of the bones can be unreadable: the pose does not need them.
  const u32 skeleton = GetSkeleton(model);
  if (skeleton == 0 && !is_seat_model) return;
  // A character that has no sit pose and no change stays as the game made it.
  if (edit_count_ == 0 && !is_seat_model && !has_runtime_) return;

  // The memory of this model.
  Runtime* runtime = nullptr;
  for (Runtime& entry : runtimes_) {
    if (entry.model == model) runtime = &entry;
  }
  if (runtime == nullptr) {
    runtime = &runtimes_[next_runtime_];
    next_runtime_ = (next_runtime_ + 1) % kMaxRuntimes;
    runtime->model = model;
    runtime->is_valid = false;
  }

  const bool is_unchanged =
      runtime->is_valid && runtime->bone_count == count &&
      runtime->skeleton == skeleton &&
      memcmp(worlds, runtime->edited, bytes) == 0;
  if (is_unchanged) {
    // The game wrote nothing new: the changes are on the matrices already.
    if (runtime->version == version_) return;
    // The changes changed: start again from the matrices of the game.
    memcpy(worlds, runtime->original, bytes);
  } else {
    memcpy(runtime->original, worlds, bytes);
  }

  for (const Edit& edit : edits_) {
    if (!edit.is_used || edit.skeleton != skeleton || edit.bone >= count) {
      continue;
    }
    // The bone gets world * D. Each bone under it gets W * world, with
    // W = world * D * inverse(world) of the bone: the bones under it follow.
    f32* world = worlds + edit.bone * 12;
    f32 inverse[12];
    if (!Invert(world, inverse)) continue;
    f32 delta[12];
    MakeMatrix(edit.change, delta);
    f32 moved[12];
    Multiply(world, delta, moved);
    f32 correction[12];
    Multiply(moved, inverse, correction);

    // The bones under this bone: a bone is in the tree when its parent is.
    bool is_in_tree[kMaxRuntimeBones] = {};
    is_in_tree[edit.bone] = true;
    for (u32 pass = 0; pass < 4; pass++) {
      for (u32 i = 0; i < count; i++) {
        const u32 parent = parents[i];
        if (!is_in_tree[i] && parent < count && parent != i &&
            is_in_tree[parent]) {
          is_in_tree[i] = true;
        }
      }
    }
    for (u32 i = 0; i < count; i++) {
      if (is_in_tree[i]) Multiply(correction, worlds + i * 12, worlds + i * 12);
    }
  }

  if (is_seat_model) ApplySeatPose(model, worlds, parents, count);

  memcpy(runtime->edited, worlds, bytes);
  runtime->is_valid = true;
  runtime->bone_count = count;
  runtime->skeleton = skeleton;
  runtime->version = version_;
  has_runtime_ = true;
}

bool SkeletonEditor::Save() {
  auto& self = GetInstance();
  ini.Clear();
  ini.AddSection("skeleton");
  ini.AddComment("skeleton hash|bone index|bone name|translate x y z, "
                 "rotate x y z (degrees), scale x y z");
  u32 number = 0;
  for (const Edit& edit : self.edits_) {
    if (!edit.is_used) continue;
    c8 key[16];
    c8 value[200];
    snprintf(key, sizeof(key), "bone%lu", (unsigned long)++number);
    const BoneChange& c = edit.change;
    snprintf(value, sizeof(value),
             "%08lX|%lu|%s|%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f",
             (unsigned long)edit.skeleton, (unsigned long)edit.bone, edit.name,
             (double)c.translate[0], (double)c.translate[1],
             (double)c.translate[2], (double)c.rotate[0], (double)c.rotate[1],
             (double)c.rotate[2], (double)c.scale[0], (double)c.scale[1],
             (double)c.scale[2]);
    ini.Set(key, value);
  }
  sys::File::CreateDirectory(u"sdmc:/sango");
  return ini.Save(kPath);
}

bool SkeletonEditor::Load() {
  auto& self = GetInstance();
  if (!ini.Load(kPath)) return false;
  for (u32 number = 1; number <= kMaxEdits; number++) {
    c8 key[16];
    snprintf(key, sizeof(key), "bone%lu", (unsigned long)number);
    const c8* value = ini.Get("skeleton", key);
    if (value == nullptr) continue;

    // "hash|index|name|values"
    c8 line[200];
    strncpy(line, value, sizeof(line) - 1);
    line[sizeof(line) - 1] = '\0';
    c8* index = strchr(line, '|');
    if (index == nullptr) continue;
    *index++ = '\0';
    c8* name = strchr(index, '|');
    if (name == nullptr) continue;
    *name++ = '\0';
    c8* values = strchr(name, '|');
    if (values == nullptr) continue;
    *values++ = '\0';

    BoneChange change;
    f32* numbers[9] = {&change.translate[0], &change.translate[1],
                       &change.translate[2], &change.rotate[0],
                       &change.rotate[1],    &change.rotate[2],
                       &change.scale[0],     &change.scale[1],
                       &change.scale[2]};
    c8* cursor = values;
    bool is_valid = true;
    for (u32 i = 0; i < 9; i++) {
      c8* end = nullptr;
      *numbers[i] = strtof(cursor, &end);
      if (end == cursor) {
        is_valid = false;
        break;
      }
      cursor = *end == ',' ? end + 1 : end;
    }
    if (!is_valid) continue;

    for (Edit& slot : self.edits_) {
      if (slot.is_used) continue;
      slot.is_used = true;
      slot.skeleton = strtoul(line, nullptr, 16);
      slot.bone = strtoul(index, nullptr, 10);
      strncpy(slot.name, name, kMaxNameLength - 1);
      slot.name[kMaxNameLength - 1] = '\0';
      slot.change = change;
      self.edit_count_++;
      break;
    }
  }
  return true;
}

} // namespace renderer
