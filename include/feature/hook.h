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
 * @brief Handles function redirection (Hooking) via instruction overwriting.
 * * This class implements an INLINE hook by replacing function headers with
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
  Hook() = default;

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
  void Initialize(u32 src, u32 dst) {
    src_addr_ = src;
    dst_addr_ = dst;
  }

  /**
   * @brief Enables the hook.
   * Replaces the first 8 bytes of the source function with an absolute jump
   * (ldr pc, [pc, #-4] followed by the destination address). Flushes the data
   * and instruction caches to ensure safe execution.
   * This method is reentrant; if the hook is already enabled, it does nothing.
   */
  void Enable(bool force = false) {
    if (!force && is_enabled_) return;

    // Save the original first two instructions (8 bytes) of the source function
    original_code_[0] = READ(vu32, src_addr_);
    original_code_[1] = READ(vu32, src_addr_ + 4);
    svcFlushProcessDataCache(0xFFFF8001, (uptr)original_code_, 8);

    // Setup the gateway to allow calling the original function
    gateway_[0] = READ(vu32, src_addr_);
    gateway_[1] = READ(vu32, src_addr_ + 4);
    gateway_[2] = 0xE51FF004; // ARM opcode for: ldr pc, [pc, #-4]
    gateway_[3] = src_addr_ + 8;
    // Address to jump back to, skipping the hooked bytes
    svcFlushProcessDataCache(0xFFFF8001, (uptr)gateway_, 0x10);

    // Flush caches to prevent CPU from executing stale, cached instructions
    svcInvalidateEntireInstructionCache();

    // Overwrite the start of the function with an absolute jump to our
    // destination
    WRITE(vu32, src_addr_, 0xE51FF004); // ldr pc, [pc, #-4]
    WRITE(vu32, src_addr_ + 4, dst_addr_); // .word dst
    svcFlushProcessDataCache(0xFFFF8001, src_addr_, 8);

    is_enabled_ = true;
  }

  /**
   * @brief Disables the hook.
   * Restores the two original instructions that were overwritten by the jump.
   * This method is reentrant; if the hook is not enabled, it does nothing.
   */
  void Disable() {
    if (!is_enabled_) return;

    // Flush caches to prevent CPU from executing stale, cached instructions
    svcInvalidateEntireInstructionCache();

    // Restore the original instructions
    WRITE(vu32, src_addr_, original_code_[0]);
    WRITE(vu32, src_addr_ + 4, original_code_[1]);
    svcFlushProcessDataCache(0xFFFF8001, src_addr_, 8);

    is_enabled_ = false;
  }

  INLINE bool IsEnabled() const { return is_enabled_; }

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
  bool is_enabled_ = false; ///< Tracking state to prevent double-enabling.
  u32 src_addr_ = 0; ///< Original function entry point address.
  u32 dst_addr_ = 0; ///< Redirected function entry point address.
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