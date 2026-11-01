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
 * @file allocation.cc
 * @brief new and delete without exceptions, and a small terminate handler.
 *
 * The plugin has no exceptions (-fno-exceptions). The new of the C++ library
 * throws std::bad_alloc when the heap is full: the exception has no handler,
 * so the plugin calls abort() at once. Here, new gives null: the code checks
 * it (-fcheck-new) and continues without the feature.
 *
 * The terminate handler of the C++ library writes the name of the exception,
 * and needs the demangler of the C++ names (about 30 KiB). This handler only
 * stops the plugin.
 */

#include <cstdlib>
#include <new>

void* operator new(std::size_t size) {
  return std::malloc(size != 0 ? size : 1);
}
void* operator new[](std::size_t size) {
  return std::malloc(size != 0 ? size : 1);
}
void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
  return std::malloc(size != 0 ? size : 1);
}
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
  return std::malloc(size != 0 ? size : 1);
}

void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, const std::nothrow_t&) noexcept {
  std::free(pointer);
}
void operator delete[](void* pointer, const std::nothrow_t&) noexcept {
  std::free(pointer);
}
void operator delete(void* pointer, std::size_t) noexcept {
  std::free(pointer);
}
void operator delete[](void* pointer, std::size_t) noexcept {
  std::free(pointer);
}

namespace __gnu_cxx {
// Replaces the handler of the C++ library (see the top of the file).
void __verbose_terminate_handler() { std::abort(); }
} // namespace __gnu_cxx
