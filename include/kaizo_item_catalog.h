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

#include "common.h"
#include "game/constant/item.h"

// Single source of truth for the curated item sets that both the starting
// bag (kaizo_bag.cc) and the in-game shop (kaizo_shop.cc) hand out. Before
// this header existed, both files kept their own separate copy of these
// three lists; they had drifted into two different orderings (though the
// same items) and any future addition/removal had to be made twice to stay
// in sync.
namespace kaizo {
static constexpr u16 kStrategicItems[] = {
    ITEM_CHOICE_BAND, ITEM_CHOICE_SCARF,
    ITEM_CHOICE_SPECS, ITEM_LIFE_ORB,
    ITEM_LEFTOVERS, ITEM_FOCUS_SASH,
    ITEM_AIR_BALLOON, ITEM_ASSAULT_VEST,
    ITEM_ROCKY_HELMET, ITEM_SITRUS_BERRY,
    ITEM_POWER_HERB, ITEM_FLAME_ORB,
    ITEM_EJECT_BUTTON, ITEM_SCOPE_LENS,
    ITEM_TOXIC_ORB, ITEM_GRIP_CLAW
};

static constexpr u16 kEvolutionStones[] = {
    ITEM_SUN_STONE, ITEM_MOON_STONE,
    ITEM_FIRE_STONE, ITEM_THUNDER_STONE,
    ITEM_WATER_STONE, ITEM_LEAF_STONE,
    ITEM_SHINY_STONE, ITEM_DUSK_STONE,
    ITEM_DAWN_STONE, ITEM_OVAL_STONE,
};

static constexpr u16 kBalls[] = {
    ITEM_MASTER_BALL, ITEM_ULTRA_BALL,
    ITEM_GREAT_BALL, ITEM_POKE_BALL,
    ITEM_SAFARI_BALL, ITEM_NET_BALL,
    ITEM_DIVE_BALL, ITEM_NEST_BALL,
    ITEM_REPEAT_BALL, ITEM_TIMER_BALL,
    ITEM_LUXURY_BALL, ITEM_PREMIER_BALL,
    ITEM_DUSK_BALL, ITEM_HEAL_BALL,
    ITEM_QUICK_BALL, ITEM_CHERISH_BALL,
    ITEM_FAST_BALL, ITEM_LEVEL_BALL,
    ITEM_LURE_BALL, ITEM_HEAVY_BALL,
    ITEM_LOVE_BALL, ITEM_FRIEND_BALL,
    ITEM_MOON_BALL, ITEM_SPORT_BALL,
    ITEM_PARK_BALL, ITEM_DREAM_BALL,
};
} // namespace kaizo
