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

#include "renderer/patch/model_filter.h"
#include "core/hook_manager.h"
#include "battle/native/config.h"
#include "battle/native/manager.h"
#include "core/native/process_manager.h"
#include "overworld/patch/weather_override.h"
#include "renderer/native/h3d_model.h"

namespace renderer {

void ModelFilter::Initialize() {
  core::HookManager::Initialize(HookId::kSceneRegister0,
                          address::kSceneRegister0,
                          (uptr)AddH3dModel);
  core::HookManager::Initialize(HookId::kParticleCreate,
                          address::kParticleCreate,
                          (uptr)OnParticleCreate);
  core::HookManager::Initialize(HookId::kResourceAttachBufferAndSetup,
                          address::kResourceAttachBufferAndSetup,
                          (uptr)OnResourceAttachBufferAndSetup);
}

bool ModelFilter::IsInBattle() {
  return core::ProcessManager::GetInstance().IsCurrentProcess(
      battle::address::kVtable);
}

bool ModelFilter::IsBattlePokemonModel(const H3dModel* h3d_model) {
  auto& grp = battle::Graphics::GetInstance();
  for (u32 i = 0; i < 6; i++) {
    auto* battle_model = grp.pokemon_model[i];
    if (battle_model == nullptr) continue;
    if (battle_model->h3d_model == h3d_model) return true;
  }
  return false;
}

bool ModelFilter::IsBattleTrainerModel(const H3dModel* h3d_model) {
  auto& grp = battle::Graphics::GetInstance();
  for (u32 i = 0; i < 4; i++) {
    auto* battle_model = grp.trainer_model[i];
    if (battle_model == nullptr) continue;
    if (battle_model->h3d_model == h3d_model) return true;
  }
  return false;
}

bool ModelFilter::IsBattlePlatformModel(const H3dModel* h3d_model) {
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

bool ModelFilter::IsBattleEnvironnementModel(const H3dModel* h3d_model) {
  auto& grp = battle::Graphics::GetInstance();
  return grp.environnement->unknow_0 == h3d_model
         || grp.environnement->unknow_1 == h3d_model
         || grp.environnement->landscape == h3d_model
         || grp.environnement->ground == h3d_model;
}

void ModelFilter::PatchWeatherParticleColor(uptr raw) {
  auto& ctx = overworld::WeatherOverride::GetInstance();
  if (ctx.mode == overworld::WeatherMode::kNormal)
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

  u16 color565;
  switch (ctx.mode) {
    case overworld::WeatherMode::kToxic: color565 = 0x90BBu; break; // violet
    case overworld::WeatherMode::kRadioactive: color565 = 0x97E0u; break; // green
    default: return;
  }

  WRITE32(texture_ptr + 52, 3u); // RGB565
  for (s32 i = 0; i < texel_count; i++) {
    WRITE16(pixels + 2 * i, color565);
  }
}

void ModelFilter::OnParticleCreate(Particle* self, uptr heap_alloc,
                               uptr device_alloc, uptr resource, uptr desc) {
  bool in_battle = IsInBattle();

  core::HookManager::Call<void>(HookId::kParticleCreate, self, heap_alloc,
                          device_alloc, resource, desc);

  if (!in_battle || self == nullptr) return;
}

void ModelFilter::OnResourceAttachBufferAndSetup(uptr self, uptr allocator,
                                           uptr p_buff, uptr desc,
                                           u32 flag_a, u32 flag_b) {
  if (IsInBattle() && p_buff != 0) {
    PatchWeatherParticleColor(p_buff);
  }
  core::HookManager::Call<void>(HookId::kResourceAttachBufferAndSetup, self,
                          allocator, p_buff, desc, flag_a, flag_b);
}

void ModelFilter::UpdateH3dModel(H3dModel* h3d_model) {
  if (!IsInBattle()) {
    return;
  }

  if (IsBattlePokemonModel(h3d_model)) {
    using FilterFunc = void (H3dModel::*)();
    static constexpr FilterFunc kFilters[] = {
        nullptr, // kNormal
        &H3dModel::ApplyPitchBlack,
        &H3dModel::ApplyInvert,
        &H3dModel::ApplyDarken,
        &H3dModel::ApplyOverexposed,
        &H3dModel::ApplyPsychedelic,
        &H3dModel::ApplyTrueSaturation,
        &H3dModel::ApplySepia,
        &H3dModel::ApplyTintRed,
        &H3dModel::ApplyTintBlue,
        &H3dModel::ApplyTintGreen,
        &H3dModel::ApplyNightVision,
        &H3dModel::ApplyVintage,
        &H3dModel::ApplySolarize,
        &H3dModel::ApplyChromeMetallic,
        &H3dModel::ApplyGoldMetallic,
        &H3dModel::ApplyDuotoneOceanFire,
        &H3dModel::ApplyThermalCam,
        &H3dModel::ApplyFilmNoir,
        &H3dModel::ApplyBlueprint,
        &H3dModel::ApplyXray,
        &H3dModel::ApplyToon,
        &H3dModel::ApplyGhostMode,
    };
    static_assert(SIZE(kFilters) == static_cast<u32>(TextureFilter::kCount),
                  "one entry per TextureFilter");

    auto& ctx = GetInstance();
    if (ctx.filter < TextureFilter::kCount) {
      if (auto func = kFilters[static_cast<u8>(ctx.filter)]) {
        (h3d_model->*func)();
      }
    }
  }
}

void ModelFilter::AddH3dModel(uptr scene, H3dModel* model, u32 idx) {
  UpdateH3dModel(model);
  core::HookManager::Call<void>(HookId::kSceneRegister0, scene, model, idx);
}

} // namespace renderer
