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
 * @file h3d_shader_model.h
 * @brief A 3D model with its shaders.
 */

#pragma once

#include "common.h"
#include "renderer/native/h3d_resource.h"
#include "renderer/address.h"
#include "system/address.h"

namespace renderer {
/// A 3D model with its shaders. The plugin uses it to show models in the overworld.
struct H3dShaderModel {
  u8 _0[0xE8];
  Vec3 scale;
  Vec3 rotation;
  u8 _1[0x30];
  Vec3 position;

  static constexpr u32 kSize = 380;

  struct Metadata {
    u32 anim_count;
    u32 _0;
    u32 _1;
    u32 _2;
  };

  STATIC_INLINE Metadata DefaultMetadata() {
    return Metadata{1, 1, 8, 0};
  }

  STATIC_INLINE H3dShaderModel* Create(void* heap) {
    auto* model = (H3dShaderModel*)core::GameFunction<void*(void*, u32,
                                                            u32)>::Call(
        sys::address::kHeapAlloc, heap, kSize, 4);
    if (model == nullptr) return nullptr;
    core::GameFunction<void(H3dShaderModel*)>::Call(
        address::kH3dShaderModelInitialize, model);
    if (*(u32*)model != address::kH3dShaderModelVtable) return nullptr;
    return model;
  }

  INLINE void Build(void* heap_allocator, void* device_allocator,
                    H3dResource* resource, s32 index_in_resource,
                    Metadata* description) {
    core::GameFunction<void(H3dShaderModel*, void*, void*, H3dResource*, s32,
                            Metadata*)>::Call(
        address::kH3dShaderModelCreate, this, heap_allocator, device_allocator,
        resource, index_in_resource, description);
  }

  INLINE s32 GetMaterialCount() {
    return core::GameFunction<s32(H3dShaderModel*)>::Call(
        address::kH3dModelGetMaterialCount, this);
  }

  INLINE void SetTranslate(const Vec3& translate) {
    core::GameFunction<void(H3dShaderModel*, const Vec3*)>::Call(
        address::kH3dModelSetTranslate, this, &translate);
  }

  INLINE Vec3 GetTranslate() {
    Vec3 translate = {};
    core::GameFunction<void(H3dShaderModel*, Vec3*)>::Call(
        address::kH3dModelGetTranslate, this, &translate);
    return translate;
  }

  INLINE void SetRotate(const Vec3& rot) {
    core::GameFunction<void(H3dShaderModel*, const Vec3*)>::Call(
        address::kH3dModelSetRotate, this, &rot);
  }

  INLINE void SetScale(f32 x, f32 y, f32 z) {
    core::GameFunction<void(H3dShaderModel*, f32, f32, f32)>::Call(
        address::kH3dModelSetScale, this, x, y, z);
  }

  INLINE void SetShaderSymbol(const c8* name, const Color& value) {
    const s32 material_count = GetMaterialCount();
    for (s32 i = 0; i < material_count; ++i) {
      core::GameFunction<void(H3dShaderModel*, s32, const c8*,
                              const Color&)>::Call(
          address::kH3dModelSetShaderSymbol, this, i, name, value);
    }
  }

  INLINE void ApplyOverworldNpcDefaults() {
    SetShaderSymbol("MirrorMatrix0", Color{1.f, 0.f, 0.f, 0.f});
    SetShaderSymbol("MirrorMatrix1", Color{0.f, 1.f, 0.f, 0.f});
    SetShaderSymbol("MirrorMatrix2", Color{0.f, 0.f, 1.f, 0.f});
    SetShaderSymbol("MirrorEnable", Color{1.f, 1.f, 1.f, 1.f});
  }

  INLINE void Destroy() {
    core::GameFunction<void(H3dShaderModel*)>::Call(
        address::kH3dModelDestroy, this);
  }
};
} // namespace renderer