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
 * @file fade_in_style.h
 * @brief The values that the scripts use for the fade-in.
 */

#pragma once

#include <types.h>

namespace script {

/// The fade-in, as a script writes it.
enum class FadeInStyle : u8 {
  kBlack = 0,
  kWhite = 1,
  kBlackFromCaptureScreen = 2,
  kWhiteFromCaptureScreen = 3,
  kCaptureScreen = 4,
  kCircleThatShrinks = 5,
  kBall = 6,
  kUp = 7,
  kDown = 8,
  kLeft = 9,
  kRight = 10,
  kDiamond = 11,
};

} // namespace script
