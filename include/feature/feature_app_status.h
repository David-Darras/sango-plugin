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
#include "game/renderer/app_layout_manager.h"
#include "game/savedata/pokemon_team.h"

namespace feature {
class Pane;

class AppStatus {
  MAKE_SINGLETON(AppStatus)
public:
  STATIC_INLINE void Initialize() {
    HookManager::Initialize(HookID::kAppStatusSetupGraphics,
                            ADDRESS_APP_STATUS_SETUP_GRAPHICS,
                            (uptr)SetupGraphicsHook, false);
  }

  static void PatchOnLoad();
  static void PatchOnUpdate();
  static void SetupGraphicsHook(uptr self,
                                savedata::PokemonParam* pokemon);

  enum class Mode : u8 {
    kIdle,
    kNavigation,
    kEditing,
  };

  enum class Page : u8 {
    kStat,
    kIv,
    kEv,
    kMax
  };

  bool& is_game_input_disabled_ = DeviceState::GetInstance().
      use_redirection;

  s8 slot_; // selected pokemon
  Mode mode_;
  Page page_;
  Pane* current_pane_;

  void Update(savedata::PokemonParam& pokemon, Controller& controller);
  void MoveAcrossPanes(Controller& controller);
  void ChangeMode(Mode mode);
  void Draw(PokemonDataAccessor& accessor, AppLayoutManager& manager) const;
};
}