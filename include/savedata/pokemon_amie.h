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

#ifndef SANGO_PLUGIN_SAVEDATA_POKEMON_AMIE_H
#define SANGO_PLUGIN_SAVEDATA_POKEMON_AMIE_H

#include "common.h"
#include "savedata.h"

namespace savedata {

/**
 * @brief Manages the Pokémon-Amie (Kawaigari) system data.
 * * This structure handles the inventory of Poké Puffs and global
 * interaction timestamps for the Amie system in Pokémon ORAS.
 */
struct PokemonAmie {
  /**
   * @brief Builds the main menu for Pokémon-Amie.
   * @param menu Reference to the PluginMenu instance.
   */
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  /**
   * @brief Returns the singleton instance of PokemonAmie from SaveData.
   * @return Reference to the unique PokemonAmie instance.
   */
  static PokemonAmie& GetInstance() {
    return SaveData::GetInstance().GetPokemonAmie();
  }

  /** @brief Maximum ID value for a Poké Puff item. */
  static constexpr u32 kMaxPokePuffId = 24;

  /** @brief Maximum number of Poké Puffs storage slots. */
  static constexpr u32 kMaxPokePuffs = 100;

  /** @brief Pointer to the virtual method table. */
  void* vtable;

  /** @brief Inventory array containing Poké Puff IDs. */
  u8 poke_puffs[kMaxPokePuffs];

  /** @brief Timestamp (in days) of the last access to the Amie system. */
  s32 last_opened_timestamp;
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_POKEMON_AMIE_H