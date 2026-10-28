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
 * @file shiny_entry.h
 * @brief A bigger star effect when a shiny Pokémon comes out of its Poke Ball.
 */

#pragma once

#include "common.h"

namespace battle {
/**
 * @brief Adds star bursts and a flash of all the hues to the shiny entry
 *        animation (ORAS only).
 *
 * It uses the system of the move animations. A change of the setting applies
 * at the next battle.
 */
class ShinyEntry {
  MAKE_SINGLETON(ShinyEntry)
public:
  bool is_enabled = false;

  static void Initialize();

  /// Makes the animation file be built again. Call it after a change.
  static void Refresh();
};
} // namespace battle
