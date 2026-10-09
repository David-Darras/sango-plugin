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
 * @file garc.h
 * @brief The GARC format: the archives of the game.
 *
 * A GARC file has four blocks, one after the other:
 * 1. Garc: the header.
 * 2. Fato: one offset into Fatb for each file id.
 * 3. Fatb: the byte ranges of the files (one range for each language).
 * 4. Fimb: the data of all the files.
 */

#pragma once

#include "common.h"

namespace renderer {
/// The FATO block (File Allocation Table Offsets): one offset into Fatb for
/// each file id.
struct Fato {
  u32 signature; ///< The signature of the block: 'FATO'.
  u32 block_size; ///< The size of this block, not of the full archive.
  u16 file_id_count; ///< The number of file ids in the archive.
  u16 _0;

  /// Returns the offsets after this header: one offset into the data of Fatb
  /// for each file id.
  INLINE const u32* GetOffsets() const {
    return (const u32*)((uptr)this + sizeof(Fato));
  }

  /// Returns the offset of a file id into the data of Fatb.
  INLINE u32 GetOffset(u32 file_id) const {
    return GetOffsets()[file_id];
  }
};

/// The byte range of one file. The offsets start at the data of Fimb, not at
/// the start of the archive.
struct FatbFileRange {
  u32 start_offset; ///< The offset of the first byte of the file.
  u32 end_offset; ///< The offset after the last byte of the file.

  /// Returns the size of the file in bytes.
  INLINE u32 Size() const { return end_offset - start_offset; }
};

/// The entry of one file id in Fatb. A Fato offset gives its position.
struct FatbEntry {
  /// One bit for each language: the languages that have data for this file
  /// id. One FatbFileRange follows for each bit that is set.
  u32 language_bitmask;

  /// Returns the ranges after this header.
  INLINE const FatbFileRange* GetRanges() const {
    return (const FatbFileRange*)((uptr)this + sizeof(u32));
  }

  /// Returns true when the file id has data for this language.
  INLINE bool HasLanguage(u32 lang_index) const {
    return (language_bitmask & (1u << lang_index)) != 0;
  }

  /// Returns the range of a language. The ranges exist only for the set bits
  /// of language_bitmask, in bit order: the index of the range is the number
  /// of set bits below `lang_index`.
  INLINE const FatbFileRange* GetRange(u32 lang_index) const {
    u32 lower_bits = language_bitmask & ((1u << lang_index) - 1);
    return &GetRanges()[__builtin_popcount(lower_bits)];
  }
};

/// The FATB block (File Allocation Table Block): the byte ranges of all the
/// file ids, with one range for each language.
struct Fatb {
  u32 signature; ///< The signature of the block: 'FATB'.
  u32 block_size; ///< The size of this block, not of the full archive.
  /// The number of files. Each language of a file counts as one file.
  u32 file_count;

  /// Returns the entry at a Fato offset.
  INLINE const FatbEntry* GetEntry(u32 fato_offset) const {
    return (const FatbEntry*)((uptr)this + sizeof(Fatb) + fato_offset);
  }
};

/// The FIMB block (File IMage Block): the data of all the files, one after
/// the other, after this header.
struct Fimb {
  u32 signature; ///< The signature of the block: 'FIMB'.
  u32 block_size; ///< The size of this block, not of the full archive.
  u32 data_size; ///< The size of the file data after this header.

  /// Returns the address of the file data.
  INLINE uptr GetData() const {
    return (uptr)this + sizeof(Fimb);
  }
};

/// The header of a GARC file (Game ARChive).
struct Garc {
  u32 signature; ///< The signature of the block: 'GARC'.
  u32 block_size; ///< The size of this block, not of the full archive.
  u16 byte_order; ///< 0xFEFF: little-endian.
  u16 version;
  u16 block_count; ///< The number of blocks after this header.
  u16 _0;
  /// The size of the header, Fato and Fatb: the offset of the data of Fimb.
  u32 blocks_before_fimb_size;
  u32 archive_size; ///< The size of the full .garc file.
  u32 largest_file_size; ///< The size of the biggest file of the archive.

  /// Returns the Fato block.
  INLINE const Fato* GetFato() const {
    return (const Fato*)((uptr)this + sizeof(Garc));
  }

  /// Returns the Fatb block.
  INLINE const Fatb* GetFatb() const {
    const Fato* fato = GetFato();
    return (const Fatb*)((uptr)fato + sizeof(Fato) +
                         fato->file_id_count * sizeof(u32));
  }

  /// Returns the Fimb block.
  INLINE const Fimb* GetFimb() const {
    return (const Fimb*)((uptr)this + blocks_before_fimb_size);
  }

  /// Returns the number of file ids.
  INLINE u32 GetFileCount() const {
    return GetFato()->file_id_count;
  }

  /// Returns the byte range of a file for a language.
  INLINE const FatbFileRange* GetFileRange(u32 file_id,
                                           u32 lang_index = 0) const {
    u32 fato_offset = GetFato()->GetOffset(file_id);
    return GetFatb()->GetEntry(fato_offset)->GetRange(lang_index);
  }

  /// Returns the size of a file in bytes.
  INLINE u32 GetFileSize(u32 file_id, u32 lang_index = 0) const {
    return GetFileRange(file_id, lang_index)->Size();
  }

  /// Returns the address of the data of a file.
  INLINE uptr GetFileAddress(u32 file_id, u32 lang_index = 0) const {
    return GetFimb()->GetData() + GetFileRange(file_id, lang_index)->
           start_offset;
  }
};
} // namespace renderer
