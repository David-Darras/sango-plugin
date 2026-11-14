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
 * @file species_packs.h
 * @brief The species and the forms that Sango Studio installs from packs.
 *
 * Sango Studio owns the data of the species that the game does not have. It
 * writes the file `sdmc:/sango-studio/species/table.bin`, and this class reads
 * it one time. The plugin has no data about these species.
 *
 * The file has a header of 16 bytes (the signature "SSPT", the version 1, the
 * number of species, the number of forms, 4 bytes of zero), then the species
 * (256 bytes each, with the numbers 724, 725...) and then the forms (88 bytes
 * each). The layout is in the structures PackSpecies and PackForm. The
 * writer is `module/pack/src/table.rs` in Sango Studio.
 *
 * A species or a form that was removed in Studio has an empty name and zero
 * data: it keeps its place, so the numbers of the other species stay the same.
 */

#pragma once

#include "common.h"

namespace pokemon {

/// A move that a species of a pack learns by level. The list ends with the
/// move 0.
struct PackMove {
  u16 move;
  u8 level;
  u8 padding;
};

/// An evolution of a species of a pack. An unused place has the species 0.
struct PackEvolution {
  u8 method;
  u8 padding;
  u16 arg;
  u16 species;
};

/// A species of a pack, as the file has it (256 bytes).
struct PackSpecies {
  static constexpr u32 kNameSize = 24;
  static constexpr u32 kMoveMax = 24;
  static constexpr u32 kEvolutionMax = 4;

  u16 species;
  /// Where the model is: 0 to 255 a form of the first carrier species, 256
  /// and more a form of the second carrier, 0x8000 | n the model of the
  /// species n of the game.
  u16 model_form;
  c16 name[kNameSize];
  u8 data[0x50];
  PackMove moves[kMoveMax];
  PackEvolution evolutions[kEvolutionMax];
  u8 padding[4];
};
static_assert(sizeof(PackSpecies) == 256,
              "PackSpecies must have the layout of the file");

/// A form that a pack adds to a species (88 bytes).
struct PackForm {
  u16 species;
  u16 form;
  u16 model_form;
  u8 data[0x50];
  u8 padding[2];
};
static_assert(sizeof(PackForm) == 88,
              "PackForm must have the layout of the file");

/// Reads the table of the packs and gives its species and forms.
class SpeciesPacks {
public:
  /// The number of the first species of a pack.
  static constexpr u16 kFirstSpecies = 724;
  /// The most species and forms that the plugin keeps. Sango Studio refuses a
  /// pack that does not fit (the same numbers are in module/pack/src/library.rs).
  static constexpr u32 kMaxSpecies = 384;
  static constexpr u32 kMaxForms = 256;

  /// Reads `species/table.bin` from the SD card. It tries one time: the next
  /// calls do nothing. Without the file, there is no species and no form.
  static void Load();

  /// Returns the number of species (the places, with the empty ones).
  static u32 SpeciesCount();
  /// Returns the species with this number, or null.
  static const PackSpecies* FindSpecies(u16 species);
  /// Returns the species at this place (0 to SpeciesCount() - 1).
  static const PackSpecies& GetSpecies(u32 index);
  /// Returns true for an empty place: a species that was removed.
  static bool IsEmpty(const PackSpecies& species);

  /// Returns the number of forms.
  static u32 FormCount();
  /// Returns the form at this place (0 to FormCount() - 1).
  static const PackForm& GetForm(u32 index);
};

} // namespace pokemon
