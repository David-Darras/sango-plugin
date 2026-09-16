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

#include "battle/patch/type_chart.h"

namespace battle {

void TypeChart::PatchLoad() {
  // Reserved for custom type chart patches
}

TypeChart::Row* TypeChart::GetTable() {
  return reinterpret_cast<Row*>(address::kTypeChart);
}

void TypeChart::Set(TypeId attacking_type, TypeId defending_type,
                    TypeMultiplier value) {
  const u8 row = static_cast<u8>(attacking_type);
  const u8 col = static_cast<u8>(defending_type);
  GetTable()[row][col] = value;
}

TypeMultiplier TypeChart::Get(TypeId attacking_type,
                                     TypeId defending_type) {
  const u8 row = static_cast<u8>(attacking_type);
  const u8 col = static_cast<u8>(defending_type);
  return GetTable()[row][col];
}

} // namespace battle
