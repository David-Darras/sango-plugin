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
} // namespace

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
  if (self.edit_count_ != 0 || self.has_runtime_) {
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
  const u32 skeleton = GetSkeleton(model);
  if (skeleton == 0) return;
  const u16* parents = reinterpret_cast<const u16*>(parents_address);
  f32* worlds = reinterpret_cast<f32*>(worlds_address);
  const u32 bytes = count * 12 * sizeof(f32);

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
