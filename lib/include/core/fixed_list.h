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
 * @file fixed_list.h
 * @brief A list with a fixed capacity, without memory allocation.
 *
 * The plugin cannot allocate memory freely: each feature keeps its entries in
 * a fixed array. core::FixedList is this array and its number of entries.
 *
 * @code
 * core::FixedList<MoveSpec, 128> moves_;
 *
 * if (moves_.IsFull()) {
 *   core::Problems::Add(u"move %u: more than %u new moves", id, moves_.Capacity());
 *   return false;
 * }
 * moves_.Add(spec);
 *
 * const MoveSpec* spec =
 *     moves_.Find([id](const MoveSpec& entry) { return entry.id == id; });
 * for (const MoveSpec& entry : moves_) { ... }
 * @endcode
 */

#pragma once

#include "core/types.h"

namespace core {

/// A list of N values at most. The values stay in the object.
template <typename T, u32 N>
class FixedList {
public:
  // constexpr: a singleton with a list stays a constant of the program (see
  // core::Event).
  constexpr FixedList() : items_(), count_(0) {}

  /// Adds a value at the end. Returns false when the list is full.
  bool Add(const T& value) {
    if (count_ >= N) return false;
    items_[count_++] = value;
    return true;
  }

  /// Returns the first value for which `predicate(value)` is true, or null.
  template <typename Predicate>
  T* Find(Predicate predicate) {
    for (u32 i = 0; i < count_; i++) {
      if (predicate(items_[i])) return &items_[i];
    }
    return nullptr;
  }
  template <typename Predicate>
  const T* Find(Predicate predicate) const {
    for (u32 i = 0; i < count_; i++) {
      if (predicate(items_[i])) return &items_[i];
    }
    return nullptr;
  }

  /// Removes the values for which `predicate(value)` is true. The other
  /// values keep their order. Returns the number of removed values.
  template <typename Predicate>
  u32 RemoveIf(Predicate predicate) {
    u32 kept = 0;
    for (u32 i = 0; i < count_; i++) {
      if (predicate(items_[i])) continue;
      if (kept != i) items_[kept] = items_[i];
      kept++;
    }
    const u32 removed = count_ - kept;
    count_ = kept;
    return removed;
  }

  /// Removes all the values.
  void Clear() { count_ = 0; }

  /// Returns the number of values.
  u32 Count() const { return count_; }
  /// Returns the maximum number of values.
  static constexpr u32 Capacity() { return N; }
  bool IsEmpty() const { return count_ == 0; }
  bool IsFull() const { return count_ >= N; }

  /// Returns the value number `index` (0 to Count() - 1).
  T& operator[](u32 index) { return items_[index]; }
  const T& operator[](u32 index) const { return items_[index]; }

  T* begin() { return items_; }
  T* end() { return items_ + count_; }
  const T* begin() const { return items_; }
  const T* end() const { return items_ + count_; }

private:
  T items_[N];
  u32 count_;
};

} // namespace core
