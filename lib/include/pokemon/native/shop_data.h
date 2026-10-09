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
 * @file shop_data.h
 * @brief The items of the shop that is open.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/shop_type.h"
#include "pokemon/native/shop_item.h"

namespace pokemon {

/// The items of the shop that is open. A shop has 60 items at most.
struct ShopData {
  static constexpr u32 kMaxItems = 60;

  void* heap;
  ShopItem items[kMaxItems];
  ShopType type;
  u32 count; ///< The number of entries in `items`.
  void* item_manager;
  String* name_buffer; ///< The text that the name function returns.
  String* info_buffer; ///< The text that the description function returns.
  Message* move_name_message;
  Message* move_info_message;
  Message* goods_name_message;
  Message* goods_info_message;
};

} // namespace pokemon
