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
 * @file script_kind.h
 * @brief The kinds of scripts.
 */

#pragma once

#include <types.h>

namespace script {

/// The kind of a script.
enum class ScriptKind : u8 {
  kNone = 0,
  kMap = 1, ///< A script of a map.
  kShared = 2, ///< A script that all the maps share.
  kAi = 3, ///< A script of the trainer AI.
  kMapInit = 4, ///< A script that runs when a map loads.
};

} // namespace script
