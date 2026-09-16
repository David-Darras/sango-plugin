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
#include "pokemon/patch/mega_evolution.h"
#include "pokemon/patch/model_replacement.h"
#include "pokemon/constant/form.h"
#include "overworld/constant/model.h"
#include "pokemon/constant/species.h"

namespace kaizo {
ModelId PatchOverworldModels(ModelId model, bool is_real_overworld) {
  if (!is_real_overworld) {
    switch (model) {
      case ModelId::kZigzagoon:
        return ModelId::kGroudon;
      case ModelId::kSkitty:
        return ModelId::kKyogre;
      default:
        break;
    }
  }
  switch (model) {
    case ModelId::kBrendan:
      return ModelId::kStevenStone;
    case ModelId::kSupportMay:
      return ModelId::kZinnia;
    case ModelId::kMomOras:
      return ModelId::kMrStone;
    case ModelId::kSnorlaxDoll:
      return ModelId::kHoopasRing;
    default:
      return model;
  }
}

void PatchPokemonModels(PokeInfo* poke_info) {
  switch (poke_info->species) {
    case SpeciesId::kLatios:
    case SpeciesId::kLatias:
      poke_info->is_shiny = true;
      poke_info->species = SpeciesId::kBeldum;
      break;
    case SpeciesId::kKyogre:
      poke_info->is_shiny = true;
      poke_info->form = FormId::kKyogreAlpha;
      break;
    case SpeciesId::kGroudon:
      poke_info->is_shiny = true;
      poke_info->form = FormId::kGroudonOmega;
      break;
    case SpeciesId::kShroomish:
      static u32 count = 0;
      if (count == 0) {
        poke_info->species = SpeciesId::kFurfrou;
        poke_info->form = FormId::kFurfrouHeart;
        poke_info->is_shiny = true;
      } else {
        poke_info->species = SpeciesId::kPikachu;
        poke_info->form = FormId::kPikachuClever;
        poke_info->is_shiny = true;
      }
      count++;
      break;
    case SpeciesId::kAzurill: {
      constexpr uptr kAddressSkipNameInput = 0x0071E05C;
      WRITE32(kAddressSkipNameInput, 0x03A00003);
      constexpr uptr kAddressForceLevel = 0x00719D60;
      constexpr u32 kForcedLevel = 25;
      WRITE32(kAddressForceLevel, 0xE3A00000 | kForcedLevel);
      poke_info->species = SpeciesId::kKeldeo;
      poke_info->form = FormId::kKeldeoResolute;
      poke_info->is_shiny = false;
      break;
    }
    case SpeciesId::kLairon:
      poke_info->is_shiny = true;
      break;
    case SpeciesId::kAron:
      poke_info->species = SpeciesId::kMawile;
      poke_info->form = FormId::kMawileMega;
      poke_info->is_shiny = false;
      break;
    case SpeciesId::kPoochyena:
      poke_info->species = SpeciesId::kKadabra;
      poke_info->form = FormId::kNormal;
      poke_info->is_shiny = false;
      break;
    case SpeciesId::kTropius:
      poke_info->is_shiny = true;
      break;
    case SpeciesId::kTaillow:
      poke_info->species = SpeciesId::kBeldum;
      poke_info->is_shiny = true;
      break;
    default:
      break;
  }
}

static bool is_enabled = false;
static s32 mega_step = 0;

void ShouldReplacePokemonModel(bool no_yes) {
  is_enabled = no_yes;
}

static void OnPokemonModel(PokeInfo* poke_info) {
  if (is_enabled) {
    PatchPokemonModels(poke_info);
  }
  if (poke_info->species == SpeciesId::kMimeJr
      && poke_info->form == pokemon::kFormMimeJrMega) {
    mega_step++;
    if (mega_step == 1) {
      poke_info->species = SpeciesId::kSmoochum;
      poke_info->is_shiny = false;
      poke_info->form = FormId::kNormal;
    } else if (mega_step == 2) {
      poke_info->species = SpeciesId::kJynx;
      poke_info->is_shiny = false;
      poke_info->form = FormId::kNormal;
    } else if (mega_step == 3) {
      poke_info->species = SpeciesId::kGardevoir;
      poke_info->is_shiny = false;
      poke_info->form = FormId::kNormal;
    } else if (mega_step == 4) {
      poke_info->species = SpeciesId::kGarchomp;
      poke_info->is_shiny = true;
      poke_info->form = FormId::kGarchompMega;
    }
  }
}

void InitializeModelHook() {
  pokemon::ModelReplacement::GetInstance().on_create = OnPokemonModel;
}
}