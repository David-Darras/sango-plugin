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
#include "game/constant/species.h"

namespace feature {
class StarterChoice {
  MAKE_SINGLETON(StarterChoice)

public:
  static constexpr u32 kCount = 3;
  Species candidates[kCount] = {Species::kNone, Species::kNone,
                                Species::kNone};

  STATIC_INLINE void PatchLoad() {
    auto& feat = GetInstance();
    if (feat.candidates[0] == Species::kNone) return;
    for (u32 i = 0; i < kCount; i++) {
      Apply((PokeInfo*)(ADDRESS_STARTER + kViewStride * i), feat.candidates[i]);
      Apply((PokeInfo*)(ADDRESS_STARTER_MODEL + kModelStride * i),
            feat.candidates[i]);
    }
  }

private:
  static constexpr u32 kViewStride = 0x54;
  static constexpr u32 kModelStride = 0x170;

  static void Apply(PokeInfo* info, Species species) {
    info->species = species;
    info->form = Form::kNormal;
    info->gender = Gender::kMale;
    info->is_shiny = false;
    info->is_egg = false;
  }
};
}
