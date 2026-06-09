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

#define EVOLUTION_METHOD_NONE                  0  // Does not evolve
#define EVOLUTION_METHOD_FRIENDSHIP            1  // Level up with high Friendship
#define EVOLUTION_METHOD_FRIENDSHIP_DAY        2  // Level up with high Friendship during the day
#define EVOLUTION_METHOD_FRIENDSHIP_NIGHT      3  // Level up with high Friendship at night
#define EVOLUTION_METHOD_LEVEL_UP              4  // Level up
#define EVOLUTION_METHOD_TRADE                 5  // Trade
#define EVOLUTION_METHOD_TRADE_ITEM            6  // Trade holding a specific item
#define EVOLUTION_METHOD_TRADE_SPECIFIC        7  // Trade for a specific Pokémon (Karrablast/Shelmet)
#define EVOLUTION_METHOD_ITEM                  8  // Use an item (Evolution Stones)
#define EVOLUTION_METHOD_STAT_ATK_GT_DEF       9  // Level up with Attack > Defense (Hitmonlee)
#define EVOLUTION_METHOD_STAT_ATK_EQ_DEF      10  // Level up with Attack = Defense (Hitmontop)
#define EVOLUTION_METHOD_STAT_ATK_LT_DEF      11  // Level up with Attack < Defense (Hitmonchan)
#define EVOLUTION_METHOD_PERSONALITY_EVEN     12  // Level up with an even personality value (Silcoon)
#define EVOLUTION_METHOD_PERSONALITY_ODD      13  // Level up with an odd personality value (Cascoon)
#define EVOLUTION_METHOD_NINJASK              14  // Unique: Nincada to Ninjask
#define EVOLUTION_METHOD_SHEDINJA             15  // Unique: Nincada to Shedinja (Empty slot + Poke Ball)
#define EVOLUTION_METHOD_BEAUTY               16  // Level up with high Beauty (Feebas)
#define EVOLUTION_METHOD_ITEM_MALE            17  // Use an item on a male Pokémon
#define EVOLUTION_METHOD_ITEM_FEMALE          18  // Use an item on a female Pokémon
#define EVOLUTION_METHOD_HOLD_ITEM_DAY        19  // Level up holding an item during the day
#define EVOLUTION_METHOD_HOLD_ITEM_NIGHT      20  // Level up holding an item at night
#define EVOLUTION_METHOD_KNOW_MOVE            21  // Level up knowing a specific move
#define EVOLUTION_METHOD_PARTY_POKEMON        22  // Level up with a specific Pokémon in the party
#define EVOLUTION_METHOD_MALE_ONLY            23  // Male levels up
#define EVOLUTION_METHOD_FEMALE_ONLY          24  // Female levels up
#define EVOLUTION_METHOD_LOCATION_MAGNETIC    25  // Level up in a special magnetic field (New Mauville)
#define EVOLUTION_METHOD_LOCATION_MOSS_ROCK   26  // Level up near a Moss Rock (Petalburg Woods)
#define EVOLUTION_METHOD_LOCATION_ICE_ROCK    27  // Level up near an Ice Rock (Shoal Cave)
#define EVOLUTION_METHOD_3DS_UPSIDE_DOWN      28  // Level up holding the 3DS upside down (Inkay)
#define EVOLUTION_METHOD_AFFECTION_FAIRY_MOVE 29  // Level up with high Affection + Fairy-type move
#define EVOLUTION_METHOD_PARTY_DARK_TYPE      30  // Level up with a Dark-type Pokémon in the party
#define EVOLUTION_METHOD_WEATHER_RAIN         31  // Level up during rain in the overworld
#define EVOLUTION_METHOD_DAY                  32  // Level up during the day
#define EVOLUTION_METHOD_NIGHT                33  // Level up at night
#define EVOLUTION_METHOD_FEMALE_FORM_CHANGE   34  // Female levels up and changes form