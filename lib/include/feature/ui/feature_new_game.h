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
#include "feature/core/hook_manager.h"
#include "game/constant/map.h"
#include "game/core/data_manager.h"
#include "game/overworld/character_placement.h"
#include "game/savedata/trainer_status.h"

namespace feature {
class NewGame {
  MAKE_SINGLETON(NewGame)

public:
  bool skip_intro = false;
  const c16* player_name = u"Sango";
  Gender player_gender = Gender::kMale;
  MapId start_zone = MapId::kNone;
  s16 start_tile_x = -1;
  s16 start_tile_z = -1;
  overworld::Facing start_facing = overworld::Facing::kInvalid;
  void (*on_new_game)() = nullptr;

  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kOverworldSetDefaultPosition,
                            ADDRESS_OVERWORLD_SET_DEFAULT_POSITION,
                            (uptr)OverworldSetDefaultPositionHook);
  }

  static void OnProcessUpdate(uptr vtable) {
    auto& feat = GetInstance();
    if (!feat.fade_in_pending_ || vtable != ADDRESS_OVERWORLD_VTABLE) return;
    feat.fade_in_pending_ = false;
    ((void (*)(u32, u32))ADDRESS_FADE_REQUEST_IN)(kBothScreens, kFadeFrames);
  }

  static void OnProcessLoad(uptr vtable) {
    auto& feat = GetInstance();
    if (feat.fade_in_pending_ && vtable == ADDRESS_OVERWORLD_VTABLE &&
        feat.start_facing != overworld::Facing::kInvalid) {
      game::DataManager::GetInstance().GetPlayerDirection() =
          static_cast<u16>(feat.start_facing);
    }
    if (!feat.skip_intro || vtable != ADDRESS_INTRODUCTION_VTABLE) return;
    const uptr slot = vtable + kUpdateSlot * sizeof(uptr);
    if (READ32(slot) == (u32)&FinishIntro) return;
    MEMORY_SCOPE(slot, sizeof(uptr));
    WRITE32(slot, (u32)&FinishIntro);
  }

private:
  static constexpr u32 kUpdateSlot = 3;
  static constexpr s32 kResultFinish = 1;
  static constexpr u32 kBothScreens = 2;
  static constexpr u32 kFadeFrames = 30;
  static constexpr float kTileSize = 18.0f;

  static s32 FinishIntro(void* process, void* manager) {
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

  static void OverworldSetDefaultPositionHook(void* location, u32 zone_id,
                                              void* heap) {
    auto& feat = GetInstance();
    if (feat.start_zone != MapId::kNone &&
        zone_id == static_cast<u32>(MapId::kInsideOfTruck)) {
      zone_id = static_cast<u32>(feat.start_zone);
    }
    HookManager::Call<void>(HookID::kOverworldSetDefaultPosition, location,
                            zone_id,
                            heap);
    if (zone_id != static_cast<u32>(feat.start_zone)) return;
    if (feat.start_facing != overworld::Facing::kInvalid) {
      *(u8*)((uptr)location + 6) = static_cast<u8>(feat.start_facing);
    }
    if (feat.start_tile_x < 0 || feat.start_tile_z < 0) return;
    auto* position = (Vec3*)((uptr)location + 16);
    position->x = feat.start_tile_x * kTileSize + kTileSize / 2;
    position->z = feat.start_tile_z * kTileSize + kTileSize / 2;
  }

  bool fade_in_pending_ = false;
};
}