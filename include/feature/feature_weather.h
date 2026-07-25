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
#include "feature_nuzlocke.h"
#include "hook_manager.h"
#include "game/overworld/weather_manager.h"

namespace feature {
class WeatherManager {
  MAKE_SINGLETON(WeatherManager)
public:
  STATIC_INLINE void Initialize() {
    // Don't update weather
    ARM_NOP(ADDRESS_UPDATE_AREA_WEATHER + 0xC);
    ARM_NOP(ADDRESS_UPDATE_ZONE_WEATHER + 0x4);
  }
};
}