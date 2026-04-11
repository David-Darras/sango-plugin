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

#ifndef SANGO_PLUGIN_SOUND_H_
#define SANGO_PLUGIN_SOUND_H_

#include "core/core.h"

/**
 * @class Sound
 * @brief Singleton class for audio control and menu integration.
 * * This class manages game audio by wrapping low-level function calls to play
 * Pokemon cries, sound effects, and background music.
 */
class Sound {
 public:
  /**
   * @brief Retrieves the singleton instance of the Sound manager.
   * @return Reference to the Sound instance.
   */
  static Sound& GetInstance() { return instance_; }

  /**
   * @brief Registers sound-related entries into the provided plugin menu.
   * @param menu Reference to the PluginMenu to populate.
   */
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  /**
   * @brief Directly plays a Pokémon's cry.
   * @param pokemon The ID of the Pokémon to play.
   */
  static void PlayPokemonCry(u16 pokemon) {
    ((void (*)(u8, u16, u16, u8, u8))ADDRESS_SOUND_PLAY_POKEMON_CRY)(0, pokemon,
                                                                     0, 0, 0);
  }

  /**
   * @brief Sets the volume for Pokémon cries.
   * @param volume Floating point multiplier for volume.
   */
  static void ChangePokemonCryVolume(f32 volume) {
    ((void (*)(u8, f32, u32))ADDRESS_SOUND_CHANGE_POKEMON_CRY_VOLUME)(0, volume,
                                                                      0);
  }

  /**
   * @brief Plays a specific sound effect from the game's bank.
   * @param index The sound effect index.
   */
  static void PlaySoundEffect(u32 index) {
    // Note: (6 << 16) is the bitmask for the SE bank
    ((void (*)(u32, u32, s32, u32))ADDRESS_SOUND_PLAY_SOUND_EFFECT)(
        (6 << 16) + index, 0, 0, 0);
  }

  /**
   * @brief Starts playing a specific background music track.
   * @param index The BGM track index.
   */
  static void PlayBackgroundMusic(u32 index) {
    // Note: (1 << 16) is the bitmask for the BGM bank
    ((void (*)(u32, u32, u32, u32))ADDRESS_SOUND_PLAY_BACKGROUND_MUSIC)(
        (1 << 16) + index, 10, 10, 1);
  }

 private:
  /** @name Internal Callbacks
   * Used for menu entry execution.
   * @{ */
  static void OnPlayPokemonCry(void* arg);
  static void OnChangeVolume(void* arg);
  static void OnPlaySoundEffect(void* arg);
  static void OnPlayBackgroundMusic(void* arg);
  /** @} */

  f32 volume_;            ///< Current volume multiplier for cries.
  u16 pokemon_cry_;       ///< Selected Pokémon ID for the menu.
  u16 sound_effect_;      ///< Selected SE index for the menu.
  u16 background_music_;  ///< Selected BGM index for the menu.

  static Sound instance_;  ///< Static singleton instance.

  /**
   * @brief Private constructor initializing default audio state.
   */
  Sound()
      : volume_(1.0f),
        pokemon_cry_(0),
        sound_effect_(0),
        background_music_(0) {}
};

#endif  // SANGO_PLUGIN_SOUND_H_