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
 * @file overworld.cc
 * @brief The overworld: a callback when it loads, and the background music.
 *
 * The declarations are in overworld/patch/overworld.h.
 */

#include "overworld/patch/overworld.h"
#include "overworld/patch/auto_surf.h"
#include "core/hook_manager.h"
#include "overworld/patch/day_care.h"
#include "overworld/patch/gift_pokemon.h"
#include "overworld/patch/run_animation.h"
#include "overworld/patch/static_randomizer.h"
#include "overworld/patch/trade.h"
#include "system/native/sound.h"

namespace overworld {

void Overworld::Initialize() {
  core::HookManager::Initialize(HookId::kGetOverworldBackgroundMusic,
                          address::kGetOverworldBackgroundMusic,
                          (uptr)GetBackgroundMusic);
}

void Overworld::PatchLoad() {
  MEMORY_SCOPE(sys::address::kMemoryRegionCro, 0xF1000);
  auto& feat = GetInstance();
  if (feat.on_overworld_load != nullptr) feat.on_overworld_load();

  DayCare::PatchLoad();
  GiftPokemon::PatchLoad();
  StaticRandomizer::PatchLoad();
  Trade::PatchLoad();

  core::HookManager::Enable(HookId::kGetEncounterPokemon);
  core::HookManager::ForceEnable(HookId::kCheckAppRequest);
  core::HookManager::ForceEnable(HookId::kGetPlayerMovement);
  RunAnimation::PatchLoad();
  AutoSurf::PatchLoad();

  if (address::kSimulateButtonPress) {
    WRITE32(address::kSimulateButtonPress, 0xE1A00000);
  }
}

u32 Overworld::GetBackgroundMusic(u32 sound_manager, u32 map_id, u32 player_form) {
  auto& instance = GetInstance();
  if (instance.freeze_background_music) {
    return sys::Sound::kBankBackgroundMusic +
           static_cast<u32>(instance.background_music);
  }
  return core::HookManager::Call<u32>(HookId::kGetOverworldBackgroundMusic,
                                sound_manager, map_id, player_form);
}

} // namespace overworld
