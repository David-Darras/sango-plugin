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
#include "feature_light.h"
#include "feature_nuzlocke.h"
#include "feature_overworld_model.h"
#include "feature/hook_manager.h"
#include "game/constant/map.h"
#include "game/constant/weather.h"
#include "game/overworld/weather_manager.h"
#include "ui/log_application.h"

namespace feature {
struct Overworld {
  MAKE_SINGLETON(Overworld)
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kGetOverworldBackgroundMusic,
                            ADDRESS_GET_OVERWORLD_BACKGROUND_MUSIC,
                            (uptr)GetBackgroundMusic);
  }

  static u32
  GetBackgroundMusic(u32 sound_manager, u32 map_id, u32 player_form) {
    u32 result = HookManager::Call<u32>(HookID::kGetOverworldBackgroundMusic,
                                        sound_manager, map_id, player_form);
    ui::LogApplication::Print(u"map=%u", map_id);

    result = Nuzlocke::SetBackgroundMusic(map_id, result);
    Nuzlocke::SetCamera(map_id);
    Nuzlocke::SetLight(map_id);
    Nuzlocke::SetWeather(map_id);

    return result;
  }
};
} // namespace feature