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

#include "feature/core/hook_manager.h"
#include "feature/weather_manager.h"
#include "game/battle/config.h"
#include "game/battle/manager.h"
#include "game/renderer/h3d_model.h"

namespace feature {
using namespace TevCombine;

struct H3dModel {
  MAKE_SINGLETON(H3dModel)
  u8 filter = 3;
  u8 mesh = 0;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kSceneRegister0,
                            ADDRESS_SCENE_REGISTER_0,
                            (uptr)AddH3dModel);
    HookManager::Initialize(HookID::kParticleCreate,
                            ADDRESS_PARTICLE_CREATE,
                            (uptr)OnParticleCreate);
    HookManager::Initialize(HookID::kResourceAttachBufferAndSetup,
                            ADDRESS_RESOURCE_ATTACH_BUFFER_AND_SETUP,
                            (uptr)OnResourceAttachBufferAndSetup);
  }

  static bool IsInBattle() {
    return game::ProcessManager::GetInstance().IsCurrentProcess(
        ADDRESS_BATTLE_VTABLE);
  }

  static bool IsBattlePokemonModel(const renderer::H3dModel* h3d_model) {
    auto& grp = battle::Graphics::GetInstance();
    for (u32 i = 0; i < 6; i++) {
      auto* battle_model = grp.pokemon_model[i];
      if (battle_model == nullptr) continue;
      if (battle_model->h3d_model == h3d_model) return true;
    }
    return false;
  }

  static bool IsBattleTrainerModel(const renderer::H3dModel* h3d_model) {
    auto& grp = battle::Graphics::GetInstance();
    for (u32 i = 0; i < 4; i++) {
      auto* battle_model = grp.trainer_model[i];
      if (battle_model == nullptr) continue;
      if (battle_model->h3d_model == h3d_model) return true;
    }
    return false;
  }

  static bool IsBattlePlatformModel(const renderer::H3dModel* h3d_model) {
    auto& grp = battle::Graphics::GetInstance();
    for (u32 i = 0; i < 6; i++) {
      auto* battle_model = grp.platform_model[i];
      if (battle_model == nullptr) continue;
      if (battle_model->h3d_model == h3d_model) return true;
    }
    for (u32 i = 0; i < 4; i++) {
      auto* battle_model = grp.big_platform_model[i];
      if (battle_model == nullptr) continue;
      if (battle_model->h3d_model == h3d_model) return true;
    }
    if (grp.prop_model != nullptr && grp.prop_model->h3d_model == h3d_model)
      return true;
    if (grp.unknow != nullptr && grp.unknow->h3d_model == h3d_model)
      return true;
    return false;
  }

  static bool IsBattleEnvironnementModel(const renderer::H3dModel* h3d_model) {
    auto& grp = battle::Graphics::GetInstance();
    return grp.environnement->unknow_0 == h3d_model
           || grp.environnement->unknow_1 == h3d_model
           || grp.environnement->landscape == h3d_model
           || grp.environnement->ground == h3d_model;
  }

  static void PatchWeatherParticleColor(uptr raw) {
    auto& ctx = feature::WeatherManager::GetInstance();
    if (ctx.mode != WeatherMode::kToxic && ctx.mode !=
        WeatherMode::kRadioactive)
      return;

    u32 file_size = READ32(raw + 12);
    if (file_size == 0 || file_size > 200000) return;
    uptr end = raw + file_size;
    auto in_bounds = [&](uptr addr, u32 size) {
      return addr >= raw && addr + size <= end;
    };

    uptr tex_dic_field = raw + 40;
    if (!in_bounds(tex_dic_field, 4)) return;
    uptr tex_dict_base = tex_dic_field + (s32)READ32(tex_dic_field);
    uptr tex_node_field = tex_dict_base + 12 + 16 * 1 + 12;
    if (!in_bounds(tex_node_field, 4)) return;
    uptr texture_ptr = tex_node_field + (s32)READ32(tex_node_field);
    if (!in_bounds(texture_ptr, 60)) return;

    s32 tex_w = (s32)READ32(texture_ptr + 28);
    s32 tex_h = (s32)READ32(texture_ptr + 24);
    u32 tex_fmt = READ32(texture_ptr + 52);

    if (tex_fmt != 5 || tex_w <= 0 || tex_w > 128 || tex_h <= 0 || tex_h > 128)
      return;

    uptr image_field = texture_ptr + 56;
    if (!in_bounds(image_field, 4)) return;
    uptr image_data_ptr = image_field + (s32)READ32(image_field);
    if (!in_bounds(image_data_ptr, 32)) return;
    uptr location_address = READ32(image_data_ptr + 24);
    uptr pixels;
    if (location_address != 0) {
      return;
    } else {
      uptr table_field = image_data_ptr + 12;
      if (!in_bounds(table_field, 4)) return;
      uptr table_base = table_field + (s32)READ32(table_field);
      if (!in_bounds(table_base, 4)) return;
      pixels = table_base + (s32)READ32(table_base);
    }

    s32 texel_count = tex_w * tex_h;
    if (!in_bounds(pixels, texel_count * 2u)) return;

    u16 color565 = ctx.mode == WeatherMode::kToxic
                     ? 0x90BBu
                     : 0x97E0u;

    WRITE32(texture_ptr + 52, 3u); // RGB565
    for (s32 i = 0; i < texel_count; i++) {
      WRITE16(pixels + 2 * i, color565);
    }
  }

  static void OnParticleCreate(renderer::Particle* self, uptr heap_alloc,
                               uptr device_alloc, uptr resource, uptr desc) {
    bool in_battle = IsInBattle();
    auto& ctx = feature::WeatherManager::GetInstance();

    HookManager::Call<void>(HookID::kParticleCreate, self, heap_alloc,
                            device_alloc, resource, desc);

    if (!in_battle || self == nullptr) return;

    // self->SetScale(5.0f * scale, 3.0f * scale, 50.0f * scale);
  }

  static void OnResourceAttachBufferAndSetup(uptr self, uptr allocator,
                                             uptr p_buff, uptr desc,
                                             u32 flag_a, u32 flag_b) {
    if (IsInBattle() && p_buff != 0) {
      PatchWeatherParticleColor(p_buff);
    }
    HookManager::Call<void>(HookID::kResourceAttachBufferAndSetup, self,
                            allocator, p_buff, desc, flag_a, flag_b);
  }

  static void UpdateH3dModel(renderer::H3dModel* h3d_model) {
    if (!IsInBattle()) {
      return;
    }

    if (IsBattlePokemonModel(h3d_model)) {
      using FilterFunc = void (renderer::H3dModel::*)();
      static constexpr FilterFunc kFilters[] = {
          nullptr,
          &renderer::H3dModel::ApplyPitchBlack, // 1
          &renderer::H3dModel::ApplyInvert, // 2
          &renderer::H3dModel::ApplyDarken, // 3
          &renderer::H3dModel::ApplyOverexposed, // 4
          &renderer::H3dModel::ApplyPsychedelic, // 5
          &renderer::H3dModel::ApplyTrueSaturation, // 6
          &renderer::H3dModel::ApplySepia, // 7
          &renderer::H3dModel::ApplyTintRed, // 8
          &renderer::H3dModel::ApplyTintBlue, // 9
          &renderer::H3dModel::ApplyTintGreen, // 10
          &renderer::H3dModel::ApplyNightVision, // 11
          &renderer::H3dModel::ApplyVintage, // 12
          &renderer::H3dModel::ApplySolarize, // 13
          &renderer::H3dModel::ApplyChromeMetallic, // 14
          &renderer::H3dModel::ApplyGoldMetallic, // 15
          &renderer::H3dModel::ApplyDuotoneOceanFire, // 16
          &renderer::H3dModel::ApplyThermalCam, // 17
          &renderer::H3dModel::ApplyFilmNoir, // 18
          &renderer::H3dModel::ApplyBlueprint, // 19
          &renderer::H3dModel::ApplyXray, // 20
          &renderer::H3dModel::ApplyToon, // 21
          &renderer::H3dModel::ApplyGhostMode // 22
      };

      auto& ctx = GetInstance();
      if (ctx.filter > 0 && ctx.filter < (
            sizeof(kFilters) / sizeof(FilterFunc))) {
        if (auto func = kFilters[ctx.filter]) {
          (h3d_model->*func)();
        }
      }
    }
  }

  static void AddH3dModel(uptr scene, renderer::H3dModel* model, u32 idx) {
    UpdateH3dModel(model);
    HookManager::Call<void>(HookID::kSceneRegister0, scene, model, idx);
  }
};
} // namespace feature