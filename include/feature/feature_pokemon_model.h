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
#include "hook_manager.h"
#include "game/constant/form.h"
#include "game/constant/species.h"
#include "ui/log_application.h"

namespace feature {
class PokemonModel {
  MAKE_SINGLETON(PokemonModel);
  bool is_enabled = false;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kReplacePokemonModel, 0x004713FC,
                            (uptr)ReplacePokemonModelHook);
    HookManager::Initialize(HookID::kUpdateBattleView, 0x0073B60C,
                            (uptr)UpdateBattleViewHook, false);
  }

  static void UpdateBattleViewHook(uptr p0) {
    static u32 counter = 20;
    if (counter >= 20) {
      FixPokemonSize();
      counter = 0;
    }
    counter++;
    HookManager::Call<void>(HookID::kUpdateBattleView, p0);
  }

  static void FixPokemonSize() {
    for (u32 i = 0; i < 6; i++) {
      u32 pkmMdl = *(u32*)(0x83F84C0 + 4 * i); // Base address of the 3D model
      if (pkmMdl == 0)
        continue;

      u16 pkmNum = *(u16*)(pkmMdl + 0x170); // Pokédex number
      if (pkmNum >= 722)
        continue;

      u32 pkmData = *(u32*)0x617A00 + 0x50 * pkmNum;
      // Personal data of the Pokémon
      float realSize = (float)*(u16*)(pkmData + 0x24);
      // Actual height according to the Pokédex
      float defaultSize = (float)*(u16*)(pkmData + 0x3C);
      // Default displayed size in battle
      float ratio = realSize / defaultSize;

      // Update Pokémon scale
      *(float*)(pkmMdl + 0x34) = ratio;
      *(float*)(pkmMdl + 0x38) = ratio;
      *(float*)(pkmMdl + 0x3C) = ratio;

      // Mark the model to be updated
      *(bool*)(pkmMdl + 0x4C) = true;
    }
  }

  static void ReplacePokemonModelHook(void* model, PokeInfo* poke_info,
                                      void* p0,
                                      void* p1, void* p2, void* p3) {
    if (!GetInstance().is_enabled) goto end;

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
      case SPECIES_AZURILL:
        WRITE(vu32, 0x0071E05C, 0x03A00003); // skip name input
        // ARM_NO_COND(0x0071AD04); // disable left/right (male only)
        // ARM_NO_COND(0x0071ACF4);
        WRITE(vu32, 0x00719D60, 0xE3A00000 | 25);
        poke_info->species = SPECIES_KELDEO;
        poke_info->form = FORM_KELDEO_RESOLUTE;
        poke_info->is_shiny = false;
        break;
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
  end:
    HookManager::Call<void>(HookID::kReplacePokemonModel, model, poke_info, p0,
                            p1, p2, p3);
  }
};
} // namespace feature