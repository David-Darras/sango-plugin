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
 * @file face_game_bubble_animation.h
 * @brief The values that the scripts use for the face game bubble animation.
 */

#pragma once

#include <types.h>

namespace script {

/// The face game bubble animation, as a script writes it.
enum class FaceGameBubbleAnimation : u8 {
  kAppears = 0,
  kWaits = 1,
  kForcedEnd = 2,
  kRightAnswer = 3,
};

} // namespace script
