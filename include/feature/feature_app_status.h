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
#include "common.h"
#include "feature_device.h"
#include "hook_manager.h"
#include "game/process_manager.h"
#include "game/renderer/app_layout_manager.h"
#include "game/savedata/pokemon_team.h"

namespace feature {
class Pane;

class AppStatus {
  MAKE_SINGLETON(AppStatus)
public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kAppStatusSetupGraphicsParams,
                            ADDRESS_APP_STATUS_SETUP_GRAPHICS_PARAMS,
                            (uptr)SetupGraphicsParamsHook, false);
    HookManager::Initialize(HookID::kAppStatusSetupGraphicsMoves,
                            ADDRESS_APP_STATUS_SETUP_GRAPHICS_MOVES,
                            (uptr)SetupGraphicsMovesHook, false);
    HookManager::Initialize(HookID::kAppStatusSetupGraphicsContest,
                            ADDRESS_APP_STATUS_SETUP_GRAPHICS_CONTEST,
                            (uptr)SetupGraphicsContestHook, false);
    HookManager::Initialize(HookID::kAppStatusSetupGraphicsInfos,
                            ADDRESS_APP_STATUS_SETUP_GRAPHICS_INFOS,
                            (uptr)SetupGraphicsInfosHook, false);
  }

  static void PatchLoad();
  static void PatchUpdate();
  static void SetupGraphicsParamsHook(uptr self,
                                      savedata::PokemonParam* pokemon);
  static void SetupGraphicsMovesHook(uptr self,
                                     savedata::PokemonParam* pokemon,
                                     u8 move_index);
  static void SetupGraphicsContestHook(uptr self,
                                       savedata::PokemonParam* pokemon);
  static void SetupGraphicsInfosHook(uptr self,
                                     savedata::PokemonParam* pokemon);

  enum class Mode : u8 {
    kIdle,
    kNavigation,
    kEditing,
  };

  enum class PowerPage : u8 {
    kStat,
    kIv,
    kEv,
    kMax
  };

  enum class ItemPage : u8 {
    kHeldItem = 0,
    kBall = 1,
    kMax = 2,

    kNature = 0,
    kForm = 1,
  };

  enum class SubMenu : u8 {
    kParamsAndMoves,
    kContest,
    kOther,
    kMax
  };

  bool& is_game_input_disabled_ = DeviceState::GetInstance().
      use_redirection;

  static uptr GetThis() {
    return (uptr)game::ProcessManager::GetInstance().
        GetCurrentProcess();
  }

  static u8 GetSlot() {
    return READ8(GetThis() + 124);
  }

  INLINE void Reset() {
    power_page_ = PowerPage::kStat;
    item_page_ = ItemPage::kHeldItem;
    sub_menu_ = SubMenu::kParamsAndMoves;
    ChangeMode(Mode::kIdle);
  }

  INLINE bool IsOn(const Pane* pane) const {
    return current_pane_ == pane;
  }

  Mode mode_;
  PowerPage power_page_;
  ItemPage item_page_;
  Pane* current_pane_;
  SubMenu sub_menu_;

  void Update(savedata::PokemonParam& pokemon, Controller& controller);
  void MoveAcrossPanes(Controller& controller);
  void ChangeMode(Mode mode);
  void Draw(Pane* panes[], u32 pane_count, PokemonDataAccessor& accessor,
            AppLayoutManager& manager) const;
};
}