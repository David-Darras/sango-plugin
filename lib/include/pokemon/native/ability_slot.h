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
 * @file ability_slot.h
 * @brief The slot of the ability of a Pokémon: first, second or hidden ability.
 *
 * A Pokémon has an ability and three flags in its data: one for each slot of its
 * species (first ability, second ability, hidden ability). The ability and the
 * flags must agree. When a product changes the species, the form or the
 * ability of a Pokémon, it calls one of these functions to keep them in agreement.
 */

#pragma once

#include "common.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/species_data.h"

namespace pokemon {

/// The bits of the three flags in `CoreData::ability_flags`.
constexpr u8 kAbilityFlagsMask = 7;

/// Returns the slot that the flags give: 0 (first ability), 1 (second ability) or
/// 2 (hidden ability). Without a flag, the slot is 0.
inline u32 GetAbilitySlot(const CoreData& core) {
  for (u32 slot = 0; slot < 3; slot++) {
    if (core.ability_flags & (1u << slot)) return slot;
  }
  return 0;
}

/// Gives the Pokémon the ability of a slot of its species, and sets the flags.
/// An empty slot gives the first ability.
inline void SetAbilitySlot(CoreData& core, u32 slot) {
  const SpeciesData& data = SpeciesData::GetInstance(core.species, core.form);
  if (slot > 2 || data.ability[slot] == AbilityId::kNone) slot = 0;
  core.ability = data.ability[slot];
  core.ability_flags = static_cast<u8>(
      (core.ability_flags & ~kAbilityFlagsMask) | (1u << slot));
}

/// Call it after a change of the species or of the form. The Pokémon keeps the
/// slot of its ability and gets the ability of this slot in the new species.
inline void AdjustAbilityToSpecies(CoreData& core) {
  SetAbilitySlot(core, GetAbilitySlot(core));
}

/// Call it after a change of the ability. The flags get the slot of the species
/// that has this ability. The current slot stays when it already has the ability.
inline void SyncAbilityFlags(CoreData& core) {
  const SpeciesData& data = SpeciesData::GetInstance(core.species, core.form);
  const u32 current = GetAbilitySlot(core);
  if (data.ability[current] == core.ability) return;
  for (u32 slot = 0; slot < 3; slot++) {
    if (data.ability[slot] == core.ability) {
      core.ability_flags = static_cast<u8>(
          (core.ability_flags & ~kAbilityFlagsMask) | (1u << slot));
      return;
    }
  }
}

} // namespace pokemon
