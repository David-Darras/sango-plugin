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
#include "overworld/constant/facing.h"
#include "overworld/constant/map.h"

namespace ui {

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

  static void Initialize();
  static void OnProcessUpdate(uptr vtable);
  static void OnProcessLoad(uptr vtable);

private:
  static constexpr u32 kUpdateSlot = 3;
  static constexpr s32 kResultFinish = 1;
  static constexpr u32 kBothScreens = 2;
  static constexpr u32 kFadeFrames = 30;
  static constexpr float kTileSize = 18.0f;

  static s32 FinishIntro(void* process, void* manager);
  /// Layout of the game's default position, as reached from the hook.
  static constexpr uptr kOffsetLocationFacing = 6;
  static constexpr uptr kOffsetLocationPosition = 16;

  static void OverworldSetDefaultPositionHook(void* location, MapId zone_id,
                                              void* heap);

  bool fade_in_pending_ = false;
};

} // namespace ui
