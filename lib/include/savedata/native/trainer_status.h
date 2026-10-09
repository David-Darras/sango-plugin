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
 * @file trainer_status.h
 * @brief The trainer data of the player: name, id, gender, outfit.
 */

#pragma once

#include "common.h"
#include "savedata/native/savedata.h"

namespace savedata {
/// The trainer data of the player.
struct TrainerStatus {
  SINGLETON(TrainerStatus)

  STATIC_INLINE TrainerStatus& GetInstance() {
    return SaveData::GetInstance().GetTrainerStatus();
  }

  /// The size of a name, in characters (with the end character).
  static constexpr u32 kPlayerNameLen = 13;
  /// The size of a PSS message, in characters (with the end character).
  static constexpr u32 kPssMessageLen = 17;

  void* vtable;
  u32 padding;

  /// @name The identity of the trainer
  /// @{
  u16 trainer_id; ///< The Trainer ID (TID) of the Trainer Card.
  u16 secret_id; ///< The secret ID (SID). The shiny calculation uses it.
  /// The game version (24: X, 25: Y, 26: Alpha Sapphire, 27: Omega Ruby).
  u8 game_version;
  Gender gender;
  u8 unknow0;
  u8 pss_icon;
  /// @}

  /// @name The ids of the network and of the console
  /// @{
  u64 nex_id; ///< The Nintendo Network id (NEX).
  /// The id of the current console (the local friend code seed).
  u64 current_console_id;
  u64 original_console_id; ///< The id of the console that made the save.
  u32 principal_id; ///< The principal id of the friend code.
  u32 unknow3; ///< Unknown network data.
  /// @}

  /// @name The place and the region
  /// @{
  u16 latitude; ///< The latitude for the Poké Miles.
  u16 longitude; ///< The longitude for the Poké Miles.
  u8 region; ///< The region of the console (0: Japan, 1: America, 2: Europe...).
  Language language; ///< The language of the game.
  /// @}

  /// @name The parental controls
  /// @{
  /// The COPPA restriction (Children's Online Privacy Protection Act). When
  /// it is true, the PSS has no voice chat and no picture exchange.
  bool coppa_restriction;
  u8 coppa_value; ///< The level of the restriction.
  /// @}

  /// @name The look and the texts
  /// @{
  u8 style[16]; ///< The outfit: clothes, hair, accessories.
  u32 pss_flags; ///< The PSS options (refusals, visibility).
  u32 reserved;
  c16 name[kPlayerNameLen];
  c16 nickname[kPlayerNameLen];
  c16 pss_messages[6][kPssMessageLen]; ///< The six PSS messages.
  /// @}

  /// @name The progress
  /// @{
  u16 unknow1;
  /// The progress flags (bit 0: Mega Ring, bit 1: Mega Rayquaza).
  u16 mega_flags;
  u8 unknow2[28];
  u64 pss_id;
  /// @}
};
} // namespace savedata
