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

enum class EvolutionMethod : u8 {
  kNone = 0, // Does not evolve
  kFriendship = 1, // Level up with high Friendship
  kFriendshipDay = 2, // Level up with high Friendship during the day
  kFriendshipNight = 3, // Level up with high Friendship at night
  kLevelUp = 4, // Level up
  kTrade = 5, // Trade
  kTradeItem = 6, // Trade holding a specific item
  kTradeSpecific = 7, // Trade for a specific Pokémon (Karrablast/Shelmet)
  kItem = 8, // Use an item (Evolution Stones)
  kStatAtkGtDef = 9, // Level up with Attack > Defense (Hitmonlee)
  kStatAtkEqDef = 10, // Level up with Attack = Defense (Hitmontop)
  kStatAtkLtDef = 11, // Level up with Attack < Defense (Hitmonchan)
  kPersonalityEven = 12, // Level up with an even personality value (Silcoon)
  kPersonalityOdd = 13, // Level up with an odd personality value (Cascoon)
  kNinjask = 14, // Unique: Nincada to Ninjask
  kShedinja = 15, // Unique: Nincada to Shedinja (Empty slot + Poke Ball)
  kBeauty = 16, // Level up with high Beauty (Feebas)
  kItemMale = 17, // Use an item on a male Pokémon
  kItemFemale = 18, // Use an item on a female Pokémon
  kHoldItemDay = 19, // Level up holding an item during the day
  kHoldItemNight = 20, // Level up holding an item at night
  kKnowMove = 21, // Level up knowing a specific move
  kPartyPokemon = 22, // Level up with a specific Pokémon in the party
  kMaleOnly = 23, // Male levels up
  kFemaleOnly = 24, // Female levels up
  kLocationMagnetic = 25, // Level up in a special magnetic field (New Mauville)
  kLocationMossRock = 26, // Level up near a Moss Rock (Petalburg Woods)
  kLocationIceRock = 27, // Level up near an Ice Rock (Shoal Cave)
  k3dsUpsideDown = 28, // Level up holding the 3DS upside down (Inkay)
  kAffectionFairyMove = 29, // Level up with high Affection + Fairy-type move
  kPartyDarkType = 30, // Level up with a Dark-type Pokémon in the party
  kWeatherRain = 31, // Level up during rain in the overworld
  kDay = 32, // Level up during the day
  kNight = 33, // Level up at night
  kFemaleFormChange = 34, // Female levels up and changes form
};
