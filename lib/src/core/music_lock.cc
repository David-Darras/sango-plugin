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
 * @file music_lock.cc
 * @brief A music lock: when it is on, the music of the game does not change.
 *
 * The declarations are in core/patch/music_lock.h.
 */

#include "core/patch/music_lock.h"

#include "core/hook.h"
#include "system/address.h"
#include "system/native/sound.h"

namespace core {

namespace {
// The function that changes the music of the overworld, and the music of
// many events. Parameters: the id, the fade out, the fade in, a flag.
core::Hook<void(u32, u32, u32, u32)> change_music_hook;
core::Hook<void(uptr, u32, u8)> start_battle_music_hook;
#ifdef GAME_ORAS
core::Hook<void(u32)> end_battle_music_hook;
#endif

constexpr u32 kFade = 10;
// The frames of the fade in after a battle.
constexpr u32 kFadeInFrames = 30;
// The offset, in the music player, of the id of the playing music.
constexpr u32 kPlayingIdOffset = 0x20;
} // namespace

void MusicLock::Initialize() {
  change_music_hook.Install(sys::address::kSoundPlayBackgroundMusic,
                            ChangeMusicHook);
  start_battle_music_hook.Install(sys::address::kSoundStartBattleMusic,
                                  StartBattleMusicHook);
#ifdef GAME_ORAS
  end_battle_music_hook.Install(sys::address::kSoundEndBattleMusic,
                                EndBattleMusicHook);
#endif
}

void MusicLock::Play(BackgroundMusicId music) {
  Play(sys::Sound::kBankBackgroundMusic + static_cast<u32>(music));
}

void MusicLock::Play(u32 sound_id) {
  auto& ctx = GetInstance();
  ctx.plugin_play_ = true;
  ctx.kept_ = sound_id;
  ChangeMusicHook(sound_id, kFade, kFade, 1);
  ctx.plugin_play_ = false;
}

u32 MusicLock::GetPlaying() const {
#ifdef GAME_ORAS
  const uptr player = READ32(sys::address::kSoundManager + 4);
  return player != 0 ? READ32(player + kPlayingIdOffset) : 0;
#else
  return last_played_;
#endif
}

bool MusicLock::IsHolding() const { return is_enabled && kept_ != 0; }

void MusicLock::OnToggle() {
  auto& ctx = GetInstance();
  // The kept music is the music that plays when the lock turns on.
  ctx.kept_ = ctx.is_enabled ? ctx.GetPlaying() : 0;
  ctx.battle_music_skipped_ = false;
}

void MusicLock::ChangeMusicHook(u32 id, u32 fade_out, u32 fade_in, u32 flag) {
  auto& ctx = GetInstance();
  if (!ctx.plugin_play_ && ctx.IsHolding()) return;
  ctx.last_played_ = id;
  // Turned on by the code before any music plays: keep the first music.
  if (ctx.is_enabled && ctx.kept_ == 0) ctx.kept_ = id;
  change_music_hook(id, fade_out, fade_in, flag);
}

void MusicLock::StartBattleMusicHook(uptr sound, u32 id, u8 fade) {
  auto& ctx = GetInstance();
  if (!ctx.IsHolding()) {
    ctx.battle_music_skipped_ = false;
    start_battle_music_hook(sound, id, fade);
    return;
  }
#ifdef GAME_ORAS
  // Do not start the music of the battle: the game then neither fades out
  // nor saves the music that the lock keeps, which goes on without a restart.
  ctx.battle_music_skipped_ = true;
#else
  start_battle_music_hook(sound, ctx.kept_, fade);
#endif
}

#ifdef GAME_ORAS
void MusicLock::EndBattleMusicHook(u32 next_music) {
  auto& ctx = GetInstance();
  if (ctx.battle_music_skipped_) {
    // The kept music never stopped, but the battle faded it out when it ended:
    // fade it in again.
    ctx.battle_music_skipped_ = false;
    ((void (*)(u32))sys::address::kSoundFadeInMusic)(kFadeInFrames);
    return;
  }
  end_battle_music_hook(next_music);
}
#endif
} // namespace core
