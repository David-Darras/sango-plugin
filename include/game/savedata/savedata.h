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

#include "pss.h"
#include "game/data_manager.h"

namespace savedata {
class Footer;
struct Misc;
struct BoxManager;
struct BagManager;
struct PokemonAmie;
struct ItemManager;
struct TrainerStatus;
struct PokemonBox;
struct BattleBox;
struct PokemonTeam;
struct Settings;
struct Pokedex;
struct RecordManager;
struct PssPhoto;
struct Minigame;
struct OPowerManager;
struct PssGroup;
struct Encounter;
struct OverworldMenu;
struct EventTable;
struct DayCare;
struct Fusion;
struct HallOfFame;

/**
* @class SaveData
* @brief Represents the structure and management of the game's save data.
* * This class provides access to the raw save buffer, the segment pointers,
* and the integrity-checking Footer.
*/
class SaveData {
  SINGLETON(SaveData)

public:
  /**
* @brief Accesses the global SaveData instance via the GameDataManager.
* @return A reference to the active SaveData instance.
*/
  STATIC_INLINE SaveData& GetInstance() {
    return game::DataManager::GetInstance().GetSavedata();
  }

  /**
* @brief Retrieves the footer associated with this save data.
* @return A reference to the Footer object used for integrity validation.
*/
  INLINE Footer& GetFooter() { return *footer_; }

  INLINE Misc& GetMisc() { return *(Misc*)segments_[11]; }

  INLINE BoxManager& GetBoxManager() {
    return *(BoxManager*)segments_[12];
  }

  INLINE Minigame& GetMinigame() { return *(Minigame*)segments_[8]; }

  INLINE BattleBox& GetBattleBox() { return *(BattleBox*)segments_[13]; }

  INLINE PssGroup& GetPssFavouriteGroup() {
    return *(PssGroup*)segments_[14];
  }

  INLINE PssGroup& GetPssFriendGroup() {
    return *(PssGroup*)segments_[15];
  }

  INLINE PssGroup& GetPssAcquaintanceGroup() {
    return *(PssGroup*)segments_[16];
  }

  INLINE BagManager& GetBagManager() {
    return *(BagManager*)segments_[2];
  }

  INLINE PokemonAmie& GetPokemonAmie() {
    return *(PokemonAmie*)segments_[0];
  }

  INLINE ItemManager& GetItemManager() {
    return *(ItemManager*)segments_[1];
  }

  INLINE TrainerStatus& GetTrainerStatus() {
    return *(TrainerStatus*)segments_[17];
  }

  // INLINE PokemonTeam& GetPokemonTeam() { return *(PokemonTeam*)segments_[18]; }

  INLINE PokemonBox& GetPokemonBox() {
    return *(PokemonBox*)segments_[56];
  }

  INLINE OPowerManager& GetOPowerManager() {
    return *(OPowerManager*)segments_[25];
  }

  INLINE Encounter& GetEncounter() {
    return *(Encounter*)segments_[31];
  }

  INLINE HallOfFame& GetHallOfFame() {
    return *(HallOfFame*)segments_[36];
  }

  INLINE OverworldMenu& GetOverworldMenu() {
    return *(OverworldMenu*)segments_[29];
  }

  INLINE RecordManager& GetRecordManager() {
    return *(RecordManager*)segments_[44];
  }

  INLINE DayCare& GetDayCare() {
    return *(DayCare*)segments_[38];
  }

  INLINE PssPhoto& GetPssPhoto() {
    return *(PssPhoto*)segments_[50];
  }

  INLINE Fusion& GetFusion() { return *(Fusion*)segments_[22]; }

  INLINE Settings& GetSettings() { return *(Settings*)segments_[23]; }

  INLINE Pokedex& GetPokedex() { return *(Pokedex*)segments_[20]; }

  INLINE EventTable& GetEventTable() { return *(EventTable*)segments_[19]; }


  /// @brief Total number of data segments monitored for integrity.
  static constexpr u32 kSegmentCount = 58;

private:
  void* vtable_; ///< Pointer to the virtual method table.
  Footer* footer_; ///< Pointer to the integrity and checksum footer.

  /**
* @brief Raw save data buffer.
* Total size: 0x722C8 bytes.
*/
  u8 data_[0x722C8];

  /**
* @brief Array of pointers to specific data segments within the save.
*/
  void* segments_[kSegmentCount]; ///< Address : 0x08CE0C68
};
} // namespace savedata