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
 * @file file.h
 * @brief Reads and writes files on the SD card.
 *
 * The paths start with "sdmc:/". They are UTF-16 texts: u"sdmc:/sango/file.bin".
 */

#pragma once

#include "common.h"
#include "core/utils.h"

namespace sys {

/**
 * @brief A file on the SD card.
 *
 * @code
 * sys::File file(u"sdmc:/sango/log.txt", true);  // true: empty the file.
 * file.WriteText(u"Level %u\n", level);
 * @endcode
 */
class File {
public:
  enum Mode : u32 {
    kRead = 1 << 0,
    kWrite = 1 << 1,
    kCreate = 1 << 2,
  };

  /// Makes the SD card available. The library calls it at start-up.
  STATIC_INLINE void MountSdmc(const c8* archiveName = "sdmc:") {
    ((void (*)(const c8*))address::kFsMountSdmc)(archiveName);
  }

  /// Makes an empty file.
  STATIC_INLINE void Create(const c16* filename, s64 size = 0) {
    ((void (*)(const c16*, s64))address::kFsCreateFile)(filename, size);
  }

  /// Deletes a file.
  STATIC_INLINE void Delete(const c16* filename) {
    ((void (*)(const c16*))address::kFsDeleteFile)(filename);
  }

  /// Makes a folder.
  STATIC_INLINE void CreateDirectory(const c16* path) {
    ((void (*)(const c16*))address::kFsCreateDirectory)(path);
  }

  File() : handle_(nullptr), pos_(0) {
  }

  /// Opens a file. `reset`: delete the file first.
  File(const c16* filename, bool reset) : handle_(nullptr), pos_(0) {
    if (reset) Delete(filename);
    Open(filename);
  }

  ~File() { Close(); }

  /// Opens a file. By default, the file is made if it does not exist.
  INLINE void Open(const c16* filename, u32 mode = kRead | kWrite | kCreate) {
    handle_ = nullptr;
    pos_ = 0;
    ((void (*)(void**, const c16*, u32))address::kFileOpen)(&handle_, filename,
      mode);
  }

  /// Returns true when the file is open.
  INLINE bool IsOpen() const { return handle_ != nullptr; }

  /// Closes the file. The destructor also closes it.
  INLINE void Close() {
    if (handle_) {
      ((void (*)(void*))address::kFileClose)(handle_);
      handle_ = nullptr;
    }
  }

  /// Reads `size` bytes after a jump of `offset` bytes. Returns the number of bytes read.
  INLINE s32 Read(void* buffer, u32 size, s64 offset = 0) {
    s32 out = 0;
    pos_ += offset;
    ((void (*)(s32*, void*, s64, void*, u32))address::kFileRead)(
        &out, handle_, pos_, buffer, size);
    pos_ += size;
    return out;
  }

  /// Reads a full file into `buffer`. Returns the size, or 0 when it fails.
  static u32 ReadAll(const c16* filename, void* buffer, u32 capacity) {
    File file;
    file.Open(filename, kRead);
    if (!file.IsOpen()) return 0;
    const s32 read = file.Read(buffer, capacity);
    if (read <= 0 || (u32)read > capacity) return 0;
    return (u32)read;
  }

  /// Writes `size` bytes after a jump of `offset` bytes.
  INLINE void Write(const void* buffer, u32 size, s64 offset = 0,
                    bool flush = true) {
    s32 out;
    pos_ += offset;
    ((void (*)(s32*, void*, s64, const void*, u32, bool))address::kFileWrite)(
        &out, handle_, pos_, buffer, size, flush);
    pos_ += size;
  }

  /// Writes a formatted UTF-16 text (128 characters at most).
  void WriteText(const c16* in, ...) {
    c16 out[address::kBufferSize];
    va_list args;
    va_start(args, in);
    ((void (*)(c16*, u32, const c16*, va_list))address::kStdVswprintf)(
        out, address::kBufferSize, in, args);
    va_end(args);
    Write(out, core::Utils::GetLength(out) * sizeof(c16));
  }

private:
  void* handle_;
  s64 pos_;
};

} // namespace sys
