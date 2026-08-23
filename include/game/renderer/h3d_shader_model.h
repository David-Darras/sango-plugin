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

#pragma once

#include "common.h"
#include "h3d_model.h"
#include "game/renderer/h3d_resource.h"

namespace renderer {
struct H3dShaderModel {
  u8 _0[0xE8];
  Vec3 scale;
  Vec3 rotation;
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
    auto* model = (H3dShaderModel*)((void* (*)(void*, u32, u32))
      ADDRESS_HEAP_ALLOC)(heap, kSize, 4);
    if (model == nullptr) return nullptr;
    ((void (*)(H3dShaderModel*))ADDRESS_H3D_SHADER_MODEL_INITIALIZE)(model);
    if (*(u32*)model != H3D_SHADER_MODEL_VTABLE) return nullptr;
    return model;
  }

  INLINE void Build(void* heap_allocator, void* device_allocator,
                    H3dResource* resource, s32 index_in_resource,
                    Metadata* description) {
    ((void (*)(H3dShaderModel*, void*, void*, H3dResource*, s32, Metadata*))
      ADDRESS_H3D_SHADER_MODEL_CREATE)(this, heap_allocator, device_allocator,
                                       resource, index_in_resource,
                                       description);
  }

  INLINE s32 GetMaterialCount() {
    return ((s32 (*)(H3dShaderModel*))ADDRESS_H3D_MODEL_GET_MATERIAL_COUNT)(
        this);
  }

  INLINE void SetTranslate(const Vec3& translate) {
    ((void (*)(H3dShaderModel*, const Vec3*))ADDRESS_H3D_MODEL_SET_TRANSLATE)(
        this, &translate);
  }

  INLINE Vec3 GetTranslate() {
    Vec3 translate = {};
    ((void (*)(H3dShaderModel*, Vec3*))ADDRESS_H3D_MODEL_GET_TRANSLATE)(
        this, &translate);
    return translate;
  }

  INLINE void SetRotate(const Vec3& rot) {
    ((void (*)(H3dShaderModel*, const Vec3*))ADDRESS_H3D_MODEL_SET_ROTATE)(
        this, &rot);
  }

  INLINE void SetScale(f32 x, f32 y, f32 z) {
    ((void (*)(H3dShaderModel*, f32, f32, f32))ADDRESS_H3D_MODEL_SET_SCALE)(
        this, x, y, z);
  }

  INLINE void SetShaderSymbol(const c8* name, const Color& value) {
    const s32 material_count = GetMaterialCount();
    for (s32 i = 0; i < material_count; ++i) {
      ((void (*)(H3dShaderModel*, s32, const c8*, const Color&))
        ADDRESS_H3D_MODEL_SET_SHADER_SYMBOL)(this, i, name, value);
    }
  }

  INLINE void ApplyOverworldNpcDefaults() {
    SetShaderSymbol("MirrorMatrix0", Color{1.f, 0.f, 0.f, 0.f});
    SetShaderSymbol("MirrorMatrix1", Color{0.f, 1.f, 0.f, 0.f});
    SetShaderSymbol("MirrorMatrix2", Color{0.f, 0.f, 1.f, 0.f});
    SetShaderSymbol("MirrorEnable", Color{1.f, 1.f, 1.f, 1.f});
  }

  INLINE void Destroy() {
    ((void (*)(H3dShaderModel*))ADDRESS_H3D_MODEL_DESTROY)(this);
  }
};
} // namespace renderer