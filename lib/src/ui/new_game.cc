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

#include "ui/patch/new_game.h"
#include "core/hook_manager.h"
#include "core/native/data_manager.h"
#include "savedata/native/trainer_status.h"

namespace ui {

void NewGame::Initialize() {
  core::HookManager::Initialize(HookId::kOverworldSetDefaultPosition,
                          overworld::address::kSetDefaultPosition,
                          (uptr)OverworldSetDefaultPositionHook);
}

void NewGame::OnProcessUpdate(uptr vtable) {
  auto& feat = GetInstance();
  if (!feat.fade_in_pending_ || vtable != overworld::address::kVtable) return;
  feat.fade_in_pending_ = false;
  ((void (*)(u32, u32))renderer::address::kFadeRequestIn)(kBothScreens, kFadeFrames);
}

void NewGame::OnProcessLoad(uptr vtable) {
  auto& feat = GetInstance();
  if (feat.fade_in_pending_ && vtable == overworld::address::kVtable &&
      feat.start_facing != overworld::Facing::kInvalid) {
    core::DataManager::GetInstance().GetPlayerDirection() = feat.start_facing;
  }
  if (!feat.skip_intro || vtable != core::address::kIntroductionVtable) return;
  const uptr slot = vtable + kUpdateSlot * sizeof(uptr);
  if (READ32(slot) == (u32)&FinishIntro) return;
  MEMORY_SCOPE(slot, sizeof(uptr));
  WRITE32(slot, (u32)&FinishIntro);
}

s32 NewGame::FinishIntro(void* process, void* manager) {
  auto& feat = GetInstance();
  auto& status = savedata::TrainerStatus::GetInstance();
  status.gender = feat.player_gender;
  u32 i = 0;
  for (; i < savedata::TrainerStatus::kPlayerNameLen - 1 &&
         feat.player_name[i] != u'\0'; i++) {
    status.name[i] = feat.player_name[i];
  }
  status.name[i] = u'\0';
  if (feat.on_new_game) feat.on_new_game();
  feat.fade_in_pending_ = true;
  return kResultFinish;
}

void NewGame::OverworldSetDefaultPositionHook(void* location, MapId zone_id,
                                              void* heap) {
  auto& feat = GetInstance();
  if (feat.start_zone != MapId::kNone && zone_id == MapId::kInsideOfTruck) {
    zone_id = feat.start_zone;
  }
  core::HookManager::Call<void>(HookId::kOverworldSetDefaultPosition, location,
                          zone_id, heap);
  if (zone_id != feat.start_zone) return;
  if (feat.start_facing != overworld::Facing::kInvalid) {
    READ(overworld::Facing, (uptr)location + kOffsetLocationFacing) =
        feat.start_facing;
  }
  if (feat.start_tile_x < 0 || feat.start_tile_z < 0) return;
  auto* position = (Vec3*)((uptr)location + kOffsetLocationPosition);
  position->x = feat.start_tile_x * kTileSize + kTileSize / 2;
  position->z = feat.start_tile_z * kTileSize + kTileSize / 2;
}

} // namespace ui
