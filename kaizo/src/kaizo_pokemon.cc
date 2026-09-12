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
        Species::kGengar,
        Form::kGengarMega);
    pkm.ability[0] = pkm.ability[1] = pkm.ability[2] =
        Ability::kInfiltrator;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        Species::kBlaziken,
        Form::kBlazikenNormal);
    pkm.ability[0] = pkm.ability[1] = pkm.ability[2] =
        Ability::kBlaze;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        Species::kBlaziken,
        Form::kBlazikenMega);
    pkm.ability[0] = pkm.ability[1] = pkm.ability[2] =
        Ability::kQuickFeet;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        Species::kKangaskhan,
        Form::kKangaskhanNormal);
    pkm.base_attack = 100;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        Species::kLucario,
        Form::kLucarioMega);
    pkm.base_attack = 145;
    pkm.base_special_attack = 140;
  }
  {
    auto& pkm = global_data::Pokemon::GetInstance(
        Species::kMawile,
        Form::kMawileMega);
    pkm.base_attack = 85;
  }
}
} // namespace kaizo