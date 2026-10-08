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
 * @file address.h
 * @brief Includes the addresses of all the domains.
 *
 * Each domain has its own file `<domain>/address.h`. Each address uses
 * GAME_ADDRESS(xy, oras).
 *
 * @see docs/concepts/hooks-and-addresses.md
 */

#pragma once

#include "core/address.h"
#include "system/address.h"
#include "battle/address.h"
#include "overworld/address.h"
#include "pokemon/address.h"
#include "renderer/address.h"
#include "ui/address.h"
