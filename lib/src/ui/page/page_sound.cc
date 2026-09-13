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

#include "ui/main_application.h"
#include "system/native/sound.h"

namespace ui {
static f32 pokemon_cry_volume = 1.0f;
static SpeciesId pokemon_cry_species = SpeciesId::kNone;
static u16 sound_effect_id = 0;
static BackgroundMusicId background_music = BackgroundMusicId::kPokemonTheme;

void ChangePokemonCryVolume(void*) {
  sys::Sound::ChangePokemonCryVolume(pokemon_cry_volume);
}

void PlayPokemonCry(void*) {
  sys::Sound::PlayPokemonCry(pokemon_cry_species);
}

void PlaySoundEffect(void*) {
  sys::Sound::PlaySoundEffect(sound_effect_id);
}

void PlayBackgroundMusic(void*) {
  sys::Sound::PlayBackgroundMusic(background_music);
}

void LoadSoundPage(MainApplication& app, void* args) {
  app.Add("Pokemon Cry Volume", pokemon_cry_volume)
      .WithCallback(ChangePokemonCryVolume)
      .AddSpecies("Pokemon Cry", pokemon_cry_species)
      .WithCallback(PlayPokemonCry)
      .Add("Sound Effect Id", sound_effect_id)
      .WithCallback(PlaySoundEffect)
      .Add("Background Music Id", background_music)
      .WithBounds(0, static_cast<u32>(BackgroundMusicId::kCount) - 1)
      .WithCallback(PlayBackgroundMusic);
}
} // namespace ui
