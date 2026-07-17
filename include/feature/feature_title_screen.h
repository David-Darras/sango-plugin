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
#include "cheat_code_manager.h"
#include "common.h"
#include "game/constant/species.h"
#include "game/constant/video.h"

namespace feature {
class TitleScreen {
  MAKE_SINGLETON(TitleScreen)
  u8 top_video = VIDEO_PRIMO_GROUDON;
  u8 bottom_video = VIDEO_PRIMO_KYOGRE;
  bool no_delay = true;
  u16 pokemon_cry = SPECIES_PIKACHU;
  f32 pokemon_cry_volume = 10.0f;

  STATIC_INLINE void Initialize() {
    auto& title = GetInstance();
    // TOP
    WRITE(vu32, 0x00740378, 0xE3A02000 | title.top_video);
    // BOTTOM
    WRITE(vu32, 0x0074039C, 0xE3A02000 | title.bottom_video);
    // NO DELAY FOR BOTTOM SCREEN
    if (title.no_delay) {
      ARM_NOP(0x00740390);
    }
    // POKEMON CRY
    WRITE(vu32, 0x00740498, title.pokemon_cry);
    // VOLUME
    WRITE(vf32, 0x0074049C, title.pokemon_cry_volume);
  }
};
} // namespace feature