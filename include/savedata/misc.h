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

#ifndef SANGO_PLUGIN_SAVEDATA_MISC_H
#define SANGO_PLUGIN_SAVEDATA_MISC_H
#include "common.h"
#include "savedata.h"

namespace savedata {

/**
 * @struct Misc
 * @brief Container for general player progress and metadata.
 * * This structure holds various information such as currency, badges,
 * the rival's name, and unlock flags for game mechanics.
 */
struct Misc {
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  static Misc& GetInstance() { return SaveData::GetInstance().GetMisc(); }

  /// @brief Maximum character length for nicknames.
  static constexpr u32 kNicknameLength = 13;

  void* vtable;      ///< Pointer to the virtual method table.
  u32 padding0;      ///< Alignment padding.
  u64 random_value;  ///< Seed or value used for RNG-related save state.

  u32 money;   ///< Current amount of player currency.
  u32 badges;  ///< Bitmask or count of obtained gym badges.

  /**
   * @brief The name assigned to the rival character.
   * Stored as a 16-bit character array (UTF-16/UCS-2).
   */
  c16 rival_nickname[kNicknameLength];
  c16 padding1;  ///< Alignment padding after nickname.

  u32 flags;          ///< General purpose game state flags.
  u32 battle_points;  ///< Currency earned from battle facilities.

  /**
   * @brief Bitmask tracking encountered Mega Evolutions.
   * Each bit represents a specific Pokémon species from the Mega Table.
   * - Bit 0: Gengar
   * - Bit 1: Gardevoir
   * - Bit 2: Ampharos
   * - Bit 3: Venusaur
   * - etc.
   */
  u8 has_encountered_mega_evolution[8];
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_MISC_H
