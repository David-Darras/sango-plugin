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

/// FATO = File Allocation Table Offsets. One entry per file id, pointing
/// to where that file's byte range lives inside Fatb.
struct Fato {
  u32 signature; // Identifies the block: 'FATO'
  u32 block_size; // Size of this block only, NOT the whole archive
  u16 file_id_count; // Number of file ids in this archive
  u16 _0; // Padding

  // The file_id_count offsets right after this header, one per file id,
  // each pointing into Fatb's data (past Fatb's own header).
  INLINE const u32* GetOffsets() const {
    return (const u32*)((uptr)this + sizeof(Fato));
  }

  INLINE u32 GetOffset(u32 file_id) const {
    return GetOffsets()[file_id];
  }
};

/// The byte range of a single sub-file's data, relative to the start of
/// Fimb's data (not the start of the archive).
struct FatbFileRange {
  u32 start_offset; // Offset of the file's first byte
  u32 end_offset; // Offset one past the file's last byte

  INLINE u32 Size() const { return end_offset - start_offset; }
};

/// One file id's entry inside Fatb, reached via a Fato offset.
struct FatbEntry {
  u32 language_bitmask;
  // One bit per language: which languages have data for this file id

  // Followed by one FatbFileRange per bit set in language_bitmask.
  INLINE const FatbFileRange* GetRanges() const {
    return (const FatbFileRange*)((uptr)this + sizeof(u32));
  }

  INLINE bool HasLanguage(u32 lang_index) const {
    return (language_bitmask & (1u << lang_index)) != 0;
  }

  // Ranges only exist for languages present in language_bitmask, packed in
  // bit order, so lang_index has to become its position among the set bits.
  INLINE const FatbFileRange* GetRange(u32 lang_index) const {
    u32 lower_bits = language_bitmask & ((1u << lang_index) - 1);
    return &GetRanges()[__builtin_popcount(lower_bits)];
  }
};

/// FATB = File Allocation Table Block. Holds the actual byte ranges for
/// every file id, including one range per available language variant.
struct Fatb {
  u32 signature; // Identifies the block: 'FATB'
  u32 block_size; // Size of this block only, NOT the whole archive
  u32 file_count;
  // Total sub-file count, counting every language variant separately

  INLINE const FatbEntry* GetEntry(u32 fato_offset) const {
    return (const FatbEntry*)((uptr)this + sizeof(Fatb) + fato_offset);
  }
};

/// FIMB = File IMage Block. Holds the raw file data referenced by Fatb.
struct Fimb {
  u32 signature; // Identifies the block: 'FIMB'
  u32 block_size; // Size of this block only, NOT the whole archive
  u32 data_size; // Total size of the raw file data that follows this header
  // Raw bytes of every sub-file follow immediately after, back to back.

  INLINE uptr GetData() const {
    return (uptr)this + sizeof(Fimb);
  }
};

/// GARC = Game ARChive. Top-level header of the archive file.
struct Garc {
  u32 signature; // Identifies the block: 'GARC'
  u32 block_size; // Size of this block only, NOT the whole archive
  u16 byte_order; // little-endian : 0xFEFF
  u16 version;
  u16 block_count; // Number of blocks after this header
  u16 _0; // Padding
  // Size of header+Fato+Fatb combined = byte offset where Fimb's data starts
  u32 blocks_before_fimb_size;
  u32 archive_size; // Size of the entire .garc file
  u32 largest_file_size; // Size of the biggest sub-file stored in this archive

  INLINE const Fato* GetFato() const {
    return (const Fato*)((uptr)this + sizeof(Garc));
  }

  INLINE const Fatb* GetFatb() const {
    const Fato* fato = GetFato();
    return (const Fatb*)((uptr)fato + sizeof(Fato) +
                         fato->file_id_count * sizeof(u32));
  }

  INLINE const Fimb* GetFimb() const {
    return (const Fimb*)((uptr)this + blocks_before_fimb_size);
  }

  INLINE u32 GetFileCount() const {
    return GetFato()->file_id_count;
  }

  INLINE const FatbFileRange* GetFileRange(u32 file_id,
                                           u32 lang_index = 0) const {
    u32 fato_offset = GetFato()->GetOffset(file_id);
    return GetFatb()->GetEntry(fato_offset)->GetRange(lang_index);
  }

  INLINE u32 GetFileSize(u32 file_id, u32 lang_index = 0) const {
    return GetFileRange(file_id, lang_index)->Size();
  }

  INLINE uptr GetFileAddress(u32 file_id, u32 lang_index = 0) const {
    return GetFimb()->GetData() + GetFileRange(file_id, lang_index)->
           start_offset;
  }
};