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
#include "feature/hook_manager.h"
#include "game/renderer/h3d_shader_model.h"

namespace renderer {
struct Scene {
  STATIC_INLINE bool Register0(void* scene, H3dShaderModel* model,
                               s32 x = 0) {
    return HookManager::GetInstance().Get(HookID::kSceneRegister0)
                                     ->CallOriginal<bool>(scene, model, x);
  }

  STATIC_INLINE bool Register1(void* scene, H3dShaderModel* model,
                               s32 x = 0) {
    return ((bool (*)(void*, H3dShaderModel*, s32))
      ADDRESS_SCENE_REGISTER_1)(scene, model, x);
  }

  STATIC_INLINE void Unregister(void* scene, H3dShaderModel* model) {
    ((void (*)(void*, H3dShaderModel*))ADDRESS_SCENE_UNREGISTER)(scene,
      model);
  }
};
} // namespace renderer