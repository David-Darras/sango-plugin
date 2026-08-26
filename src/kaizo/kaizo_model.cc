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
#include "feature/hook_manager.h"
#include "game/constant/form.h"
#include "game/constant/model.h"
#include "game/constant/species.h"

namespace kaizo {
u32 PatchOverworldModels(u32 model, bool is_real_overworld) {
  if (!is_real_overworld) {
    switch (model) {
      case MODEL_ZIGZAGOON:
        return MODEL_GROUDON;
      case MODEL_SKITTY:
        return MODEL_KYOGRE;
    }
  }
  switch (model) {
    case MODEL_BRENDAN:
      return MODEL_STEVEN_STONE;
    case MODEL_SUPPORT_MAY:
      return MODEL_ZINNIA;
    case MODEL_MOM_ORAS:
      return MODEL_MR_STONE;
    case MODEL_SNORLAX_DOLL:
      return MODEL_HOOPAS_RING;
  }
  return model;
}

void PatchPokemonModels(PokeInfo* poke_info) {
  switch (poke_info->species) {
    case SPECIES_LATIOS:
    case SPECIES_LATIAS:
      poke_info->is_shiny = true;
      poke_info->species = SPECIES_BELDUM;
      break;
    case SPECIES_KYOGRE:
      poke_info->is_shiny = true;
      poke_info->form = FORM_KYOGRE_ALPHA;
      break;
    case SPECIES_GROUDON:
      poke_info->is_shiny = true;
      poke_info->form = FORM_GROUDON_OMEGA;
      break;
    case SPECIES_SHROOMISH:
      static u32 count = 0;
      if (count == 0) {
        poke_info->species = SPECIES_FURFROU;
        poke_info->form = FORM_FURFROU_HEART;
        poke_info->is_shiny = true;
      } else {
        poke_info->species = SPECIES_PIKACHU;
        poke_info->form = FORM_PIKACHU_CLEVER;
        poke_info->is_shiny = true;
      }
      count++;
      break;
    case SPECIES_AZURILL: {
      // Skip the mandatory nickname-entry prompt that normally follows
      // catching/receiving this species.
      constexpr uptr kAddressSkipNameInput = 0x0071E05C;
      WRITE32(kAddressSkipNameInput, 0x03A00003);
      // Force the gift/starter level to 25 regardless of the game's default.
      constexpr uptr kAddressForceLevel = 0x00719D60;
      constexpr u32 kForcedLevel = 25;
      WRITE32(kAddressForceLevel, 0xE3A00000 | kForcedLevel);
      poke_info->species = SPECIES_KELDEO;
      poke_info->form = FORM_KELDEO_RESOLUTE;
      poke_info->is_shiny = false;
      break;
    }
    case SPECIES_LAIRON:
      poke_info->is_shiny = true;
      break;
    case SPECIES_ARON:
      poke_info->species = SPECIES_MAWILE;
      poke_info->form = FORM_MAWILE_MEGA;
      poke_info->is_shiny = false;
      break;
    case SPECIES_POOCHYENA:
      poke_info->species = SPECIES_KADABRA;
      poke_info->form = 0;
      poke_info->is_shiny = false;
      break;
    case SPECIES_TROPIUS:
      poke_info->is_shiny = true;
      break;
    case SPECIES_TAILLOW:
      poke_info->species = SPECIES_BELDUM;
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
  HookManager::Call<void>(HookID::kReplacePokemonModel, model, poke_info, p0,
                          p1, p2, p3);
}

void InitializeModelHook() {
  HookManager::Initialize(HookID::kReplacePokemonModel,
                          ADDRESS_REPLACE_POKEMON_MODEL,
                          (uptr)ReplacePokemonModelHook);
}
}