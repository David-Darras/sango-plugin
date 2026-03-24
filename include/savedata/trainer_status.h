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

#ifndef SANGO_PLUGIN_SAVEDATA_TRAINER_STATUS_H
#define SANGO_PLUGIN_SAVEDATA_TRAINER_STATUS_H

#include "common.h"
#include "savedata.h"

namespace savedata {

/**
 * @class TrainerStatus
 * @brief Manages player identity, network synchronization, and PSS
 * configurations.
 */
struct TrainerStatus {
  /** @brief Accessor for the singleton instance. */
  static TrainerStatus& GetInstance() {
    return SaveData::GetInstance().GetTrainerStatus();
  }

  /** @brief Initializes the plugin menu entries for real-time editing. */
  static void LoadMenu(menu::PluginMenu& menu, void* args);

  static constexpr u32 kPlayerNameLen = 13;  ///< Max name length
  static constexpr u32 kPssMessageLen = 17;  ///< Max PSS message length

  void* vtable;
  u32 padding;

  /** @name Core Identity */
  ///@{
  u16 trainer_id;   ///< Visible Trainer ID (TID) shown on the Trainer Card
  u16 secret_id;    ///< Secret Trainer ID (SID) used for Shiny calculations
  u8 game_version;  ///< Game version ID (e.g., 24: X, 25: Y, 26: AS, 27: OR)
  u8 gender;        ///< Player gender (0: Male, 1: Female)
  u8 unknow0;
  u8 pss_icon;  ///< Active PSS avatar icon index
  ///@}

  /** @name Network & Console Identifiers */
  ///@{
  u64 nex_id;               ///< Unique Nintendo Network identifier (NEX)
  u64 current_console_id;   ///< Current hardware ID (AccountID /
                            ///< LocalFriendCodeSeed)
  u64 original_console_id;  ///< ID of the console where the save was created
  u32 principal_id;         ///< Principal ID linked to Friend Code
  u32 unknow3;              ///< Undocumented network-related data
  ///@}

  /** @name Geolocation & Regional Settings */
  ///@{
  u16 latitude;   ///< Latitude used for Poke Miles distance calculation
  u16 longitude;  ///< Longitude used for Poke Miles distance calculation
  u8 region;      ///< Console region code (0: JPN, 1: USA, 2: EUR, etc.)
  u8 language;    ///< Game language (1: JPN, 2: ENG, 3: FRE, etc.)
  ///@}

  /** @name Parental Controls & Privacy */
  ///@{
  /** @brief Children's Online Privacy Protection Act (COPPA) flag.
   * If true, restricts Voice Chat and Picture Exchange on the PSS. */
  bool coppa_restriction;
  u8 coppa_value;  ///< Detailed restriction levels/values
  ///@}

  /** @name Appearance & Strings */
  ///@{
  u8 style[16];   ///< Customization data (Clothing/Hair/Accessories)
  u32 pss_flags;  ///< Bitfield for PSS settings (Rejections, visibility)
  u32 reserved;
  c16 name[kPlayerNameLen];             ///< Player name
  c16 nickname[kPlayerNameLen];         ///< Player nickname
  c16 pss_messages[6][kPssMessageLen];  ///< Set of 6 customizable PSS shout-out
                                        ///< messages
  ///@}

  /** @name Progression & Attributes */
  ///@{
  u16 unknow1;
  /** @brief Special progression flags (e.g., Bit 0: Mega Ring, Bit 1: Mega
   * Rayquaza). */
  u16 mega_flags;
  u8 unknow2[28];
  u64 pss_id;
  ///@}
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_SAVEDATA_TRAINER_STATUS_H