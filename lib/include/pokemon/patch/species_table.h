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
#include "system/native/string.h"

namespace pokemon {

struct PokeInfo;
struct EvolutionData;

class SpeciesTable {
  MAKE_SINGLETON(SpeciesTable)

public:
  static constexpr u16 kSpeciesCount = 721;
  static constexpr u32 kEntries = 826;
  static constexpr u32 kEntrySize = 0x50;
  static constexpr u32 kAlolanCount = 202;
  static constexpr u32 kGen7Count = 81;
  static constexpr u32 kGen8Count = 216;
  static constexpr u32 kExtraCount = kGen7Count + kGen8Count;
  static constexpr u32 kTotalEntries = kEntries + kAlolanCount + kExtraCount;

  static void Initialize();
  static void Update();
  static void PatchModelRequest(PokeInfo* info);
  static bool IsGen7(u16 species);
  static bool PatchEvolutionTable(u16 species, EvolutionData* table);
  static u16 kFirstGen7Species;
  static constexpr u16 kFirstGen8Species = 805;
  static constexpr u32 kGen8Skipped = 0;

private:
  static void BuildTable(u8* game_table);
  static void GetSpeciesNameHook(String* output, u16 species);
  static void LoadMovepoolHook(u16 species, u8 form);

  bool is_built_ = false;
  alignas(4) u8 table_[kTotalEntries * kEntrySize];
};

} // namespace pokemon
