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
 * @file move_tutor_kind.h
 * @brief The values that the scripts use for the move tutor mode.
 */

#pragma once

#include <types.h>

namespace script {

/// The move tutor mode, as a script writes it.
enum class MoveTutorKind : u8 {
  kDragonMoves = 0,
  kUltimateMoves = 1,
  kCombinedMoves = 2,
  kDragonAscent = 3,
  kBattleShop = 4,
};

} // namespace script
