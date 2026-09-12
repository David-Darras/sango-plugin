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

#include "game/core/manager.h"

namespace overworld {
class WeatherManager {
  SINGLETON(WeatherManager)
public:
  STATIC_INLINE WeatherManager& GetInstance() {
    return game::Manager::GetInstance().GetWeatherManager();
  }

  INLINE u8& GetCurrentWeather() { return *(u8*)((uptr)this + 0x1C); }

  INLINE u8& GetRequestedWeather() { return *(u8*)((uptr)this + 0x1E); }

  INLINE void SetMapId(u16 map_id) { WRITE16((uptr)this + 0x14, map_id); }
  INLINE void SetWeather(u8 weather) { WRITE8((uptr)this + 0x1E, weather); }

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
    WRITE32((uptr)this + 48, sync);
  }
};
} // namespace overworld