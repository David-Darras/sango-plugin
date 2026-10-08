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
 * @file status_overwrite_mode.h
 * @brief How a new status condition replaces an old one.
 */

#pragma once
#include <types.h>

namespace battle {
/// How a new status condition replaces an existing one.
enum class StatusOverwriteMode : u8 {
  kNone, ///< Cannot replace an existing status condition.
  kOverwriteBasicStatus, ///< Replaces a different major status condition.
  kForceOverwrite, ///< Replaces all status conditions.
};
}
