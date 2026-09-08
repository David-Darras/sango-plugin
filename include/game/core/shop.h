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

/** @brief Kind of counter the shop event is running (app::shop::ShopType). */
enum class ShopType : u32 {
  kNormal, ///< Regular mart, pays with money.
  kBattlePoint, ///< Battle Point counter.
  kBattlePointMove, ///< Battle Point move tutor counter.
  kMileagePoint, ///< Mileage Point counter.
  kSecretBaseGoods, ///< Secret base goods counter.
};

struct ShopItem {
  u32 id;
  u32 price;
};

/**
 * @brief The item list a shop counter is currently selling
 * (app::shop::ItemContainer).
 */
struct ShopData {
  static constexpr u32 kMaxItems = 60;

  void* heap;
  ShopItem items[kMaxItems];
  u32 type; ///< ShopType.
  u32 count; ///< Number of entries filled in items.
  void* item_manager;
  String* name_buffer; ///< Buffer returned by the name getter.
  String* info_buffer; ///< Buffer returned by the description getter.
  Message* move_name_message;
  Message* move_info_message;
  Message* goods_name_message;
  Message* goods_info_message;
};
