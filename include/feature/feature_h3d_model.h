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

#include "hook_manager.h"
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

  static void UpdateH3dModel(renderer::H3dModel* h3d_model) {
    if (!IsInBattle()) {
      return;
    }

    if (!IsBattlePokemonModel(h3d_model)) {
      return;
    }

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

  static void AddH3dModel(uptr scene, renderer::H3dModel* model, u32 idx) {
    UpdateH3dModel(model);
    HookManager::Call<void>(HookID::kSceneRegister0, scene, model, idx);
  }
};
} // namespace feature