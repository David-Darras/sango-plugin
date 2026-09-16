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

#include "overworld/patch/field.h"
#include "core/hook_manager.h"
#include "overworld/patch/day_care.h"
#include "overworld/patch/gift_pokemon.h"
#include "overworld/patch/run_animation.h"
#include "overworld/patch/static_randomizer.h"
#include "overworld/patch/trade.h"
#include "overworld/native/map_manager.h"
#include "system/native/sound.h"
#include "ui/log_application.h"

namespace overworld {

void Field::Initialize() {
  core::HookManager::Initialize(HookId::kGetOverworldBackgroundMusic,
                          address::kGetOverworldBackgroundMusic,
                          (uptr)GetBackgroundMusic);
  core::HookManager::Initialize(HookId::kOverworldUpdateZone,
                          address::kUpdateZone,
                          (uptr)UpdateZone, false);
}

void Field::UpdateZone(MapManager* manager) {
  auto& next_map_id = manager->GetNextMapId();
  static s32 state = 0;
  if (next_map_id != MapManager::kNoMap && state == 0) {
    ui::LogApplication::Print(u"New Zone: %u", next_map_id);
    state = 1;
  }
  if (next_map_id == MapManager::kNoMap && state == 1) {
    state = 0;
  }
  core::HookManager::Call<void>(HookId::kOverworldUpdateZone, manager);
}

void Field::PatchLoad() {
  MEMORY_SCOPE(sys::address::kMemoryRegionGameCode, 0xF1000);
  auto& feat = GetInstance();
  if (feat.on_field_load != nullptr) feat.on_field_load();

  DayCare::PatchLoad();
  GiftPokemon::PatchLoad();
  StaticRandomizer::PatchLoad();
  Trade::PatchLoad();

  core::HookManager::Enable(HookId::kGetEncounterPokemon);
  core::HookManager::ForceEnable(HookId::kCheckAppRequest);
  core::HookManager::ForceEnable(HookId::kOverworldUpdateZone);
  core::HookManager::ForceEnable(HookId::kGetPlayerMovement);
  RunAnimation::PatchLoad();

  WRITE32(address::kSimulateButtonPress, 0xE1A00000);
}

u32 Field::GetBackgroundMusic(u32 sound_manager, u32 map_id, u32 player_form) {
  auto& instance = GetInstance();
  if (instance.freeze_background_music) {
    return sys::Sound::kBankBackgroundMusic +
           static_cast<u32>(instance.background_music);
  }
  return core::HookManager::Call<u32>(HookId::kGetOverworldBackgroundMusic,
                                sound_manager, map_id, player_form);
}

} // namespace overworld
