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
#include "common.h"
#include "feature/core/hook_manager.h"
#include "feature/pokemon/feature_mega_evolution.h"
#include "game/constant/form.h"
#include "game/constant/model.h"
#include "game/constant/species.h"

namespace kaizo {
u32 PatchOverworldModels(u32 model, bool is_real_overworld) {
  if (!is_real_overworld) {
    switch (static_cast<ModelID>(model)) {
      case ModelID::kZigzagoon:
        return static_cast<u32>(ModelID::kGroudon);
      case ModelID::kSkitty:
        return static_cast<u32>(ModelID::kKyogre);
    }
  }
  switch (static_cast<ModelID>(model)) {
    case ModelID::kBrendan:
      return static_cast<u32>(ModelID::kStevenStone);
    case ModelID::kSupportMay:
      return static_cast<u32>(ModelID::kZinnia);
    case ModelID::kMomOras:
      return static_cast<u32>(ModelID::kMrStone);
    case ModelID::kSnorlaxDoll:
      return static_cast<u32>(ModelID::kHoopasRing);
  }
  return model;
}

void PatchPokemonModels(PokeInfo* poke_info) {
  switch (poke_info->species) {
    case Species::kLatios:
    case Species::kLatias:
      poke_info->is_shiny = true;
      poke_info->species = Species::kBeldum;
      break;
    case Species::kKyogre:
      poke_info->is_shiny = true;
      poke_info->form = Form::kKyogreAlpha;
      break;
    case Species::kGroudon:
      poke_info->is_shiny = true;
      poke_info->form = Form::kGroudonOmega;
      break;
    case Species::kShroomish:
      static u32 count = 0;
      if (count == 0) {
        poke_info->species = Species::kFurfrou;
        poke_info->form = Form::kFurfrouHeart;
        poke_info->is_shiny = true;
      } else {
        poke_info->species = Species::kPikachu;
        poke_info->form = Form::kPikachuClever;
        poke_info->is_shiny = true;
      }
      count++;
      break;
    case Species::kAzurill: {
      // Skip the mandatory nickname-entry prompt that normally follows
      // catching/receiving this species.
      constexpr uptr kAddressSkipNameInput = 0x0071E05C;
      WRITE32(kAddressSkipNameInput, 0x03A00003);
      // Force the gift/starter level to 25 regardless of the game's default.
      constexpr uptr kAddressForceLevel = 0x00719D60;
      constexpr u32 kForcedLevel = 25;
      WRITE32(kAddressForceLevel, 0xE3A00000 | kForcedLevel);
      poke_info->species = Species::kKeldeo;
      poke_info->form = Form::kKeldeoResolute;
      poke_info->is_shiny = false;
      break;
    }
    case Species::kLairon:
      poke_info->is_shiny = true;
      break;
    case Species::kAron:
      poke_info->species = Species::kMawile;
      poke_info->form = Form::kMawileMega;
      poke_info->is_shiny = false;
      break;
    case Species::kPoochyena:
      poke_info->species = Species::kKadabra;
      poke_info->form = static_cast<Form>(0);
      poke_info->is_shiny = false;
      break;
    case Species::kTropius:
      poke_info->is_shiny = true;
      break;
    case Species::kTaillow:
      poke_info->species = Species::kBeldum;
      poke_info->is_shiny = true;
      break;
  }
}

static bool is_enabled = false;

void ShouldReplacePokemonModel(bool no_yes) {
  is_enabled = no_yes;
}

static void ReplacePokemonModelHook(void* model, PokeInfo* poke_info,
                                    void* p0,
                                    void* p1, void* p2, void* p3) {
  if (is_enabled) {
    PatchPokemonModels(poke_info);
  }
  if (poke_info->species == Species::kMimeJr
      && poke_info->form == static_cast<Form>(10)) {
    mega_step++;
    if (mega_step == 1) {
      poke_info->species = Species::kSmoochum;
      poke_info->is_shiny = false;
      poke_info->form = Form::kNormal;
    } else if (mega_step == 2) {
      poke_info->species = Species::kJynx;
      poke_info->is_shiny = false;
      poke_info->form = Form::kNormal;
    } else if (mega_step == 3) {
      poke_info->species = Species::kGardevoir;
      poke_info->is_shiny = false;
      poke_info->form = Form::kNormal;
    } else if (mega_step == 4) {
      poke_info->species = Species::kGarchomp;
      poke_info->is_shiny = true;
      poke_info->form = Form::kGarchompMega;
    }
  }
  HookManager::Call<void>(HookID::kReplacePokemonModel, model, poke_info, p0,
                          p1, p2, p3);
}

void InitializeModelHook() {
  HookManager::Initialize(HookID::kReplacePokemonModel,
                          ADDRESS_REPLACE_POKEMON_MODEL,
                          (uptr)ReplacePokemonModelHook);
}
}