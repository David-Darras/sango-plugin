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
 * along with this program.  If not, see <https:
 */

#include "game/constant/form.h"
#include "game/constant/species.h"
#include "game/constant/ability.h"
#include "game/global_data/pokemon.h"

namespace kaizo {
void PatchPokemonData() {
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        SPECIES_GENGAR, FORM_GENGAR_MEGA);
    pkm.ability[0] = pkm.ability[1] = pkm.ability[2] = ABILITY_INFILTRATOR;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        SPECIES_BLAZIKEN, FORM_BLAZIKEN_NORMAL);
    pkm.ability[0] = pkm.ability[1] = pkm.ability[2] = ABILITY_BLAZE;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        SPECIES_BLAZIKEN, FORM_BLAZIKEN_MEGA);
    pkm.ability[0] = pkm.ability[1] = pkm.ability[2] = ABILITY_QUICK_FEET;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        SPECIES_KANGASKHAN, FORM_KANGASKHAN_NORMAL);
    pkm.base_attack = 100;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        SPECIES_LUCARIO, FORM_LUCARIO_MEGA);
    pkm.base_attack = 145;
    pkm.base_special_attack = 140;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        SPECIES_MAWILE, FORM_MAWILE_MEGA);
    pkm.base_attack = 85;
  }
}
} // namespace kaizo