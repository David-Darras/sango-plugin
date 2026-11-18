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
 * @file amie_icon.h
 * @brief The values that the scripts use for the pokémon-Amie icon.
 */

#pragma once

#include <types.h>

namespace script {

/// The pokémon-Amie icon, as a script writes it.
enum class AmieIcon : u8 {
  kTap = 0,
  kHit = 1,
  kPointingFingerStill = 2, ///< Pointing finger (still).
  kPointingFingerAnimated = 3, ///< Pointing finger (animated).
  kOpenHandStill = 4, ///< Open hand (still).
  kOpenHandAnimated = 5, ///< Open hand (animated).
  kElectricShock = 6,
  kBeckoningHandStill = 7, ///< Beckoning hand (still).
  kBeckoningHandAnimated = 8, ///< Beckoning hand (animated).
  kBeckoningHandFade30Frames = 9, ///< Beckoning hand (fade of 30 frames).
  kBeckoningHandFade15Frames = 10, ///< Beckoning hand (fade of 15 frames).
  kWaveringIcon = 11,
  kIceIcon = 12,
  kScribbleIcon = 13,
  kBurntHand = 14,
  kHandHurtByThorns = 15,
  kMark1 = 16,
  kMark2 = 17,
  kMark3 = 18,
  kMark4 = 19,
  kMark5 = 20,
  kGhostType = 21,
};

} // namespace script
