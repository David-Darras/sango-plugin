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
 * @file camera_state.h
 * @brief The modes of the camera of the plugin.
 */

#pragma once

#include <types.h>

namespace overworld {

/// A mode of the camera of the plugin.
enum class CameraState : u8 {
  kIdle, ///< The camera of the game.
  kTps, ///< Third-person view, behind the player.
  kRotate, ///< The camera turns around the player.
  kTop, ///< View from above.
  kFpv, ///< First-person view.
  kFree, ///< Free camera: the player moves it.
};

} // namespace overworld
