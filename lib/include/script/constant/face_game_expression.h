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
 * @file face_game_expression.h
 * @brief The values that the scripts use for the face game expression.
 */

#pragma once

#include <types.h>

namespace script {

/// The face game expression, as a script writes it.
enum class FaceGameExpression : u8 {
  kStraightFace = 0,
  kSlightSmile = 1,
  kSmileSmall = 2, ///< Smile (small).
  kSmileBig = 3, ///< Smile (big).
  kEyesOpen = 4,
  kEyesHalfClosed = 5,
  kEyesClosed = 6,
  kWinkLeft = 7, ///< Wink (left).
  kWinkRight = 8, ///< Wink (right).
  kPursedMouth = 9,
  kBigMouth = 10,
  kEyesClosedAndBigMouth = 11,
  kEyesClosedAndPursedMouth = 12,
  kEyesOpenAndBigMouth = 13,
  kEyesOpenAndPursedMouth = 14,
};

} // namespace script
