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
#include "pokemon/constant/species.h"
#include "system/constant/background_music.h"

namespace sys {

class Sound {
  SINGLETON(Sound)
public:
  /// Sound ids are grouped by bank in the high half word.
  static constexpr u32 kBankBackgroundMusic = 1 << 16;
  static constexpr u32 kBankSoundEffect = 6 << 16;

  STATIC_INLINE void PlayPokemonCry(SpeciesId species) {
    ((void (*)(u8, SpeciesId, u16, u8, u8))address::kSoundPlayPokemonCry)(
        0, species, 0, 0, 0);
  }

  STATIC_INLINE void ChangePokemonCryVolume(f32 volume) {
    ((void (*)(u8, f32, u32))address::kSoundChangePokemonCryVolume)(
        0, volume, 0);
  }

  STATIC_INLINE void PlaySoundEffect(u32 index) {
    ((void (*)(u32, u32, s32, u32))address::kSoundPlaySoundEffect)(
        kBankSoundEffect + index, 0, 0, 0);
  }

  STATIC_INLINE void PlayBackgroundMusic(BackgroundMusicId music) {
    ((void (*)(u32, u32, u32, u32))address::kSoundPlayBackgroundMusic)(
        kBankBackgroundMusic + static_cast<u32>(music), 10, 10, 1);
  }
};

} // namespace sys
