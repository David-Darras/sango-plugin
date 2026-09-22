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
#include "pokemon/constant/species.h"

namespace pokemon {

struct MegaEvolutionData;

class AlolanForms {
  MAKE_SINGLETON(AlolanForms)

public:
  static constexpr u32 kSpeciesEntries = 826;
  static constexpr u32 kEntrySize = 0x50;
  static constexpr u32 kFormCount = 18;

  static bool PatchMegaTable(SpeciesId species, MegaEvolutionData* table);
  static void Initialize();
  static bool HasForm(u16 species);

private:
  static u32 GetMegaEvolvedFormNoHook(void* manager, void* poke);
};

} // namespace pokemon
