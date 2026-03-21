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

#ifndef SANGO_PLUGIN_FOOTER_H
#define SANGO_PLUGIN_FOOTER_H

#include "savedata/savedata.h"

namespace savedata {

/**
 * @class Footer
 * @brief Manages save data integrity through checksums and signatures.
 * * This class is responsible for verifying that save data has not been
 * corrupted or externally tampered with by comparing checksums and
 * validating a specific signature.
 */
class Footer {
 public:
  /**
   * @brief Retrieves the singleton instance of Footer from the global SaveData.
   * @return A reference to the Footer instance.
   */
  Footer& GetInstance() { return SaveData::GetFooter(); }

 private:
  void* vtable_;  ///< Pointer to the virtual method table.
  u32 padding0_;  ///< Memory alignment padding.

  u64 checksum_;       ///< Current checksum of the save data.
  u64 last_checksum_;  ///< Previous valid checksum recorded.

  /**
   * @brief Magic signature for format identification (Expected value: "BEEF").
   */
  u32 signature_;

  /**
   * @struct Entry
   * @brief Individual entry structure for tracking save data segments.
   */
  struct Entry {
    u32 size;      ///< Size of the associated data segment.
    u16 id;        ///< Unique identifier for the entry.
    u16 checksum;  ///< Partial checksum for this specific segment.
  } entries_[SaveData::kSegmentCount];  ///< Array of save segment entries.

  u64 padding1_;  ///< Final padding for structure alignment.
};

}  // namespace savedata

#endif  // SANGO_PLUGIN_FOOTER_H
