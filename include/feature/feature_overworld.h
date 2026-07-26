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
#include "feature_camera.h"
#include "feature_overworld_model.h"
#include "kaizo.h"
#include "feature/hook_manager.h"

namespace feature {
struct Overworld {
  MAKE_SINGLETON(Overworld)
  bool freeze_background_music = false;
  u16 background_music = 0;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kGetOverworldBackgroundMusic,
                            ADDRESS_GET_OVERWORLD_BACKGROUND_MUSIC,
                            (uptr)GetBackgroundMusic);
  }

  STATIC_INLINE void Patch() {
#ifdef KAIZO
    kaizo::PatchOverworld();
#endif

    HookManager::Enable(HookID::kGetEncounterPokemon);
    HookManager::ForceEnable(HookID::kCheckAppRequest);

    // Simulate a button press
    WRITE(vu32, 0x00715C48, 0xE1A00000);
  }

  static u32
  GetBackgroundMusic(u32 sound_manager, u32 map_id, u32 player_form) {
    auto& instance = GetInstance();
    if (instance.freeze_background_music) {
      return (1 << 16) + instance.background_music; // NO SOUND
    }
    return HookManager::Call<u32>(HookID::kGetOverworldBackgroundMusic,
                                  sound_manager, map_id, player_form);
  }
};
} // namespace feature