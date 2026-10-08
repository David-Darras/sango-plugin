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
 * @file field_position.h
 * @brief The positions on the battlefield.
 */

#pragma once
#include <types.h>

namespace battle {
/// A position on the battlefield: a side and a slot.
enum class FieldPosition : u8 {
  /// The side of the player in a local battle, the side of the host online.
  kFirstSideSlot0,
  kSecondSideSlot0,
  kFirstSideSlot1,
  kSecondSideSlot1,
  kFirstSideSlot2,
  kSecondSideSlot2,
  kFirstSideSlot3,
  kSecondSideSlot3,
  kFirstSideSlot4,
  kSecondSideSlot4,
  kCount,
  kNone = kCount,
};
}
