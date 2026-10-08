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
 * @file gender.h
 * @brief The genders of a Pokémon.
 */

#pragma once

#include <types.h>


namespace pokemon {

/// The gender of a Pokémon.
enum class Gender : u8 {
  kMale = 0,
  kFemale = 1,
  kUnknown = 2, ///< A Pokémon without a gender (for example Magnemite).
  kCount = 3,
};

} // namespace pokemon

using pokemon::Gender;
