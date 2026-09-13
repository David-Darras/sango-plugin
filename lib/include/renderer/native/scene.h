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
#include "core/hook_manager.h"
#include "renderer/native/h3d_shader_model.h"

namespace renderer {
struct Scene {
  STATIC_INLINE bool Register0(void* scene, H3dShaderModel* model,
                               s32 x = 0) {
    return core::HookManager::GetInstance().Get(HookId::kSceneRegister0)
                                     ->CallOriginal<bool>(scene, model, x);
  }

  STATIC_INLINE bool Register1(void* scene, H3dShaderModel* model,
                               s32 x = 0) {
    return ((bool (*)(void*, H3dShaderModel*, s32))
      address::kSceneRegister1)(scene, model, x);
  }

  STATIC_INLINE void Unregister(void* scene, H3dShaderModel* model) {
    ((void (*)(void*, H3dShaderModel*))address::kSceneUnregister)(scene,
      model);
  }
};
} // namespace renderer