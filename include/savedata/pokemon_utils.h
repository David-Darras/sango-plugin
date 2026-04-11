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

#ifndef SANGO_PLUGIN_POKEMON_UTILS_H
#define SANGO_PLUGIN_POKEMON_UTILS_H

#include "core/core.h"

class PokemonUtils {
 public:
  static u32 GetExperienceFromLevel(u16 species, u8 form, u16 level) {
    ((void (*)(u16, u8))ADDRESS_POKEMON_UTILS_LOAD_POKEMON_EXPERIENCE_TABLE)(
        species, form);
    return ((u32 (*)(u8))ADDRESS_POKEMON_UTILS_GET_POKEMON_MINIMUM_EXPERIENCE)(
        level);
  }

  static u8 GetLevelFromExperience(u16 species, u8 form, u32 experience) {
    return ((u8 (*)(
        u16, u8, u32))ADDRESS_POKEMON_UTILS_GET_POKEMON_LEVEL_FROM_EXPERIENCE)(
        species, form, experience);
  }

  static bool IsShiny(u32 id, u32 shiny_id) {
    return ((bool (*)(u32, u32))ADDRESS_POKEMON_UTILS_IS_SHINY)(id, shiny_id);
  }

  static void ConvertToShiny(u32 id, u32* shiny_id) {
    *shiny_id =
        ((u32 (*)(u32, u32))ADDRESS_POKEMON_UTILS_TO_SHINY)(id, *shiny_id);
  }

  static void ConvertToNormal(u32 id, u32* shiny_id) {
    *shiny_id =
        ((u32 (*)(u32, u32))ADDRESS_POKEMON_UTILS_TO_NORMAL)(id, *shiny_id);
  }
};

#endif  // SANGO_PLUGIN_POKEMON_UTILS_H
