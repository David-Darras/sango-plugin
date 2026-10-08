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
 * @file option.h
 * @brief An optional value: a value, or nothing.
 */

#pragma once

#include <utility>
#include <type_traits>
#include <new>

/**
 * @brief A value of type T, or nothing.
 *
 * @code
 * Option<u32> level;          // Nothing.
 * level = 5;                  // A value.
 * if (level) { u32 v = *level; }
 * @endcode
 */
template <typename T>
class Option {
private:
  bool has_value;
  typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;

  T* ptr() {
    return reinterpret_cast<T*>(&storage);
  }

  const T* ptr() const {
    return reinterpret_cast<const T*>(&storage);
  }

public:
  Option() : has_value(false) {}

  Option(const T& value) : has_value(true) {
    new(ptr()) T(value);
  }

  Option(T&& value) noexcept : has_value(true) {
    new(ptr()) T(std::move(value));
  }

  Option(const Option& other) : has_value(other.has_value) {
    if (other.has_value) {
      new(ptr()) T(*other.ptr());
    }
  }

  Option(Option&& other) noexcept : has_value(other.has_value) {
    if (other.has_value) {
      new(ptr()) T(std::move(*other.ptr()));
      other.reset();
    }
  }

  ~Option() {
    reset();
  }

  Option& operator=(const Option& other) {
    if (this != &other) {
      reset();
      has_value = other.has_value;
      if (has_value) {
        new(ptr()) T(*other.ptr());
      }
    }
    return *this;
  }

  Option& operator=(Option&& other) noexcept {
    if (this != &other) {
      reset();
      has_value = other.has_value;
      if (has_value) {
        new(ptr()) T(std::move(*other.ptr()));
        other.reset();
      }
    }
    return *this;
  }

  void reset() {
    if (has_value) {
      ptr()->~T();
      has_value = false;
    }
  }

  /// Returns true when the option contains a value.
  bool is_some() const { return has_value; }
  /// Returns true when the option contains nothing.
  bool is_none() const { return !has_value; }

  explicit operator bool() const { return has_value; }

  T& operator*() { return *ptr(); }
  const T& operator*() const { return *ptr(); }

  T* operator->() { return ptr(); }
  const T* operator->() const { return ptr(); }

  /// Returns the value, or `default_value` when the option contains nothing.
  T value_or(const T& default_value) const {
    if (has_value) {
      return *ptr();
    }
    return default_value;
  }
};

/// The type of None.
struct None_t {};
/// Compares with an option: `option == None` is true when it contains nothing.
static const None_t None{};

template <typename T>
bool operator==(const Option<T>& opt, None_t) {
  return opt.is_none();
}

template <typename T>
bool operator==(None_t, const Option<T>& opt) {
  return opt.is_none();
}
