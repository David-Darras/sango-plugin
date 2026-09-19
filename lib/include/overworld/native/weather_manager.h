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

#include "core/native/game_manager.h"
#include "overworld/constant/map.h"
#include "overworld/constant/weather.h"

namespace overworld {
class WeatherManager {
  SINGLETON(WeatherManager)
public:
  STATIC_INLINE WeatherManager& GetInstance() {
    return core::GameManager::GetInstance().GetWeatherManager();
  }

  INLINE Weather& GetCurrentWeather() {
    return *(Weather*)((uptr)this + GAME_CONSTANT(0x34, 0x1C));
  }

  INLINE Weather& GetRequestedWeather() {
    return *(Weather*)((uptr)this + GAME_CONSTANT(0x36, 0x1E));
  }

  INLINE void SetMapId(MapId map_id) {
    *(MapId*)((uptr)this + GAME_CONSTANT(0x24, 0x14)) = map_id;
  }
#ifdef GAME_XY
  static constexpr u32 kNoWeatherZone = 29;
  INLINE u32 GetZoneListIndex() { return READ32((uptr)this + 0x18); }
  INLINE u32& IsForced() { return *(u32*)((uptr)this + 0x2C); }
  INLINE void ForceWeather(Weather weather, u8 season) {
    WRITE32((uptr)this + 0x2C, 1);
    WRITE8((uptr)this + 0x3D, static_cast<u8>(weather));
    WRITEF((uptr)this + 0x40, 10.0f);
    WRITE8((uptr)this + 0x44, season);
  }
  INLINE void ReleaseWeather() { WRITE32((uptr)this + 0x2C, 0); }
  INLINE void SetWeather(Weather weather) { ForceWeather(weather, 0); }
#else
  INLINE void SetWeather(Weather weather) { GetRequestedWeather() = weather; }
#endif

  INLINE void SetSkybox(bool is_enabled) {
    uptr addr = READ32((uptr)this);
    WRITE32(addr + 64, is_enabled);
  }

  INLINE void SetWeatherEffects(bool is_enabled) {
    uptr addr = READ32((uptr)this + 16);
    if (!READ32(addr + 12)) {
      WRITE8(addr + 33, is_enabled);
    }
  }

  INLINE void Synchronize(bool sync) {
#ifndef GAME_XY
    WRITE32((uptr)this + 48, sync);
#endif
  }
};
} // namespace overworld
