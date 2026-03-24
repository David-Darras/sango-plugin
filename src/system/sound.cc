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

#include "system/sound.h"

#include "menu/plugin_menu.h"

// Define the static singleton instance
Sound Sound::instance_;

/**
 * @brief Callback to play a cry using the menu's selected ID.
 */
void Sound::OnPlayPokemonCry(void*) { PlayPokemonCry(instance_.pokemon_cry_); }

/**
 * @brief Callback to apply the volume selected in the menu.
 */
void Sound::OnChangeVolume(void*) { ChangePokemonCryVolume(instance_.volume_); }

/**
 * @brief Callback to play a sound effect from the menu.
 */
void Sound::OnPlaySoundEffect(void*) {
  PlaySoundEffect(instance_.sound_effect_);
}

/**
 * @brief Callback to play background music from the menu.
 */
void Sound::OnPlayBackgroundMusic(void*) {
  PlayBackgroundMusic(instance_.background_music_);
}

/**
 * @brief Populates the PluginMenu with audio controls.
 * Uses method chaining to add entries and their corresponding callbacks.
 */
void Sound::LoadMenu(menu::PluginMenu& menu, void* args) {
  menu.Add("Pokemon Cry", instance_.pokemon_cry_)
      .WithCallback(OnPlayPokemonCry)

      .Add("Pokemon Cry Volume", instance_.volume_)
      .WithCallback(OnChangeVolume)

      .Add("Sound Effect", instance_.sound_effect_)
      .WithCallback(OnPlaySoundEffect)

      .Add("Background Music", instance_.background_music_)
      .WithCallback(OnPlayBackgroundMusic);
}
