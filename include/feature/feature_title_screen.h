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
#include "game/constant/video.h"
#include "ui/log_application.h"

namespace feature {
class TitleScreen {
  MAKE_SINGLETON(TitleScreen)
  u8 top_video = VIDEO_PRIMO_KYOGRE;
  u8 bottom_video = VIDEO_PRIMO_GROUDON;
  bool no_delay = true;
  u16 pokemon_cry_species = SPECIES_METAGROSS;
  f32 pokemon_cry_volume = 10.0f;

  STATIC_INLINE void Initialize() {
    // Load Trainer Model : 0x00458214
    // 0 = Yvonne, 1 = Xavier, 2 = Malva, 3 = Hologram, 4 = May
    // 5 = Brendan, 6 = Wally, 7 = Wally', 8 = Steven, 9 = Maxie
    // 10 = Archie, 11 = Zinnia, 12 = Shauna, 13 = Tierno, 14 = Trevor
    u32 src = 4, dst = 11;
    WRITE(vu32, 0x00586B8A + src * 2, 0 + dst * 5); // model
    WRITE(vu32, 0x00586BA8 + src * 2, 1 + dst * 5); // base
    WRITE(vu32, 0x00586BC6 + src * 2, 2 + dst * 5); // battle ?
    WRITE(vu32, 0x00586BE4 + src * 2, 3 + dst * 5); // trainer case position
    WRITE(vu32, 0x00586C02 + src * 2, 4 + dst * 5); // introduction position
    src = 5, dst = 8;
    WRITE(vu32, 0x00586B8A + src * 2, 0 + dst * 5); // model
    WRITE(vu32, 0x00586BA8 + src * 2, 1 + dst * 5); // base
    WRITE(vu32, 0x00586BC6 + src * 2, 2 + dst * 5); // battle ?
    WRITE(vu32, 0x00586BE4 + src * 2, 3 + dst * 5); // trainer case position
    WRITE(vu32, 0x00586C02 + src * 2, 4 + src * 5); // introduction position

    HookManager::Initialize(HookID::kSelectPokemonModel, 0x004713FC,
    (uptr)SelectPokemonModelHook);
    HookManager::Initialize(HookID::kSelectPokemonModel, 0x004713FC,
    (uptr)SelectPokemonModelHook);
  }

  struct PokeInfo {
    u16 species;
    u8 form;
    u8 gender;
    bool is_shiny;
    bool is_egg;
    u32 _0;
  };

  static void SelectPokemonModelHook(void* model, PokeInfo* poke_info, void* p0,
                                     void* p1, void* p2, void* p3) {
    auto& title = GetInstance();

    switch (poke_info->species) {
      case SPECIES_LATIOS:
      case SPECIES_LATIAS:
        poke_info->is_shiny = true;
        poke_info->species = SPECIES_BELDUM;
        // Load video : 0x007063C4
        WRITE(vu32, 0x00740378, 0xE3A02000 | title.top_video);
        WRITE(vu32, 0x0074039C, 0xE3A02000 | title.bottom_video);
        if (title.no_delay)
          ARM_NOP(0x00740390);
        WRITE(vu32, 0x00740498, title.pokemon_cry_species);
        WRITE(vf32, 0x0074049C, title.pokemon_cry_volume);
        break;
      case SPECIES_KYOGRE:
        poke_info->species = SPECIES_BELDUM;
        poke_info->is_shiny = false;
        poke_info->form = 0;
        break;
      case SPECIES_GROUDON:
        poke_info->species = SPECIES_METAGROSS;
        poke_info->is_shiny = true;
        poke_info->form = FORM_METAGROSS_MEGA;
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
      case SPECIES_TALLOW:
        poke_info->species = SPECIES_BELDUM;
        poke_info->is_shiny = false;
        break;
      case SPECIES_TORCHIC:
      case SPECIES_TREECKO:
      case SPECIES_MUDKIP:
        poke_info->species = SPECIES_BELDUM;
        poke_info->is_shiny = true;
        break;
    }
    HookManager::Call<void>(HookID::kSelectPokemonModel, model, poke_info, p0,
                            p1, p2, p3);

  }
};
} // namespace feature