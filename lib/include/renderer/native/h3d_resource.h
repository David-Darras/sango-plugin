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
 * @file h3d_resource.h
 * @brief A 3D resource of the game: the data of models and textures.
 */

#pragma once

#include "common.h"

namespace renderer {

/// A 3D resource: the loaded data of models and textures.
struct H3dResource {
  static constexpr u32 kSize = 76;
  static constexpr u32 kIsSetup = 1 << 2;
  static constexpr u32 kCommonNone = 9;
  static constexpr u32 kCommonDefaultShader = 1;

  INLINE void* GetBuffer() const { return ReadFromEnd<void*>(16); }
  INLINE void* GetResource() const { return ReadFromEnd<void*>(12); }
  INLINE u32 GetFlags() const { return ReadFromEnd<u32>(8); }
  INLINE u32 GetState() const { return ReadFromEnd<u32>(4); }

  INLINE bool IsSetup() const { return (GetFlags() & kIsSetup) != 0; }

  /// Returns true when the resource has data and is set up.
  INLINE bool IsValid() const {
    if (((uptr)this & 3) != 0) return false;
    return GetBuffer() != nullptr && GetResource() != nullptr && IsSetup();
  }

  INLINE void SetData(void* data) {
    ((void (*)(H3dResource*, void*))address::kH3dResourceSetData)(this, data);
  }

  INLINE bool Setup(void* heap_allocator, void* device_allocator,
                    H3dResource* other = nullptr, u32 common = kCommonNone) {
    return ((bool (*)(H3dResource*, void*, void*, H3dResource*, u32))
      address::kH3dResourceSetup)(this, heap_allocator, device_allocator, other,
                            common);
  }

  INLINE void RemoveData(bool uninitialize = true) {
    ((void (*)(H3dResource*, u32))address::kH3dResourceRemoveData)(
        this, uninitialize);
  }

  /// Makes an empty resource in a heap.
  STATIC_INLINE H3dResource* Create(void* heap) {
    auto* resource = (H3dResource*)((void* (*)(void*, u32, u32))
      sys::address::kHeapAlloc)(heap, kSize, 4);
    if (resource != nullptr) {
      ((void (*)(H3dResource*))address::kH3dResourceInitialize)(resource);
    }
    return resource;
  }

private:
  template <typename T>
  INLINE T ReadFromEnd(u32 offset_from_end) const {
    return *(T*)((uptr)this + kSize - offset_from_end);
  }
};
} // namespace renderer
