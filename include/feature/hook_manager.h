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

#include "feature/hook.h"

/**
 * @brief Identifiers for specific input-related function hooks.
 * Used by the HookManager to track and manage multiple hooks.
 */
enum class HookID : u32 {
  kIsKeyPressed, ///< Hook for Controller::IsKeyPressed
  kIsKeyReleased, ///< Hook for Controller::IsKeyReleased
  kIsKeyDown, ///< Hook for Controller::IsKeyDown
  kIsKeyRepeated, ///< Hook for Controller::IsKeyRepeated
  kIsDPadDown, ///< Hook for DPad::IsDown
  kIsDPadRepeated, ///< Hook for DPad::IsRepeated
  kIsTouchDown, ///< Hook for TouchScreen::IsDown
  kIsTouchReleased, ///< Hook for TouchScreen::IsReleased
  kUpdateMatrices,
  kUpdateLookAt,
  kSetupBattleConfig,
  kSetupTrainerBattleConfig,
  kOnStartTurn,
  kPlayBattleAnimation,
  kOnLoadTrainerModel,
  kChangeOutlineScale,
  kChangeAmbientLightColor,
  kChangeDiffuseLightColor,
  kDrawPicture,
  kDrawTextBox,
  kGetMapTile,
  kOnLoadCroFile,
  kOnSaveGameData,
  kUpdateFrame,
  kCreatePokemonModel,
  kCallApp,
  kLoadShopItems,
  kUpdateExp,
  kCheckAppRequest,
  kLoadEvolveTable,
  kCheckEvolve,
  kLoadPokemonIconTexture,
  kUpdateAppStatus,
  kGlobalDataItemGetParam,
  kAddPokemonToTeam,
  kMax
};

/**
 * @brief Singleton registry for managing the lifecycle of all plugin hooks.
 */
class HookManager {
  MAKE_SINGLETON(HookManager)

public:
  /**
 * @brief Registers and creates a new hook in the manager.
 * @param id   The identifier for the hook.
 * @param src  The source address to hook.
 * @param dst  The destination address to redirect execution to.
 */
  void Add(HookID id, u32 src, u32 dst, bool enable) {
    if (id >= HookID::kMax) return;
    if (hooks_[(u32)id].IsEnabled()) return;
    hooks_[(u32)id].Initialize(src, dst);
    if (enable) {
      hooks_[(u32)id].Enable();
    }
    count_++;
  }

  /**
 * @brief Searches for a registered hook by its ID.
 * @param id The ID of the hook to find.
 * @return Pointer to the Hook if found, nullptr otherwise.
 */
  Hook* Get(HookID id) {
    if (id >= HookID::kMax) return nullptr;
    return &hooks_[(u32)id];
  }

  template <typename R, typename... Args>
  STATIC_INLINE R Call(HookID id, Args... args) {
    return GetInstance().Get(id)->CallOriginal<R>(args...);
  }

  STATIC_INLINE void
  Initialize(HookID id, u32 src, u32 dst, bool enable = true) {
    GetInstance().Add(id, src, dst, enable);
  }

  STATIC_INLINE void
  ForceEnable(HookID id) {
    GetInstance().Get(id)->Enable(true);
  }

private:
  static constexpr int kMaxHooks = (int)HookID::kMax;

  Hook hooks_[kMaxHooks]; ///< Array of pre-allocated Hook objects.
  u32 count_ = 0; ///< Current number of registered hooks.
};