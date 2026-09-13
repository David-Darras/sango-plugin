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
#include "core/patch/device_patch.h"
#include "core/hook_manager.h"
#include "core/native/process_manager.h"
#include "renderer/native/app_layout_manager.h"
#include "savedata/native/pokemon_team.h"

namespace ui {
class Pane;

class AppStatus {
  MAKE_SINGLETON(AppStatus)
public:
  /// Read-only variant of the editor: only what a player is meant to touch
  /// (nature, ability among the species' own, forms) stays editable.
  bool is_restricted = false;

  STATIC_INLINE void Initialize() {
    core::HookManager::Initialize(HookId::kAppStatusSetupGraphicsParams,
                            renderer::address::kAppStatusSetupGraphicsParams,
                            (uptr)SetupGraphicsParamsHook, false);
    core::HookManager::Initialize(HookId::kAppStatusSetupGraphicsMoves,
                            renderer::address::kAppStatusSetupGraphicsMoves,
                            (uptr)SetupGraphicsMovesHook, false);
    core::HookManager::Initialize(HookId::kAppStatusSetupGraphicsContest,
                            renderer::address::kAppStatusSetupGraphicsContest,
                            (uptr)SetupGraphicsContestHook, false);
    core::HookManager::Initialize(HookId::kAppStatusSetupGraphicsInfos,
                            renderer::address::kAppStatusSetupGraphicsInfos,
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

  bool& is_game_input_disabled_ = core::DevicePatch::GetInstance().
      use_redirection;

  static uptr GetThis() {
    return (uptr)core::ProcessManager::GetInstance().
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

  void Update(savedata::PokemonParam& pokemon, sys::Controller& controller);
  void MoveAcrossPanes(sys::Controller& controller);
  void ChangeMode(Mode mode);
  void Draw(Pane* panes[], u32 pane_count, pokemon::DataAccessor& accessor,
            AppLayoutManager& manager) const;
};

} // namespace ui
