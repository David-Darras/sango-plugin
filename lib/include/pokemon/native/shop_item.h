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
 * @file shop_item.h
 * @brief One item of a shop.
 */

#pragma once

#include "core/types.h"
#include "pokemon/constant/item.h"

namespace pokemon {

/// One item of a shop and its price.
struct ShopItem {
  /// The game uses 32 bits for the item. The id is in the 16 low bits. The 16
  /// high bits are zero.
  ItemId id;
  u16 _0;
  u32 price;
};

} // namespace pokemon
