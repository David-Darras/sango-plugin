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

#include "core/types.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender.h"
#include "pokemon/constant/species.h"
#include "renderer/native/species_flag_metadata.h"
#include "renderer/native/species_metadata.h"

namespace renderer {

/// The table mapping a species / form / gender to its files in the game's
/// Pokémon model archive.
struct PokemonModelTable {
  static constexpr u16 kSpeciesCount = 721;
  /// Files per Pokémon in the model archive, and index of the first one.
  static constexpr u32 kFileSectionCount = 8;
  static constexpr u32 kPackFirstFile = 3;
  static constexpr u32 kInvalidPack = 0xFFFFFFFF;

  explicit PokemonModelTable(void* buffer)
    : species_metadata_((SpeciesMetadata*)buffer),
      flags_((SpeciesFlagMetadata*)((uptr)buffer + (kSpeciesCount + 1) *
                                    sizeof(SpeciesMetadata))) {
  }

  INLINE bool IsValid() const { return species_metadata_ != nullptr; }

  INLINE const SpeciesMetadata* GetSpeciesMetadata(SpeciesId species) const {
    const u16 index = static_cast<u16>(species);
    if (index == 0 || index > kSpeciesCount) return nullptr;
    return &species_metadata_[index - 1];
  }

  INLINE const SpeciesFlagMetadata* GetSpeciesFlagMetadata(
      SpeciesId species, Form form, Gender gender) const {
    const s32 index = GetDataIndex(species, form, gender);
    return index < 0 ? nullptr : &flags_[index];
  }

  INLINE s32 GetDataIndex(SpeciesId species, Form form, Gender gender) const {
    const SpeciesMetadata* species_metadata = GetSpeciesMetadata(species);
    if (species_metadata == nullptr) return -1;

    const u32 form_index = static_cast<u8>(form);
    u32 index = 0;
    if ((species_metadata->flags & SpeciesMetadata::kHasFormChange) != 0 &&
        form_index > 0) {
      index = (species_metadata->flags & SpeciesMetadata::kHasFemale) != 0
                ? form_index + 1u
                : form_index;
    } else if (gender == Gender::kFemale &&
               (species_metadata->flags & SpeciesMetadata::kHasFemale) != 0) {
      index = 1;
    }
    if (index >= species_metadata->data_count) index = 0;
    return (s32)(species_metadata->data_offset + index);
  }

  INLINE u32 GetPackTop(SpeciesId species, Form form, Gender gender) const {
    const s32 index = GetDataIndex(species, form, gender);
    if (index < 0) return kInvalidPack;
    return (u32)index * kFileSectionCount + kPackFirstFile;
  }

private:
  SpeciesMetadata* species_metadata_;
  SpeciesFlagMetadata* flags_;
};

} // namespace renderer
