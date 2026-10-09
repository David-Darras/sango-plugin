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
 * @file gender_roll.h
 * @brief How a table of the game selects a gender.
 */

#pragma once

#include <types.h>

namespace pokemon {

/// How an entry of a table (gift, static encounter...) selects the gender.
enum class GenderRoll : u8 {
  kRandom = 0,
  kMale = 1,
  kFemale = 2,
};

} // namespace pokemon

using pokemon::GenderRoll;
