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

#include "savedata/native/savedata.h"

namespace savedata {
/**
* @class Footer
* @brief Manages save data integrity through checksums and signatures.
* * This class is responsible for verifying that save data has not been
* corrupted or externally tampered with by comparing checksums and
* validating a specific signature.
*/
class Footer {
  SINGLETON(Footer)

public:
  STATIC_INLINE Footer& GetInstance() { return SaveData::GetInstance().GetFooter(); }

private:
  void* vtable_;
  u32 padding0_;

  u64 checksum_;
  u64 last_checksum_;

  /**
* @brief Magic signature for format identification (Expected value: "BEEF").
*/
  u32 signature_;

  struct Entry {
    u32 size;
    u16 id;
    u16 checksum;
  } entries_[SaveData::kSegmentCount];

  u64 padding1_;
};
} // namespace savedata