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
#include "renderer/native/h3d_model.h"

namespace renderer {

struct Particle {
  uptr vtable;

  INLINE s32 GetModelCount() {
    return ((s32(*)(Particle*))
      address::kParticleGetModelCount)(this);
  }

  INLINE H3dModel* GetModel(s32 index) {
    return ((H3dModel*(*)(Particle*, s32))
      address::kParticleGetModel)(this, index);
  }

  INLINE void SetScale(f32 x, f32 y, f32 z) {
    return ((void(*)(Particle*, f32, f32, f32))
      address::kParticleSetScale)(this, x, y, z);
  }
};

} // namespace renderer
