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

#ifndef SANGO_PLUGIN_HOOK_H
#define SANGO_PLUGIN_HOOK_H

#include "common.h"

/**
 * @brief Identifiers for specific input-related function hooks.
 * Used by the HookManager to track and manage multiple hooks.
 */
enum class HookID : u32 {
  kIsKeyPressed,     ///< Hook for Controller::IsKeyPressed
  kIsKeyReleased,    ///< Hook for Controller::IsKeyReleased
  kIsKeyDown,        ///< Hook for Controller::IsKeyDown
  kIsKeyRepeated,    ///< Hook for Controller::IsKeyRepeated
  kIsDPadDown,       ///< Hook for DPad::IsDown
  kIsDPadRepeated,   ///< Hook for DPad::IsRepeated
  kIsTouchDown,      ///< Hook for TouchScreen::IsDown
  kIsTouchReleased,  ///< Hook for TouchScreen::IsReleased
  kUpdateMatrices,
  kUpdateLookAt,
  kSetupBattleConfig,
  kOnStartTurn,
  kPlayBattleAnimation,
  kOnLoadTrainerModel,
  kOnChangeOutlineScale,
  kOnChangeAmbientLightColor,
  kOnChangeDiffuseLightColor,
  kMax
};

/**
 * @brief Handles function redirection (Hooking) via instruction overwriting.
 * * This class implements an inline hook by replacing function headers with
 * an absolute jump. It uses a gateway (trampoline) to execute the original
 * logic.
 */
class Hook {
 public:
  /**
   * @brief Default constructor.
   * Used exclusively to initialize an empty array of hooks within the
   * HookManager.
   */
  Hook();

  /**
   * @brief Initializes and enables a new hook.
   * Used by the HookManager to populate its hook array. This constructor
   * automatically sets up the gateway and enables the hook upon creation.
   *
   * @param id   The identifier of the hook.
   * @param src  The address of the original source function to be hooked.
   * @param dst  The address of the destination function to execute instead of
   * src.
   */
  void Initialize(u32 src, u32 dst);

  /**
   * @brief Enables the hook.
   * Replaces the first 8 bytes of the source function with an absolute jump
   * (ldr pc, [pc, #-4] followed by the destination address). Flushes the data
   * and instruction caches to ensure safe execution.
   * This method is reentrant; if the hook is already enabled, it does nothing.
   */
  void Enable();

  /**
   * @brief Disables the hook.
   * Restores the two original instructions that were overwritten by the jump.
   * This method is reentrant; if the hook is not enabled, it does nothing.
   */
  void Disable();

  bool IsEnabled() const { return is_enabled_; }

  /**
   * @brief Calls the original, unhooked function through the gateway.
   * @tparam R    The return type of the original function.
   * @tparam Args The argument types of the original function.
   * @param args  The arguments to pass to the original function.
   * @return      The result of the original function call.
   */
  template <typename R, typename... Args>
  R CallOriginal(Args... args) {
    using FunctionType = R (*)(Args...);
    // The gateway contains the first instructions + a jump back to src + 8
    FunctionType func = reinterpret_cast<FunctionType>(&gateway_[0]);
    return func(args...);
  }

 private:
  bool is_enabled_;  ///< Tracking state to prevent double-enabling.
  u32 src_addr_;     ///< Original function entry point address.
  u32 dst_addr_;     ///< Redirected function entry point address.
  /**
   * @brief Backup of the first two instructions (8 bytes) of the source.
   */
  u32 original_code_[2];
  /**
   * @brief Trampoline code to jump back to original logic.
   * Usually contains: [Orig Inst 1][Orig Inst 2][LDR PC, [PC, #-4]][Src + 8]
   */
  u32 gateway_[4];
};

#endif  // SANGO_PLUGIN_HOOK_H