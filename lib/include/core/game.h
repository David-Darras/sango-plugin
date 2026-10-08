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
 * @file game.h
 * @brief Selects the addresses and the constants of the target game.
 *
 * The Makefile defines GAME_ORAS (Omega Ruby and Alpha Sapphire) or
 * GAME_XY (X and Y). Each address of the library has one value for each
 * game. The value 0 means that nobody found the address for this game yet.
 *
 * @code
 * constexpr uptr kHealTeam = GAME_ADDRESS(0x0039F2E4, 0x003B5FC0);
 * //                                      X v1.5      Alpha Sapphire v1.4
 * @endcode
 */

#pragma once

#if defined(GAME_XY) && defined(GAME_ORAS)
#error "GAME_XY and GAME_ORAS are exclusive"
#elif defined(GAME_XY)
/// Selects the address of the target game: the first value for X/Y.
#define GAME_ADDRESS(xy, oras) (xy)
/// Selects a constant of the target game: the first value for X/Y.
#define GAME_CONSTANT(xy, oras) (xy)
#elif defined(GAME_ORAS)
/// Selects the address of the target game: the second value for ORAS.
#define GAME_ADDRESS(xy, oras) (oras)
/// Selects a constant of the target game: the second value for ORAS.
#define GAME_CONSTANT(xy, oras) (oras)
#else
#error "Build with -DGAME_XY or -DGAME_ORAS"
#endif
