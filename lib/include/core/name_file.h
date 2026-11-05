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
 * @file name_file.h
 * @brief A list of names that the plugin reads from the SD card.
 */

#pragma once

#include "core/types.h"

namespace core {

/**
 * @brief The names of one kind (the species, the moves...), read from a file.
 *
 * The files are in sdmc:/sango/names, made by tools/gen_data_names.py (see its
 * --names option). A file has its size, the number of names, one offset for
 * each name, then the names one after the other. Load() reads the file at once
 * and adds the address of the file to the offsets: the file is then an array
 * of `const c8*` that WithArray() accepts, and a name costs no copy. The
 * index of a name is its value. Unload() gives the memory back: a page loads
 * its lists when it opens and unloads them when it closes.
 *
 * @code
 * core::NameFile species;
 * if (species.Load("species")) {
 *   app.Add("Species", number).WithArray(species.GetNames(), species.GetCount());
 * }
 * @endcode
 */
class NameFile {
public:
  NameFile() = default;
  ~NameFile() { Unload(); }

  NameFile(const NameFile&) = delete;
  NameFile& operator=(const NameFile&) = delete;

  /// Reads the file of a kind of names. A list that is already loaded is
  /// unloaded first. Returns false when the file is missing or is not valid,
  /// or when the heap is full.
  bool Load(const c8* kind);

  /// Gives the memory of the list back.
  void Unload();

  /// Returns true when the list is loaded.
  bool IsLoaded() const { return data_ != nullptr; }

  /// Returns the number of names.
  u32 GetCount() const { return count_; }

  /// Returns the names, or null when the list is not loaded.
  const c8** GetNames() const { return names_; }

  /// Returns the name of a value, or an empty text.
  const c8* Get(u32 value) const;

  /// Finds a name (the case and the spaces do not count). Returns false when
  /// the list has no such name.
  bool Find(const c8* text, u32* value) const;

private:
  u8* data_ = nullptr;
  const c8** names_ = nullptr;
  u32 count_ = 0;
};

} // namespace core
