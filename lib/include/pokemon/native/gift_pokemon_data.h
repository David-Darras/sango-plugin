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
 * @file gift_pokemon_data.h
 * @brief The table of the gift Pokémon.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/gender_roll.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/shiny_roll.h"
#include "pokemon/constant/species.h"

namespace pokemon {
/// One gift Pokémon of the game: a Pokémon that a character gives to the player.
struct GiftPokemonData {
  /// Returns the gift Pokémon with this index.
  STATIC_INLINE GiftPokemonData& GetInstance(u32 idx) {
    return *(GiftPokemonData*)(
      address::kGiftPokemonTable + sizeof(GiftPokemonData) * idx);
  }

  static constexpr u16 kNotAnEgg = 0xFFFF;
  static constexpr s8 kRandomAbility = -1;
  static constexpr s8 kRandomNature = -1;
  static constexpr s32 kRandomItem = -1;
  static constexpr s8 kRandomIv = -1;

  /// The game stores the species in a 32-bit slot; the id itself fits in
  /// the low half, the high half stays zero.
  SpeciesId species;
  u16 _0;
  FormId form;
  u8 level; ///< The level.
  ShinyRoll shiny; ///< Shiny, not shiny or random.
  s8 ability_slot; ///< The ability slot (0, 1, 2), or kRandomAbility.
  s8 nature; ///< The nature, or kRandomNature.
  s32 item; ///< The held item, or kRandomItem.
  GenderRoll gender;
  u16 egg_place; ///< The place of the Egg, or kNotAnEgg.
  MoveId move;
  s8 iv[6]; ///< The six IVs, or kRandomIv.
  u8 contest[6];
};
} // namespace pokemon
