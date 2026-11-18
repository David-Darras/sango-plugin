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
 * @file special_character.h
 * @brief The values that the scripts use for the special character.
 */

#pragma once

#include <types.h>

namespace overworld {

/// The special character, as a script writes it.
enum class SpecialCharacter : s16 {
  kTalkTarget = -1, ///< The character that the player talks to.
  kPlayer = 255,
  kScriptCharacter1 = 254, ///< Character added by the script (first).
  kScriptCharacter2 = 253, ///< Character added by the script (second).
  kScriptCharacter3 = 252, ///< Character added by the script (third).
  kNoCharacter = 240,
};

} // namespace overworld
