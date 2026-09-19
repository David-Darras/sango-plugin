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

namespace overworld {
#ifdef GAME_XY
enum class Weather : u8 {
  kSunny = 0,
  kSunny2 = 1,
  kSunny3 = 2,
  kSunny4 = 3,
  kSunny5 = 4,
  kSunny6 = 5,
  kSunny7 = 6,
  kSunny8 = 7,
  kSunny9 = 8,
  kSunny10 = 9,
  kSunny11 = 10,
  kSunny12 = 11,
  kSunny13 = 12,
  kCloudy = 13,
  kLightRain = 14,
  kRainy = 15,
  kHeavyRain = 16,
  kLightSnow = 17,
  kSnow = 18,
  kHeavySnow = 19,
  kSnowstorm = 20,
  kDiamondDust = 21,
  kSunnyWind = 22,
  kNone = 23,
};
#else
enum class Weather : u8 {
  kSunny = 0,
  kRainy = 1,
  kThunderstorm = 2,
  kMisty = 3,
  kAsh = 4,
  kSandstorm = 5,
  kCloudy = 6,
  kStormy = 7,
  kDry = 8,
  kNone = 9,
};
#endif
} // namespace overworld