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

#include "common.h"
#include "utils.h"

/**
 * @brief Wrapper class for filesystem operations and file handle management.
 * Provides static methods for global FS actions and instance methods for
 * individual file I/O.
 */
class File {
public:
  /**
 * @brief File access and creation modes.
 */
  enum Mode {
    kRead = 1 << 0, ///< Open for reading.
    kWrite = 1 << 1, ///< Open for writing.
    kCreate = 1 << 2, ///< Create file if it does not exist.
  };

  /**
 * @brief Mounts the SD card archive.
 * @param archiveName The mounting point name (defaults to "sdmc:").
 */
  STATIC_INLINE void MountSdmc(const c8* archiveName = "sdmc:") {
    ((void (*)(const c8*))ADDRESS_FS_MOUNT_SDMC)(archiveName);
  }

  /**
 * @brief Creates a new file on the filesystem.
 * @param filename Path to the file.
 * @param size     Initial allocated size for the file.
 */
  STATIC_INLINE void Create(const c16* filename, s64 size = 0) {
    ((void (*)(const c16*, s64))ADDRESS_FS_CREATE_FILE)(filename, size);
  }

  /**
 * @brief Deletes a file from the filesystem.
 * @param filename Path to the file to be removed.
 */
  STATIC_INLINE void Delete(const c16* filename) {
    ((void (*)(const c16*))ADDRESS_FS_DELETE_FILE)(filename);
  }

  STATIC_INLINE void CreateDirectory(const c16* path) {
    ((void (*)(const c16*))ADDRESS_FS_CREATE_DIRECTORY)(path);
  }

  /**
 * @brief Default constructor.
 */
  File() : handle_(nullptr), pos_(0) {
  }

  /**
 * @brief Constructor that opens or resets a file immediately.
 * @param filename Path to the file.
 * @param reset    If true, deletes the existing file before opening.
 */
  File(const c16* filename, bool reset) : handle_(nullptr), pos_(0) {
    if (reset) Delete(filename);
    Open(filename);
  }

  /**
 * @brief Destructor. Automatically closes the file handle if open.
 */
  ~File() { Close(); }

  /**
 * @brief Opens a file with the specified modes.
 * @param filename Path to the file.
 * @param mode     Bitmask of Mode flags.
 */
  INLINE void Open(const c16* filename, u32 mode = kRead | kWrite | kCreate) {
    handle_ = nullptr;
    pos_ = 0;
    ((void (*)(void**, const c16*, u32))ADDRESS_FILE_OPEN)(&handle_, filename,
      mode);
  }

  /**
 * @brief Whether the last Open() produced a usable handle.
 * Opening without kCreate is how a missing file is detected: the engine
 * leaves the out-parameter alone, so the handle stays null.
 */
  INLINE bool IsOpen() const { return handle_ != nullptr; }

  /**
 * @brief Closes the file handle and releases resources.
 */
  INLINE void Close() {
    if (handle_) {
      // Vtable call to close the handle (offset 44)
      (*(void (**)())(*(u32*)handle_ + 44))();
      handle_ = nullptr;
    }
  }

  /**
 * @brief Reads data from the file into a buffer.
 * @param buffer Destination buffer.
 * @param size   Number of bytes to read.
 * @param offset Optional offset to apply before reading.
 */
  INLINE s32 Read(void* buffer, u32 size, s64 offset = 0) {
    s32 out = 0;
    pos_ += offset;
    ((void (*)(s32*, void*, s64, void*, u32))ADDRESS_FILE_READ)(
        &out, handle_, pos_, buffer, size);
    pos_ += size;
    return out;
  }

  /**
 * @brief Reads a whole file into a buffer, in one call.
 * @param buffer   Destination buffer.
 * @param capacity Size of the buffer.
 * @return Bytes read, or 0 if the file is missing, empty, or larger than
 *         the buffer.
 * @note Asking for more than the file holds is how the size is learned:
 *       the engine reports what it actually read, so no separate stat
 *       call is needed.
 */
  static u32 ReadAll(const c16* filename, void* buffer, u32 capacity) {
    File file;
    file.Open(filename, kRead);
    if (!file.IsOpen()) return 0;
    const s32 read = file.Read(buffer, capacity);
    if (read <= 0 || (u32)read > capacity) return 0;
    return (u32)read;
  }

  /**
 * @brief Writes data from a buffer to the file.
 * @param buffer Source buffer.
 * @param size   Number of bytes to write.
 * @param offset Optional offset to apply before writing.
 * @param flush  If true, flushes the write buffer to disk.
 */
  INLINE void Write(const void* buffer, u32 size, s64 offset = 0,
                    bool flush = true) {
    s32 out;
    pos_ += offset;
    ((void (*)(s32*, void*, s64, const void*, u32, bool))ADDRESS_FILE_WRITE)(
        &out, handle_, pos_, buffer, size, flush);
    pos_ += size;
  }

  /**
 * @brief Formats and writes a UTF-16 string to the file.
 * @param in  Format string (printf style).
 * @param ... Variadic arguments for formatting.
 */
  void WriteText(const c16* in, ...) {
    c16 out[BUFFER_SIZE];
    va_list args;
    va_start(args, in);
    ((void (*)(c16*, u32, const c16*, va_list))ADDRESS_STD_VSWPRINTF)(
        out, BUFFER_SIZE, in, args);
    va_end(args);
    Write(out, Utils::GetSize(out));
  }

private:
  void* handle_; ///< Internal engine handle for the opened file.
  s64 pos_; ///< Current cursor position in the file.
};