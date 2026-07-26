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
#include "game/renderer/pokemon_model.h"
#include "ui/log_application.h"

namespace feature {
class TitleScreen {
  MAKE_SINGLETON(TitleScreen)
  bool is_enabled = true;
  u8 top_video = VIDEO_PRIMO_KYOGRE;
  u8 bottom_video = VIDEO_PRIMO_GROUDON;
  bool no_delay = true;
  bool no_shadow = true;
  u16 pokemon_cry_species = SPECIES_BELDUM;
  f32 pokemon_cry_volume = 1.0f;

  STATIC_INLINE void Patch() {
    auto& title = GetInstance();
    if (!title.is_enabled) return;

    WRITE(vu32, 0x00740378, 0xE3A02000 | title.top_video);
    WRITE(vu32, 0x0074039C, 0xE3A02000 | title.bottom_video);
    WRITE(vu32, 0x00740498, title.pokemon_cry_species);
    WRITE(vf32, 0x0074049C, title.pokemon_cry_volume);
    if (title.no_delay)
      ARM_NOP(0x00740390);
    if (title.no_shadow)
      ARM_NOP(0x00740104);

    PokemonModel::GetInstance().is_enabled = true;
  }
};
} // namespace feature