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
 * @file heap_buffer.h
 * @brief A buffer of the heap for the time of a block.
 *
 * The plugin, its data and its heap share the memory of the .plgInfo
 * (MemorySize). A static buffer uses its memory all the time. A buffer that a
 * function needs only while it reads or writes a file comes from the heap,
 * and goes back at the end of the block.
 *
 * @code
 * core::HeapBuffer buffer(kTextLimit);
 * if (!buffer.IsValid()) return false;  // The heap is full.
 * const u32 size = sys::File::ReadAll(path, buffer.Get(), buffer.Size());
 * @endcode
 */

#pragma once

#include <cstdlib>

#include "core/types.h"

namespace core {

/// A buffer of `size` bytes of the heap. The destructor frees it.
class HeapBuffer {
public:
  explicit HeapBuffer(u32 size)
    : data_(static_cast<u8*>(std::malloc(size))), size_(size) {}
  ~HeapBuffer() { std::free(data_); }

  HeapBuffer(const HeapBuffer&) = delete;
  HeapBuffer& operator=(const HeapBuffer&) = delete;

  /// Returns false when the heap has no room for the buffer.
  bool IsValid() const { return data_ != nullptr; }
  /// Returns the bytes (null when the heap is full).
  u8* Get() const { return data_; }
  /// Returns the bytes as an array of T.
  template <typename T>
  T* As() const {
    return reinterpret_cast<T*>(data_);
  }
  /// Returns the size in bytes.
  u32 Size() const { return size_; }

private:
  u8* data_;
  u32 size_;
};

} // namespace core
