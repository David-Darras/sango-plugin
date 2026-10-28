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

/**
 * @file page_sound.cc
 * @brief The menu page of the sounds and the music.
 */

#include "core/patch/music_files.h"
#include "core/patch/music_lock.h"
#include "ui/page/enum_names.h"
#include "ui/main_application.h"
#include "system/native/sound.h"

namespace ui {
static f32 pokemon_cry_volume = 1.0f;
static SpeciesId pokemon_cry_species = SpeciesId::kNone;
static u16 sound_effect_id = 0;
static BackgroundMusicId background_music = BackgroundMusicId::kDefault;
static u32 sd_music = 0;
static u32 sd_music_count = 0;
static const c8* sd_music_names[core::MusicFiles::kMaxFiles];

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
  core::MusicLock::Play(background_music);
}

void PlaySdMusic(void*) {
  auto& files = core::MusicFiles::GetInstance();
  files.PlayFile(sd_music);
}

void LoadSoundPage(MainApplication& app, void* args) {
  auto& files = core::MusicFiles::GetInstance();
  sd_music_count = files.LoadList();
  for (u32 i = 0; i < sd_music_count; i++) {
    sd_music_names[i] = files.GetTitle(i);
  }
  app.OnChange([](void*) { core::MusicLock::OnToggle(); })
      .Add("Lock Music", core::MusicLock::GetInstance().is_enabled)
      .WithDescription("On: the music that plays now stays. It does not "
                       "change in a battle, when the zone changes, or in "
                       "the other places where the game changes its music. "
                       "The music of this page still plays.")
      .Add("Pokemon Cry Volume", pokemon_cry_volume)
      .WithFactor(0.1f)
      .WithCallback(ChangePokemonCryVolume)
      .WithDescription("Press {A} to use this volume. 1 is the normal volume.")
      .AddSpecies("Pokemon Cry", pokemon_cry_species)
      .WithCallback(PlayPokemonCry)
      .WithDescription("Press {A} to play the cry.")
      .Add("Sound Effect", sound_effect_id)
#ifndef GAME_XY
      .WithArray(kSoundEffectNames, SIZE(kSoundEffectNames))
#endif
      .WithCallback(PlaySoundEffect)
      .WithDescription("Press {A} to play the sound effect.")
      .Add("Background Music", background_music)
      .WithArray(kBackgroundMusicNames, SIZE(kBackgroundMusicNames))
      .WithCallback(PlayBackgroundMusic)
      .WithDescription("Press {A} to play the music.");
  if (sd_music_count > 0) {
    app.Add("SD Music", sd_music)
        .WithArray(sd_music_names, sd_music_count)
        .WithCallback(PlaySdMusic)
        .WithDescription("Press {A} to play a file of sdmc:/sango/bgm/list.txt.");
  }
}
} // namespace ui
