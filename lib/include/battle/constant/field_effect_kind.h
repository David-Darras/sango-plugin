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
 * @file field_effect_kind.h
 * @brief The effects that act on the full battlefield.
 */

#pragma once
#include <types.h>

namespace battle {
/// An effect on the full battlefield: weather, Trick Room, a terrain...
enum class FieldEffectKind : u8 {
  kWeather,
  kTrickRoom,
  kGravity,
  kImprison, ///< The opponents cannot use the moves that the user knows.
  kWaterSport,
  kMudSport,
  kWonderRoom,
  kMagicRoom,
  kIonDeluge, ///< The Normal moves become Electric moves.
  kFairyLock,
  kTerrain, ///< A terrain is active. See TerrainKind.

  kCount,
};
}