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
 * @file static_encounter_kind.h
 * @brief The kinds of static encounters.
 */

#pragma once

#include <types.h>

namespace overworld {

/// How the game runs a static encounter.
enum class StaticEncounterKind : u16 {
  kNormal = 0, ///< A regular random wild encounter, with no visible sprite
  kOverworldEncounter = 1, ///< A regular Pokémon visible on the map
  kLegendary = 2,
  ///< A legendary Pokémon: a special message, and one battle only
  kLegendaryEndless = 3,
  ///< Like kLegendary, but the Pokémon stays: the player can battle it again
  kRescueEvent = 4,
  ///< The first battle, when the player saves the professor from a
  ///< Poochyena. The player cannot run, and the messages are different.
  kLegendaryUnlosable = 5,
  ///< Like kLegendary, but the player cannot lose the battle
};

} // namespace overworld

using overworld::StaticEncounterKind;
