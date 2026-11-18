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
/**
 * @file contest_data.h
 * @brief The values that the scripts use for the contest data.
 */

#pragma once

#include <types.h>

namespace script {

/// The contest data, as a script writes it.
enum class ContestData : u8 {
  kTotalScore = 0,
  kRanking = 1,
  kConditionEntry = 2,
  kRankEntry = 3,
  kRankingLisia = 4,
  kRankingWallace = 5,
  kRankingOtherCharacters = 6,
};

} // namespace script
