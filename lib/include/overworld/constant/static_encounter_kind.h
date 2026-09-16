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

#pragma once

#include <types.h>

namespace overworld {

/// How the game runs a scripted encounter (StaticEncounter).
enum class StaticEncounterKind : u16 {
  kNormal = 0, ///< A regular random wild encounter, with no visible sprite
  kOverworldEncounter = 1, ///< A regular Pokémon visible on the map
  kLegendary = 2,
  ///< A legendary Pokémon - uses a distinct encounter message and only fights once
  kLegendaryEndless = 3,
  ///< Same as kLegendary, but stays on the map and can be re-fought after being defeated or caught
  kRescueEvent = 4,
  ///< The early-game scripted battle where the player saves the professor
  ///< from a wild Pokémon (a level 3 male Poochyena) - running away is
  ///< disabled and the messages differ
  kLegendaryUnlosable = 5,
  ///< Same as kLegendary, but the battle can't be lost
};

} // namespace overworld

using overworld::StaticEncounterKind;
