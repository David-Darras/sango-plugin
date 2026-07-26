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

namespace feature {
class Starter {
  struct Data {
    PokeInfo info;
    u8 padding[0x54 - sizeof(PokeInfo)];
  };
public:
  STATIC_INLINE void Patch() {
    Data* data = (Data*)(ADDRESS_STARTER);
    data[0].info.is_egg = true;
    data[0].info.species = 0;
    data[1].info.is_egg = true;
    data[1].info.species = 0;
    data[2].info.is_egg = true;
    data[2].info.species = 0;
  }
};
} // namespace feature