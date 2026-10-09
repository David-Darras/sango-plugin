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
 * @file hook.h
 * @brief The hooks: the redirection of a function of the game to a function
 * of the plugin.
 *
 * @see docs/concepts/hooks-and-addresses.md
 * @see docs/tutorials/03-add-a-hook.md
 */

#pragma once

#include "common.h"

namespace core {

/**
 * @brief The part of a hook that does not depend on the function type.
 *
 * Use core::Hook or the macro HOOK() instead of this class.
 */
class HookBase {
public:
  HookBase(const HookBase&) = delete;
  HookBase& operator=(const HookBase&) = delete;

  /// Writes the jump into the game function. With `force`, writes it again
  /// also when the hook is enabled (the game loaded the code again).
  void Enable(bool force = false);
  /// Writes the original instructions back: the game function is normal
  /// again.
  void Disable();

  /// Returns true when the jump is in the game code.
  bool IsEnabled() const { return is_enabled_; }
  /// Returns true when the gateway exists: the original function can run.
  bool IsInitialized() const { return is_initialized_; }

  /// Installs the hooks of HOOK() that have no process.
  /// plugin::InitializeEngine() calls it.
  static void InstallAll();
  /// Enables again the hooks of the process with this vtable.
  /// core::ProcessPatch calls it each time that the game loads a process.
  static void OnProcessLoad(uptr vtable);

protected:
  HookBase() = default;
  /// Registers a hook of HOOK(). InstallAll() installs it.
  HookBase(u32 address, u32 function, bool has_process, uptr process);

  /// Installs the hook now, or at each load of the process.
  void Install(u32 address, u32 function, bool has_process, uptr process);

  /// The two original instructions, then a jump back to the game function.
  u32 gateway_[4] = {};

private:
  /// Adds the hook to the list of InstallAll() and OnProcessLoad().
  void Link();

  u32 address_ = 0; ///< The address of the game function.
  u32 function_ = 0; ///< The address of the plugin function.
  uptr process_ = 0; ///< The vtable of the process of the code.
  u32 original_code_[2] = {};
  bool has_process_ = false; ///< true: the code is in a CRO.
  bool is_automatic_ = false; ///< true: a hook of HOOK().
  bool is_linked_ = false;
  bool is_enabled_ = false;
  bool is_initialized_ = false;
  HookBase* next_ = nullptr;

  static HookBase* first_;
};

template <typename Signature>
class Hook;

/**
 * @brief Redirects one function of the game to a function of the plugin.
 *
 * The hook replaces the first two instructions of the game function with a
 * jump to the plugin function. A gateway keeps the two instructions and a
 * jump back. The call operator runs the original function with the gateway.
 *
 * The type of the hook is the type of the game function. The compiler
 * checks the plugin function and the parameters of the original function.
 *
 * @code
 * namespace {
 * core::Hook<bool(u32, u32)> is_shiny_hook;
 *
 * bool IsShinyHook(u32 id, u32 pid) {
 *   return is_shiny_hook(id, pid); // Runs the original function.
 * }
 * } // namespace
 *
 * void Shiny::Initialize() {
 *   is_shiny_hook.Install(address::kIsShiny, IsShinyHook);
 * }
 * @endcode
 *
 * The macro HOOK() is shorter: it also installs the hook.
 *
 * @tparam R The return type of the game function.
 * @tparam Args The parameter types of the game function.
 */
template <typename R, typename... Args>
class Hook<R(Args...)> : public HookBase {
public:
  /// The type of the plugin function: the type of the game function.
  typedef R (*Function)(Args...);

  /// Makes a hook that the code installs later with Install().
  Hook() = default;

  /// Makes a hook that InstallAll() installs. HOOK() uses it.
  Hook(Function function, u32 address)
      : HookBase(address, ToAddress(function), false, 0) {}

  /// Makes a hook in the code of a process. The plugin enables it at each
  /// load of the process. HOOK() uses it.
  Hook(Function function, u32 address, uptr process)
      : HookBase(address, ToAddress(function), true, process) {}

  /// Installs the hook now. Nothing occurs when `address` is 0 (the address
  /// is not known for this game yet).
  void Install(u32 address, Function function) {
    HookBase::Install(address, ToAddress(function), false, 0);
  }

  /**
   * @brief Installs a hook in the code of a process (a CRO).
   *
   * The code of a CRO is in memory only while its process runs. The plugin
   * enables the hook each time that the game loads the process.
   *
   * @param address The address of the game function.
   * @param function The plugin function.
   * @param process The vtable of the process, for example
   * battle::address::kVtable.
   */
  void Install(u32 address, Function function, uptr process) {
    HookBase::Install(address, ToAddress(function), true, process);
  }

  /// Runs the original function of the game.
  R operator()(Args... args) const {
    return reinterpret_cast<Function>(reinterpret_cast<uptr>(gateway_))(
        args...);
  }

private:
  static u32 ToAddress(Function function) {
    return reinterpret_cast<u32>(function);
  }
};

} // namespace core

/**
 * @brief Declares a hook and its function. The plugin installs it at the
 * start (plugin::InitializeEngine()).
 *
 * @param R The return type of the game function.
 * @param name The name of the hook. It must be unique in the namespace.
 * @param params The parameters of the game function, in parentheses.
 * @param ... The address of the game function. For a function in a CRO, also
 * the vtable of its process: the plugin enables the hook at each load of
 * the process.
 *
 * In the body, `original(...)` runs the original function of the game.
 * Outside, `name::original` is the core::Hook: `name::original.Disable()`.
 *
 * @code
 * HOOK(bool, AddPokemonToTeam,
 *      (savedata::PokemonTeam* team, savedata::PokemonParam* pokemon),
 *      pokemon::address::kAddPokemonToTeam) {
 *   if (team == nullptr) return false;
 *   return original(team, pokemon);
 * }
 *
 * HOOK(void, UpdateGauge, (void* gauge),
 *      battle::address::kUpdateGauge, battle::address::kVtable) {
 *   original(gauge);
 * }
 * @endcode
 */
#define HOOK(R, name, params, ...)                                    \
  namespace {                                                         \
  struct name {                                                       \
    static R Run params;                                              \
    static ::core::Hook<R params> original;                           \
  };                                                                  \
  }                                                                   \
  ::core::Hook<R params> name::original(&name::Run, __VA_ARGS__);     \
  R name::Run params
