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

#include <type_traits>

#include "common.h"
#include "renderer/constant/texture_filter.h"
#include "renderer/native/particle.h"

namespace renderer {
struct H3dModel;

struct ModelFilterSettings {
  TextureFilter filter = TextureFilter::kDarken;
  u8 mesh = 0;
};
static_assert(std::is_standard_layout<ModelFilterSettings>::value,
              "ModelFilterSettings must have standard layout");

struct ModelFilter : public ModelFilterSettings {
  MAKE_SINGLETON(ModelFilter)

  static void Initialize();
  static bool IsInBattle();
  static bool IsBattlePokemonModel(const H3dModel* h3d_model);
  static bool IsBattleTrainerModel(const H3dModel* h3d_model);
  static bool IsBattlePlatformModel(const H3dModel* h3d_model);
  static bool IsBattleEnvironmentModel(const H3dModel* h3d_model);
  static void PatchWeatherParticleColor(uptr raw);
  static void OnParticleCreate(Particle* self, uptr heap_alloc,
                               uptr device_alloc, uptr resource, uptr desc);
  static void OnResourceAttachBufferAndSetup(uptr self, uptr allocator,
                                             uptr p_buff, uptr desc,
                                             u32 flag_a, u32 flag_b);
  static void UpdateH3dModel(H3dModel* h3d_model);
  static void AddH3dModel(uptr scene, H3dModel* model, u32 idx);
};
} // namespace renderer
