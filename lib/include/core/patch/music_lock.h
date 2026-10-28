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
 * @file music_lock.h
 * @brief A music lock: when it is on, the music of the game does not change.
 */

#pragma once

#include "common.h"
#include "system/constant/background_music.h"

namespace core {
/**
 * @brief Keeps the music that plays when the lock is on.
 *
 * With the lock on, the music stays the same in a battle, when the zone
 * changes, and in the other places where the game changes its music. The
 * music is not restarted: it goes on from where it is.
 *
 * The lock blocks only the changes that the game asks for. A music that
 * the plugin starts (the pages of the menu, the zone settings, the SD
 * music) goes through MusicLock::Play(), and becomes the kept music.
 *
 * On Omega Ruby and Alpha Sapphire the music is never stopped. On X and Y the
 * music of a battle is replaced by the kept music, so it restarts.
 */
class MusicLock {
  MAKE_SINGLETON(MusicLock)
public:
  bool is_enabled = false;

  static void Initialize();

  /// Plays a music for the plugin. The lock does not block it. The music
  /// becomes the kept music.
  static void Play(BackgroundMusicId music);
  static void Play(u32 sound_id);

  /// Returns true when the lock is on and has a kept music.
  bool IsHolding() const;
  /// Returns the id of the music that plays now, or 0.
  u32 GetPlaying() const;
  /// Returns the id of the kept music, or 0.
  u32 GetKept() const { return kept_; }

  /// Called when the lock turns on or off (the menu calls it).
  static void OnToggle();

private:
  static void ChangeMusicHook(u32 id, u32 fade_out, u32 fade_in, u32 flag);
  static void StartBattleMusicHook(uptr sound, u32 id, u8 fade);
#ifdef GAME_ORAS
  static void EndBattleMusicHook(u32 next_music);
#endif

  u32 kept_ = 0; ///< The music that the lock keeps.
  u32 last_played_ = 0; ///< The last music that a change asked for.
  bool plugin_play_ = false; ///< true while Play() calls the game.
  bool battle_music_skipped_ = false;
};
} // namespace core
