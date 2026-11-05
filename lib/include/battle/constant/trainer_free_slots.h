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
 * @file trainer_free_slots.h
 * @brief The trainers of the game that nothing uses.
 *
 * These trainers have no name, no map and a Zigzagoon of level 5. Sango Maker
 * gives them to the new trainers of trainers.txt. The list comes from
 * tools/export_trainers.py (see its --free-slots option).
 */

#pragma once

#include <types.h>

namespace battle {

/// The numbers of the empty trainers of the game.
constexpr u16 kFreeTrainerSlots[] = {
    51, 52, 61, 70, 71, 72, 73, 75, 82, 84,
    88, 93, 101, 116, 117, 121, 122, 127, 128, 160,
    161, 162, 164, 166, 168, 169, 170, 171, 172, 183,
    188, 192, 214, 216, 218, 220, 224, 229, 230, 245,
    246, 260, 265, 277, 278, 303, 304, 313, 345, 347,
    349, 350, 351, 381, 382, 383, 384, 440, 448, 449,
    450, 451, 452, 455, 456, 464, 482, 483, 487, 488,
    489, 490, 491, 493, 498, 499, 500, 501, 509, 510,
    536, 537, 566, 587, 588, 589, 590, 612, 630, 665,
    666, 748, 749, 761, 762, 773, 797, 805, 825, 826,
};

/// The number of the empty trainers.
constexpr u32 kFreeTrainerSlotCount =
    sizeof(kFreeTrainerSlots) / sizeof(kFreeTrainerSlots[0]);

} // namespace battle
