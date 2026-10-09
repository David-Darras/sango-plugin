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
 * @file device_patch.h
 * @brief Hides the buttons from the game when the menu of the plugin is open.
 */

#pragma once

#include "common.h"

namespace core {

/// Hides the buttons and the touch screen from the game. The hooks are in
/// device_patch.cc: the plugin installs them at the start.
struct DevicePatch {
  MAKE_SINGLETON(DevicePatch)
  bool use_redirection = false; ///< true: the game sees no button. The menu sets it when it opens.
};

} // namespace core
