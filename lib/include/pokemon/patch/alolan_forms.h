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
 * @file alolan_forms.h
 * @brief Adds extra forms to species: Alolan, Galarian, new Mega Evolutions...
 *
 * A Pokémon changes into its extra form in battle, like a Mega Evolution,
 * when it meets the condition of the form. Sango Studio gives the condition
 * of each form in its file (see pokemon/patch/species_packs.h).
 */

#pragma once

#include "common.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/species.h"

namespace pokemon {

struct MegaEvolutionData;

/// Adds extra forms to species and changes them in battle like Mega Evolutions.
class AlolanForms {
  MAKE_SINGLETON(AlolanForms)

public:
  /// Adds the extra forms of a species to its Mega Evolution table.
  static bool PatchMegaTable(SpeciesId species, MegaEvolutionData* table);
  static void Initialize();
  /// Returns true when the species has an extra form.
  static bool HasForm(u16 species);
  /// Returns the number of extra forms of the species.
  static u32 FormCount(u16 species);
  /// Returns the form number of an extra form.
  static u32 GetForm(u16 species, u32 rank = 0);
  /// Returns the form of the model of an extra form.
  static u32 GetModelForm(u16 species, u32 form);
  /// Returns the extra form that the Pokémon can change into now, or 0. A
  /// form has one condition: no condition, a held item, a known move or an
  /// ability. `pokemon` is the data of the Pokémon.
  static u32 FindMegaForm(u16 species, void* pokemon);
  /// Returns true when the Life Orb changes the species.
  static bool IsLifeOrbSpecies(u16 species);

private:
  static u32 GetMegaEvolvedFormNoHook(void* manager, void* poke);
};

} // namespace pokemon
