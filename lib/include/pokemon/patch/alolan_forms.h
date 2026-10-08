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
 * when it holds the item of the form (Life Orb, Choice Band, Choice Specs or
 * Choice Scarf). The data is in lib/src/pokemon/data/alolan_form.inc.
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
  static constexpr u32 kSpeciesEntries = 826;
  static constexpr u32 kEntrySize = 0x50;
  static constexpr u32 kFormCount = 138;

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
  /// Returns the item that changes the Pokémon into its extra form number `rank`.
  static ItemId GetItem(u32 rank);
  /// Returns the extra form of an item, or 0.
  static u32 GetFormByItem(u16 species, ItemId item);
  /// Returns true when the Life Orb changes the species.
  static bool IsLifeOrbSpecies(u16 species);

private:
  static u32 GetMegaEvolvedFormNoHook(void* manager, void* poke);
};

} // namespace pokemon
