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

#include "core/native/data_manager.h"

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
struct Repel;
struct OverworldMenu;
struct EventTable;
struct DayCare;
struct Fusion;
struct HallOfFame;

class SaveData {
  SINGLETON(SaveData)

public:
  STATIC_INLINE SaveData& GetInstance() {
    return core::DataManager::GetInstance().GetSavedata();
  }

  INLINE Footer& GetFooter() { return *footer_; }
  INLINE Misc& GetMisc() { return *(Misc*)segments_[11]; }
  INLINE BoxManager& GetBoxManager() { return *(BoxManager*)segments_[12]; }
  INLINE Minigame& GetMinigame() { return *(Minigame*)segments_[8]; }
  INLINE BattleBox& GetBattleBox() { return *(BattleBox*)segments_[13]; }
  INLINE PssGroup& GetPssFavouriteGroup() { return *(PssGroup*)segments_[14]; }
  INLINE PssGroup& GetPssFriendGroup() { return *(PssGroup*)segments_[15]; }
  INLINE PssGroup& GetPssAcquaintanceGroup() { return *(PssGroup*)segments_[16]; }
  INLINE BagManager& GetBagManager() { return *(BagManager*)segments_[2]; }
  INLINE PokemonAmie& GetPokemonAmie() { return *(PokemonAmie*)segments_[0]; }
  INLINE ItemManager& GetItemManager() { return *(ItemManager*)segments_[1]; }
  INLINE TrainerStatus& GetTrainerStatus() { return *(TrainerStatus*)segments_[17]; }
  INLINE OPowerManager& GetOPowerManager() { return *(OPowerManager*)segments_[25]; }
  INLINE Repel& GetRepel() { return *(Repel*)segments_[31];}
  INLINE HallOfFame& GetHallOfFame() { return *(HallOfFame*)segments_[36]; }
  INLINE OverworldMenu& GetOverworldMenu() { return *(OverworldMenu*)segments_[29]; }
  INLINE RecordManager& GetRecordManager() { return *(RecordManager*)segments_[44]; }
  INLINE DayCare& GetDayCare() { return *(DayCare*)segments_[38]; }
  INLINE Fusion& GetFusion() { return *(Fusion*)segments_[22]; }
  INLINE Settings& GetSettings() { return *(Settings*)segments_[23]; }
  INLINE Pokedex& GetPokedex() { return *(Pokedex*)segments_[20]; }
  INLINE EventTable& GetEventTable() { return *(EventTable*)segments_[19]; }
  INLINE PokemonBox& GetPokemonBox() { return *(PokemonBox*)segments_[kPokemonBoxIndex]; }
  INLINE PssPhoto& GetPssPhoto() { return *(PssPhoto*)segments_[50]; }

  static constexpr u32 kPokemonBoxIndex = GAME_CONSTANT(53, 56);
  static constexpr u32 kSegmentCount = GAME_CONSTANT(55, 58);
  static constexpr u32 kSize = GAME_CONSTANT(0x61A68, 0x722C8);

private:
  void* vtable_;
  Footer* footer_;
  u8 data_[kSize];
  void* segments_[kSegmentCount];
};
} // namespace savedata