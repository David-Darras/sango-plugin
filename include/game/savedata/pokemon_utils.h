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
#include "game/constant/ball.h"
#include "game/constant/form.h"
#include "game/constant/item.h"
#include "game/constant/species.h"

class PokemonUtils {
public:
  // The raw function pointers below are real game-ABI entry points. Each enum
  // class has the exact integer width the ABI expects, so the pointer types
  // below simply declare the enum directly instead of casting at every call.
  STATIC_INLINE u32 GetExperienceFromLevel(Species species, Form form,
                                           u16 level) {
    ((void (*)(Species, Form))
        ADDRESS_POKEMON_UTILS_LOAD_POKEMON_EXPERIENCE_TABLE)(species, form);
    return ((u32 (*)(u8))ADDRESS_POKEMON_UTILS_GET_POKEMON_MINIMUM_EXPERIENCE)(
        level);
  }

  STATIC_INLINE u8
  GetLevelFromExperience(Species species, Form form, u32 experience) {
    return ((u8 (*)(Species, Form, u32))
        ADDRESS_POKEMON_UTILS_GET_POKEMON_LEVEL_FROM_EXPERIENCE)(
        species, form, experience);
  }

  STATIC_INLINE bool IsShiny(u32 id, u32 shiny_id) {
    return ((bool (*)(u32, u32))ADDRESS_POKEMON_UTILS_IS_SHINY)(id, shiny_id);
  }

  STATIC_INLINE void ConvertToShiny(u32 id, u32* shiny_id) {
    *shiny_id =
        ((u32 (*)(u32, u32))ADDRESS_POKEMON_UTILS_TO_SHINY)(id, *shiny_id);
  }

  STATIC_INLINE void ConvertToNormal(u32 id, u32* shiny_id) {
    *shiny_id =
        ((u32 (*)(u32, u32))ADDRESS_POKEMON_UTILS_TO_NORMAL)(id, *shiny_id);
  }

  STATIC_INLINE ItemId ConvertBallIdToItemId(Ball ball_id) {
    return ((ItemId (*)(Ball))
      ADDRESS_POKEMON_UTILS_FROM_BALL_ID_TO_ITEM_ID)(ball_id);
  }
};