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
 * @file event.h
 * @brief A list of functions that the library calls when something occurs.
 *
 * An event has one main function and some more functions. Several features
 * can listen to the same event: a feature does not remove the function of an
 * other feature.
 *
 * @code
 * // The main function: `=` replaces the main function of the event.
 * overworld::WildEncounter::GetInstance().on_wild_pokemon = OnWildPokemon;
 *
 * // One more function: Subscribe() keeps the other functions.
 * overworld::WildEncounter::GetInstance().on_wild_pokemon.Subscribe(MakeShiny);
 * @endcode
 */

#pragma once

#include "common.h"
#include "core/problems.h"

namespace core {

template <typename Signature, u32 N = 8>
class Event;

/// An event with the parameters `Args`. It keeps 1 main function and N more.
template <u32 N, typename... Args>
class Event<void(Args...), N> {
public:
  /// The type of the functions of the event.
  typedef void (*Listener)(Args...);

  // constexpr: a singleton with an event stays a constant of the program. A
  // singleton that the program builds at run time has a guard, and the guard
  // calls abort() when two threads build it at the same time.
  constexpr Event() : main_(nullptr), listeners_(), count_(0) {}
  Event(const Event&) = delete;
  Event& operator=(const Event&) = delete;

  /// Replaces the main function. Give nullptr to remove it. The functions of
  /// Subscribe() stay.
  Event& operator=(Listener listener) {
    main_ = listener;
    return *this;
  }

  /// Adds one more function. Returns false when the list is full (see the
  /// problems screen). A function that is already in the list stays one time.
  bool Subscribe(Listener listener) {
    if (listener == nullptr) return false;
    for (u32 i = 0; i < count_; i++) {
      if (listeners_[i] == listener) return true;
    }
    if (count_ >= N) {
      Problems::Add(u"event: more than %u functions", N);
      return false;
    }
    listeners_[count_++] = listener;
    return true;
  }

  /// Removes a function of Subscribe().
  void Unsubscribe(Listener listener) {
    for (u32 i = 0; i < count_; i++) {
      if (listeners_[i] != listener) continue;
      for (u32 k = i + 1; k < count_; k++) listeners_[k - 1] = listeners_[k];
      count_--;
      return;
    }
  }

  /// Returns true when the event has no function.
  bool IsEmpty() const { return main_ == nullptr && count_ == 0; }

  /// Calls the main function, then the other functions in the order of
  /// Subscribe(). The library calls it.
  void Emit(Args... args) const {
    if (main_ != nullptr) main_(args...);
    for (u32 i = 0; i < count_; i++) listeners_[i](args...);
  }

private:
  Listener main_;
  Listener listeners_[N];
  u32 count_;
};

} // namespace core
