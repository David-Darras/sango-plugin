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
#include "pokemon/constant/type.h"

namespace battle {

class TypeChart {
  MAKE_SINGLETON(TypeChart)

public:
  enum class Multiplier : u8 {
    k0 = 0,
    k05 = 1,
    k1 = 2,
    k2 = 4
  };

  static constexpr u32 kTypeCount = static_cast<u32>(TypeId::kCount);
  typedef Multiplier Row[kTypeCount];

  static void PatchLoad();
  static Row* GetTable();
  static void Set(TypeId attacking_type, TypeId defending_type, Multiplier value);
  static Multiplier Get(TypeId attacking_type, TypeId defending_type);
};
} // namespace battle
