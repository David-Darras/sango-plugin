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
 * @file shop_type.h
 * @brief The types of shops.
 */

#pragma once

#include <types.h>

namespace pokemon {

/// The type of a shop: what it sells and what the player pays with.
enum class ShopType : u32 {
  kNormal, ///< A normal Poké Mart. The player pays with money.
  kBattlePoint, ///< Pays with Battle Points (BP).
  kBattlePointMove, ///< A move tutor that takes Battle Points.
  kPokeMiles, ///< Pays with Poké Miles.
  kSecretBaseGoods, ///< Sells Secret Base decorations.
};

} // namespace pokemon

using pokemon::ShopType;
