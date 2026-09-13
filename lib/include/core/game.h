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

// Which game the plugin is built for: the Makefile passes -DGAME_ORAS
// (Omega Ruby / Alpha Sapphire) or -DGAME_XY (X / Y). Every address of the
// library is declared once for each game through GAME_ADDRESS; 0 marks an
// address not found yet in that game.
#if defined(GAME_XY) && defined(GAME_ORAS)
#error "GAME_XY and GAME_ORAS are exclusive"
#elif defined(GAME_XY)
#define GAME_ADDRESS(xy, oras) (xy)
#elif defined(GAME_ORAS)
#define GAME_ADDRESS(xy, oras) (oras)
#else
#error "Build with -DGAME_XY or -DGAME_ORAS"
#endif
