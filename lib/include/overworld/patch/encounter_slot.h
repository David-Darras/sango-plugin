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
 * @file encounter_slot.h
 * @brief One wild Pokémon of an encounter table.
 *
 * The file has no game structure: the tests of tests/ compile it on a
 * computer (see core/maker_reader.h).
 */

#pragma once

#include "core/types.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/species.h"

namespace overworld {

/// One wild Pokémon of an encounter table: its species and its levels.
struct EncounterSlot {
  SpeciesId species;
  u8 min_level;
  u8 max_level;
  FormId form;

  constexpr EncounterSlot()
    : species(SpeciesId::kNone), min_level(1), max_level(1),
      form(FormId::kNormal) {}
  constexpr EncounterSlot(SpeciesId species, u8 min_level, u8 max_level,
                FormId form = FormId::kNormal)
    : species(species), min_level(min_level), max_level(max_level),
      form(form) {}
};

} // namespace overworld
