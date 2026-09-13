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

#include <utility>
#include <type_traits>
#include <new>

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

  bool is_some() const { return has_value; }
  bool is_none() const { return !has_value; }

  explicit operator bool() const { return has_value; }

  T& operator*() { return *ptr(); }
  const T& operator*() const { return *ptr(); }

  T* operator->() { return ptr(); }
  const T* operator->() const { return ptr(); }

  T value_or(const T& default_value) const {
    if (has_value) {
      return *ptr();
    }
    return default_value;
  }
};

struct None_t {};
static const None_t None{};

template <typename T>
bool operator==(const Option<T>& opt, None_t) {
  return opt.is_none();
}

template <typename T>
bool operator==(None_t, const Option<T>& opt) {
  return opt.is_none();
}
