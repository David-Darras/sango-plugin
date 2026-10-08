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
 * @file cheat_code_id.h
 * @brief The ids of the cheat codes of the plugin.
 */

#pragma once

#include <types.h>

namespace core {

/// The id of a cheat code. See core::CheatCodeManager.
enum class CheatCodeId : u32 {
  kNoclip, ///< The player walks through the walls.
  kSwarmMod, ///< The Pokémon of the party follow the player in a circle.
  kNoEncounter, ///< No wild Pokémon (an infinite Repel).
  kPokemonShop, ///< The normal Poké Marts sell Pokémon. See pokemon::CustomShop.
  kMax ///< The number of cheat codes.
};

} // namespace core

using core::CheatCodeId;
