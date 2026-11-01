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
 * @file game_function.h
 * @brief Calls a function of the game at an address.
 *
 * Write the signature of the function one time, in the same form as for
 * core::Hook:
 *
 * @code
 * // Before: ((void (*)(PokemonTeam*))address::kHealTeam)(this);
 * core::GameFunction<void(PokemonTeam*)>::Call(address::kHealTeam, this);
 * @endcode
 */

#pragma once

#include <types.h>

namespace core {

template <typename Signature>
struct GameFunction;

/// The function of the game with the type `R(Args...)`.
template <typename R, typename... Args>
struct GameFunction<R(Args...)> {
  /// The type of a pointer to the function.
  typedef R (*Pointer)(Args...);

  /// Calls the function of the game at `address`.
  static inline __attribute__((always_inline)) R Call(uptr address,
                                                      Args... args) {
    return reinterpret_cast<Pointer>(address)(args...);
  }
};

} // namespace core
