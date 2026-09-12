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

#include <types.h>

enum class BattleWeather : u8 {
  kNone = 0,
  kHarshSunlight = 1,
  kRain = 2,
  kHail = 3,
  kSandstorm = 4,
  kHeavyRain = 5,
  kExtremelyHarshSunlight = 6,
  kStrongWinds = 7,
  kInvalid = 8,
};

enum class OverworldWeather : u8 {
  kSunny = 0,
  kRainy = 1,
  kThunderstorm = 2,
  kMisty = 3,
  kAsh = 4,
  kSandstorm = 5,
  kCloudy = 6,
  kStormy = 7,
  kDry = 8,
};
