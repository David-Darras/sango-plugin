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
 * @file evolution_method.h
 * @brief The methods of evolution.
 */

#pragma once

#include <types.h>


namespace pokemon {

/// How a Pokémon evolves. The comment of each value gives the condition.
enum class EvolutionMethod : u8 {
  kNone = 0, ///< The Pokémon does not evolve.
  kFriendship = 1, ///< Level up with a high friendship.
  kFriendshipDay = 2, ///< Level up with a high friendship during the day.
  kFriendshipNight = 3, ///< Level up with a high friendship at night.
  kLevelUp = 4, ///< Level up to a level.
  kTrade = 5, ///< Trade.
  kTradeItem = 6, ///< Trade while it holds an item.
  /// Trade for a specific Pokémon (Karrablast and Shelmet).
  kTradeSpecific = 7,
  kItem = 8, ///< Use an item (an evolution stone).
  kStatAtkGtDef = 9, ///< Level up with Attack higher than Defense (Hitmonlee).
  kStatAtkEqDef = 10, ///< Level up with Attack equal to Defense (Hitmontop).
  kStatAtkLtDef = 11, ///< Level up with Attack lower than Defense (Hitmonchan).
  kPersonalityEven = 12, ///< Level up with an even personality value (Silcoon).
  kPersonalityOdd = 13, ///< Level up with an odd personality value (Cascoon).
  kNinjask = 14, ///< Nincada evolves into Ninjask.
  /// Nincada gives Shedinja (a free party slot and a Poké Ball).
  kShedinja = 15,
  kBeauty = 16, ///< Level up with a high Beauty (Feebas).
  kItemMale = 17, ///< Use an item on a male Pokémon.
  kItemFemale = 18, ///< Use an item on a female Pokémon.
  kHoldItemDay = 19, ///< Level up during the day while it holds an item.
  kHoldItemNight = 20, ///< Level up at night while it holds an item.
  kKnowMove = 21, ///< Level up while it knows a move.
  kPartyPokemon = 22, ///< Level up with a specific Pokémon in the party.
  kMaleOnly = 23, ///< Level up, male only.
  kFemaleOnly = 24, ///< Level up, female only.
  kLocationMagnetic = 25, ///< Level up in a magnetic field (New Mauville).
  kLocationMossRock = 26, ///< Level up near a Moss Rock (Petalburg Woods).
  kLocationIceRock = 27, ///< Level up near an Ice Rock (Shoal Cave).
  k3dsUpsideDown = 28, ///< Level up with the 3DS upside down (Inkay).
  /// Level up with a high affection and a Fairy move.
  kAffectionFairyMove = 29,
  kPartyDarkType = 30, ///< Level up with a Dark Pokémon in the party.
  kWeatherRain = 31, ///< Level up in the rain of the overworld.
  kDay = 32, ///< Level up during the day.
  kNight = 33, ///< Level up at night.
  kFemaleFormChange = 34, ///< Level up, female only, with a form change.
};

} // namespace pokemon

using pokemon::EvolutionMethod;
