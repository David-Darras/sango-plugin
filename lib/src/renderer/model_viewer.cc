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
 * @file model_viewer.cc
 * @brief Lists the 3D models of the screen, and changes their parts and their
 *        materials.
 *
 * The declarations are in renderer/patch/model_viewer.h.
 */

#include "renderer/patch/model_viewer.h"

#include "core/hook.h"
#include "renderer/address.h"
#include "renderer/native/h3d_model.h"
#include "renderer/patch/skeleton_editor.h"
#include "system/native/graphics.h"

namespace renderer {

namespace {
core::Hook<void(uptr, H3dModel*)> remove_model_hook;

// The fields of a model that the game functions do not give: the array of the
// states of the materials (a state has 372 bytes), and the array of the
// colors (340 bytes for each material, the ambient and diffuse colors are
// packed as four bytes).
constexpr u32 kMaterialStatesOffset = 0x6C;
constexpr u32 kMaterialStateSize = 372;
constexpr u32 kCullingOffset = 0x114;
constexpr u32 kAlphaTestOffset = 0xE8;
constexpr u32 kMaterialColorsOffset = 0x30;
constexpr u32 kMaterialColorsSize = 340;
constexpr u32 kAmbientOffset = 0x40;
constexpr u32 kDiffuseOffset = 0x44;
constexpr u32 kVisibleFlagsOffset = 0xE4;
constexpr u32 kTextureTableOffset = 0x80;
// The texture coordinates: in the state of each material (340 bytes at the byte
// 0x30 of the model), each unit has a block of 24 bytes from the byte 0x54. The
// scale (2 floats), the turn (1 float, in radians) and the move (2 floats)
// are in this block.
constexpr u32 kUvBlockOffset = 0x54;
constexpr u32 kUvBlockSize = 24;
constexpr u32 kUvScaleOffset = 0x98;
constexpr u32 kUvRotateOffset = 0xA0;
constexpr u32 kUvTranslateOffset = 0xA4;

template <typename T>
T* At(const H3dModel* model, u32 offset) {
  return reinterpret_cast<T*>(reinterpret_cast<uptr>(model) + offset);
}

Color UnpackColor(u32 packed) {
  return Color((packed & 0xFF) / 255.0f, ((packed >> 8) & 0xFF) / 255.0f,
               ((packed >> 16) & 0xFF) / 255.0f,
               ((packed >> 24) & 0xFF) / 255.0f);
}

bool IsReadableArray(uptr address) {
  return (address & 3) == 0 && sys::Graphics::IsDataAddress(address);
}
} // namespace

void ModelViewer::Initialize() {
#ifdef GAME_ORAS
  remove_model_hook.Install(address::kSceneRemoveModel, RemoveHook);
#endif
}

void ModelViewer::RemoveHook(uptr scene, H3dModel* model) {
  auto& self = GetInstance();
  for (H3dModel*& slot : self.models_) {
    if (slot == model) slot = nullptr;
  }
  SkeletonEditor::OnModelRemoved(model);
  remove_model_hook(scene, model);
}

void ModelViewer::OnModelAdded(H3dModel* model) {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  for (H3dModel* slot : self.models_) {
    if (slot == model) return;
  }
  for (H3dModel*& slot : self.models_) {
    if (slot == nullptr) {
      slot = model;
      return;
    }
  }
  // The list is full: the oldest model leaves.
  self.models_[self.next_slot_] = model;
  self.next_slot_ = (self.next_slot_ + 1) % kMaxModels;
#else
  (void)model;
#endif
}

u32 ModelViewer::GetModels(H3dModel** out, u32 max) {
  auto& self = GetInstance();
  u32 count = 0;
  for (H3dModel* model : self.models_) {
    if (model != nullptr && count < max) out[count++] = model;
  }
  return count;
}

bool ModelViewer::IsKnown(const H3dModel* model) {
  if (model == nullptr) return false;
  for (const H3dModel* slot : GetInstance().models_) {
    if (slot == model) return true;
  }
  return false;
}

const c8* ModelViewer::GetKindName(const H3dModel* model) {
  const u32 vtable = *At<u32>(model, 0);
  if (vtable == address::kH3dPokemonShaderModelVtable) return "Pokemon Model";
  if (vtable == address::kH3dShaderModelVtable) return "Shader Model";
  return "Model";
}

void ModelViewer::GetPosition(const H3dModel* model, f32 out[3]) {
#ifdef GAME_ORAS
  Vec3 position = {};
  ((void (*)(const H3dModel*, Vec3*))address::kH3dModelGetTranslate)(
      model, &position);
  out[0] = position.x;
  out[1] = position.y;
  out[2] = position.z;
#else
  out[0] = out[1] = out[2] = 0.0f;
#endif
}

bool ModelViewer::IsVisible(const H3dModel* model) {
  return (*At<u32>(model, kVisibleFlagsOffset) & 1) != 0;
}

void ModelViewer::SetVisible(H3dModel* model, bool is_visible) {
#ifdef GAME_ORAS
  ((void (*)(H3dModel*, s32))address::kH3dModelSetVisible)(model, is_visible);
#else
  (void)model;
  (void)is_visible;
#endif
}

u32 ModelViewer::GetPartCount(const H3dModel* model) {
#ifdef GAME_ORAS
  const s32 count =
      ((s32(*)(const H3dModel*))address::kH3dModelGetMeshNodeCount)(model);
  return count > 0 && count < 512 ? static_cast<u32>(count) : 0;
#else
  (void)model;
  return 0;
#endif
}

const c8* ModelViewer::GetPartName(const H3dModel* model, u32 index) {
#ifdef GAME_ORAS
  const c8* name = ((const c8* (*)(const H3dModel*, s32))
                        address::kH3dModelGetMeshNodeName)(model, index);
  if (name != nullptr && sys::Graphics::IsDataAddress(
                             reinterpret_cast<uptr>(name))) {
    return name;
  }
#else
  (void)model;
  (void)index;
#endif
  return "";
}

bool ModelViewer::IsPartVisible(const H3dModel* model, u32 index) {
#ifdef GAME_ORAS
  return ((bool (*)(const H3dModel*, s32))address::kH3dModelIsMeshNodeVisible)(
      model, index);
#else
  (void)model;
  (void)index;
  return true;
#endif
}

void ModelViewer::SetPartVisible(H3dModel* model, u32 index, bool is_visible) {
#ifdef GAME_ORAS
  ((void (*)(H3dModel*, s32, s32))address::kH3dModelSetMeshNodeVisible)(
      model, index, is_visible);
#else
  (void)model;
  (void)index;
  (void)is_visible;
#endif
}

u32 ModelViewer::GetMaterialCount(const H3dModel* model) {
  const s32 count = const_cast<H3dModel*>(model)->GetMaterialCount();
  return count > 0 && count < 512 ? static_cast<u32>(count) : 0;
}

const c8* ModelViewer::GetMaterialName(const H3dModel* model, u32 index) {
#ifdef GAME_ORAS
  const c8* name = ((const c8* (*)(const H3dModel*, s32))
                        address::kH3dModelGetMaterialName)(model, index);
  if (name != nullptr && sys::Graphics::IsDataAddress(
                             reinterpret_cast<uptr>(name))) {
    return name;
  }
#else
  (void)model;
  (void)index;
#endif
  return "";
}

MaterialState ModelViewer::ReadMaterial(const H3dModel* model, u32 index) {
  MaterialState state;
  if (index >= GetMaterialCount(model)) return state;
  const uptr states = *At<uptr>(model, kMaterialStatesOffset);
  if (IsReadableArray(states)) {
    const uptr material = states + index * kMaterialStateSize;
    state.culling = *reinterpret_cast<const u8*>(material + kCullingOffset);
    state.has_alpha_test =
        *reinterpret_cast<const u8*>(material + kAlphaTestOffset) != 0;
  }
  const uptr colors = *At<uptr>(model, kMaterialColorsOffset);
  if (IsReadableArray(colors)) {
    const uptr material = colors + index * kMaterialColorsSize;
    state.ambient = UnpackColor(*reinterpret_cast<const u32*>(
        material + kAmbientOffset));
    state.diffuse = UnpackColor(*reinterpret_cast<const u32*>(
        material + kDiffuseOffset));
  }
  return state;
}

bool ModelViewer::ReadTextureUnit(const H3dModel* model, u32 material,
                                  u32 unit, TextureUnitInfo& out) {
  out = TextureUnitInfo();
  if (unit >= 3 || material >= GetMaterialCount(model)) return false;
  // The table of the textures: 16 bytes for each material. A word with one
  // byte for each unit tells the kind of the unit, then come three pointers.
  const uptr table = *At<uptr>(model, kTextureTableOffset);
  if (!IsReadableArray(table)) return false;
  const uptr row = table + material * 16;
  out.kind = static_cast<u8>(*reinterpret_cast<const u32*>(row) >> (8 * unit));
  out.object = *reinterpret_cast<const uptr*>(row + 4 + 4 * unit);
  if (out.kind == 0 || !IsReadableArray(out.object)) return true;
  for (u32 i = 0; i < 10; i++) {
    out.words[i] = reinterpret_cast<const u32*>(out.object)[i];
  }
  return true;
}

UvTransform ModelViewer::ReadUv(const H3dModel* model, u32 material, u32 unit) {
  UvTransform uv;
  if (unit >= 3 || material >= GetMaterialCount(model)) return uv;
  const uptr states = *At<uptr>(model, kMaterialColorsOffset);
  if (!IsReadableArray(states + material * kMaterialColorsSize)) return uv;
  const uptr block = states + material * kMaterialColorsSize + kUvBlockOffset +
                     unit * kUvBlockSize;
  const f32* scale = reinterpret_cast<const f32*>(block + kUvScaleOffset);
  const f32 rotate = *reinterpret_cast<const f32*>(block + kUvRotateOffset);
  const f32* translate =
      reinterpret_cast<const f32*>(block + kUvTranslateOffset);
  uv.scale[0] = scale[0];
  uv.scale[1] = scale[1];
  uv.rotate = rotate * (180.0f / 3.14159265f);
  uv.translate[0] = translate[0];
  uv.translate[1] = translate[1];
  return uv;
}

void ModelViewer::WriteUv(H3dModel* model, u32 material, u32 unit,
                          const UvTransform& uv) {
#ifdef GAME_ORAS
  if (unit >= 3 || material >= GetMaterialCount(model)) return;
  const f32 scale[2] = {uv.scale[0], uv.scale[1]};
  const f32 translate[2] = {uv.translate[0], uv.translate[1]};
  ((void (*)(H3dModel*, s32, s32, const f32*))address::kH3dModelSetUvScale)(
      model, material, unit, scale);
  ((void (*)(H3dModel*, s32, s32, f32))address::kH3dModelSetUvRotate)(
      model, material, unit, uv.rotate * (3.14159265f / 180.0f));
  ((void (*)(H3dModel*, s32, s32, const f32*))address::kH3dModelSetUvTranslate)(
      model, material, unit, translate);
#else
  (void)model;
  (void)material;
  (void)unit;
  (void)uv;
#endif
}

void ModelViewer::WriteMaterial(H3dModel* model, u32 index,
                                const MaterialState& state) {
#ifdef GAME_ORAS
  if (index >= GetMaterialCount(model)) return;
  auto set_ambient = (void (*)(H3dModel*, s32, const Color&))
      address::kH3dModelSetMaterialColorAmbient;
  auto set_diffuse = (void (*)(H3dModel*, s32, const Color&))
      address::kH3dModelSetMaterialColorDiffuse;
  auto set_culling =
      (void (*)(H3dModel*, s32, s32))address::kH3dModelSetCullingMode;
  set_ambient(model, index, state.ambient);
  set_diffuse(model, index, state.diffuse);
  set_culling(model, index, state.culling);
  model->SetAlphaTestEnable(index, state.has_alpha_test);
#else
  (void)model;
  (void)index;
  (void)state;
#endif
}

} // namespace renderer
