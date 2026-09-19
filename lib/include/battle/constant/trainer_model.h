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

#include "core/game.h"

namespace battle {
#ifdef GAME_XY
enum class TrainerModelId : u8 {
  kSerena = 0,
  kCalem = 1,
  kShauna = 2,
  kTierno = 3,
  kTrevor = 4,
  kSycamore = 5,
  kMalva = 6,
  kLysandre = 7,
  kFlareAdminMale = 8,
  kFlareAdminFemale = 9,
  kAliana = 10,
  kXerosic = 11,
  kFlareGruntMale = 12,
  kFlareGruntFemale = 13,
  kCelosia = 14,
  kBryony = 15,
  kMable = 16,
  kAz = 17,
  kLysandreFinal = 18,
  kHolocaster = 19,
  kRivalCalem = 20,
  kRivalSerena = 21,
  kPupilMale = 22,
  kPupilFemale = 23,
  kCount = 24,
};
#else
enum class TrainerModelId : u8 {
  kSerena = 0,
  kCalem = 1,
  kMalva = 2,
  kHolocaster = 3,
  kMay = 4,
  kBrendan = 5,
  kWally = 6,
  kWallyAlt = 7,
  kSteven = 8,
  kMaxie = 9,
  kArchie = 10,
  kZinnia = 11,
  kShauna = 12,
  kTierno = 13,
  kTrevor = 14,
  kCount = 15,
};
#endif
} // namespace battle

using battle::TrainerModelId;
