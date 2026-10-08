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
 * @file move_app_input.h
 * @brief The input of the move applications (Move Reminder, Move Deleter, move tutor).
 */

#pragma once

#include "core/types.h"
#include "pokemon/constant/move.h"

namespace savedata {
struct PokemonParam;
}

namespace core {

/// The data that the Move Deleter, the Move Reminder and the move tutor
/// applications read when they start.
struct MoveAppInput {
  savedata::PokemonParam* pokemon;
  MoveId move_id;
  bool delete_move;
  u8 move_index;
};

} // namespace core
