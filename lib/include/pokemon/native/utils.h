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
#include "pokemon/constant/ball.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/species.h"

namespace pokemon {

class Utils {
public:
  // The raw function pointers below are real game-ABI entry points. Each enum
  // class has the exact integer width the ABI expects, so the pointer types
  // below simply declare the enum directly instead of casting at every call.
  STATIC_INLINE u32 GetExperienceFromLevel(SpeciesId species, FormId form,
                                           u16 level) {
    ((void (*)(SpeciesId, FormId))
        address::kUtilsLoadPokemonExperienceTable)(species, form);
    return ((u32 (*)(u8))address::kUtilsGetPokemonMinimumExperience)(
        level);
  }

  STATIC_INLINE u8
  GetLevelFromExperience(SpeciesId species, FormId form, u32 experience) {
    return ((u8 (*)(SpeciesId, FormId, u32))
        address::kUtilsGetPokemonLevelFromExperience)(
        species, form, experience);
  }

  STATIC_INLINE bool IsShiny(u32 id, u32 shiny_id) {
    return ((bool (*)(u32, u32))address::kUtilsIsShiny)(id, shiny_id);
  }

  STATIC_INLINE void ConvertToShiny(u32 id, u32* shiny_id) {
    *shiny_id =
        ((u32 (*)(u32, u32))address::kUtilsToShiny)(id, *shiny_id);
  }

  STATIC_INLINE void ConvertToNormal(u32 id, u32* shiny_id) {
    *shiny_id =
        ((u32 (*)(u32, u32))address::kUtilsToNormal)(id, *shiny_id);
  }

  STATIC_INLINE ItemId ConvertBallIdToItemId(Ball ball_id) {
    return ((ItemId (*)(Ball))
      address::kUtilsFromBallIdToItemId)(ball_id);
  }
};

} // namespace pokemon
