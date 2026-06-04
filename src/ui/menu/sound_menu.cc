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

#include "menu/plugin_menu.h"
#include "system/sound.h"

namespace ui {
static f32 pokemon_cry_volume = 1.0f;
static u16 pokemon_cry_id = 0;
static u16 sound_effec_id = 0;
static u16 background_music_id = 0;

void ChangePokemonCryVolume(void*) {
  Sound::ChangePokemonCryVolume(pokemon_cry_volume);
}

void PlayPokemonCry(void*) {
  Sound::PlayPokemonCry(pokemon_cry_id);
}

void PlaySoundEffect(void*) {
  Sound::PlaySoundEffect(sound_effec_id);
}

void PlayBackgroundMusic(void*) {
  Sound::PlayBackgroundMusic(background_music_id);
}

void LoadSoundMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Pokemon Cry Volume", pokemon_cry_volume)
      .WithCallback(ChangePokemonCryVolume)
      .Add("Pokemon Cry Id", pokemon_cry_id)
      .WithCallback(PlayPokemonCry)
      .Add("Sound Effect Id", sound_effec_id)
      .WithCallback(PlaySoundEffect)
      .Add("Background Music Id", background_music_id)
      .WithCallback(PlayBackgroundMusic);
}
}