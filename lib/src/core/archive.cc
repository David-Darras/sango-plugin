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
 * @file archive.cc
 * @brief Lets a product change the files that the game reads from its archives.
 *
 * The declarations are in core/patch/archive.h.
 */

#include "core/patch/archive.h"
#include <cstring>
#include "battle/patch/move_animation.h"
#include "battle/patch/effect_style.h"
#include "core/hook.h"
#include "core/utils.h"
#include "system/native/file.h"
#include "system/address.h"

namespace core {

namespace {
core::Hook<bool(void*, ArchiveInput*)> read_file_async_hook;
core::Hook<bool(u32*, void*, u32, void*, u32, u32, u32)> read_file_async2_hook;
core::Hook<void*(u32*, u32, void*, void*, s32, u32*)> load_compressed_hook;
core::Hook<u32(u32*, u32)> get_file_size_hook;
core::Hook<u32(u32*, u32, void*)> load_data2_hook;
core::Hook<u32(u32*, u32)> get_file_size2_hook;
core::Hook<void(u32*, u32, u32*)> get_info_hook;
core::Hook<void(u32*, u32, u32, void*, u32*)> read_hook;
core::Hook<bool(void*, void*, const c16*, u32, u32, u32, void*, u32, void*, void*)>
    file_async_read_hook;
} // namespace

static bool FileStartAsyncReadHook(void* file, void* heap, const c16* filename,
                                   u32 offset, u32 size, u32 size_again,
                                   void* buffer, u32 priority, void* callback,
                                   void* work);

namespace {
} // namespace

void Archive::Initialize() {
  read_file_async_hook.Install(sys::address::kArchiveReadFileAsync,
                               ReadFileAsync);
  read_file_async2_hook.Install(sys::address::kArchiveReadFileAsync2,
                                ReadFileAsync2);
  load_compressed_hook.Install(sys::address::kArchiveLoadCompressedFile,
                               LoadCompressedHook);
  get_file_size_hook.Install(sys::address::kArchiveGetFileSize,
                             GetFileSizeHook);
  load_data2_hook.Install(sys::address::kArchiveLoadData2, LoadDataHook2);
  if (sys::address::kArchiveGetFileSize2) {
    get_file_size2_hook.Install(sys::address::kArchiveGetFileSize2,
                                GetFileSizeHook2);
  }
  file_async_read_hook.Install(sys::address::kFileStartAsyncRead,
                               FileStartAsyncReadHook);
  if (sys::address::kArchiveGetInfo) {
    get_info_hook.Install(sys::address::kArchiveGetInfo, GetInfoHook);
    read_hook.Install(sys::address::kArchiveRead, ReadHook);
  }
}

static bool OpenSd(sys::File* file, const c16* path) {
  static bool mounted = false;
  file->Open(path, sys::File::kRead);
  if (file->IsOpen()) return true;
  if (!mounted) {
    mounted = true;
    sys::File::MountSdmc();
    file->Open(path, sys::File::kRead);
  }
  return file->IsOpen();
}

struct OverrideEntry {
  u32 file_id;
  u32 size;
};

// The files of an archive that the SD card replaces. The index file has the
// number of files, then (file id, size) for each file, sorted by file id.
//
// With `whole_file` false, a file of the SD card has its size (4 bytes), then
// the data of the file without compression. With `whole_file` true, a file of
// the SD card has the same bytes as the file of the archive (compressed), and
// the game reads it as it reads the archive.
struct OverrideSet {
  ArchiveId archive;
  const c16* index_path;
  const c16* file_format; ///< The path of a file, with one `%d`.
  u32 token; ///< The bit that marks a read of this set in an offset.
  bool whole_file;
  OverrideEntry* entries;
  u32 capacity;
  u32 count;
  bool loaded;
};

// The offsets of the real archives are less than 0x40000000. A fake offset has
// the token in its four high bits and the file id in the other bits.
static constexpr u32 kPokemonToken = 0x80000000;
static constexpr u32 kOverworldToken = 0x40000000;
static constexpr u32 kLandToken = 0xC0000000;
static constexpr u32 kAreaToken = 0xD0000000;
static constexpr u32 kMatrixToken = 0xE0000000;
static constexpr u32 kTokenMask = 0xF0000000;
static constexpr u32 kFileIdMask = ~kTokenMask;
static OverrideEntry pokemon_entries[6144];
static OverrideEntry overworld_entries[1024];
static OverrideEntry land_entries[1024];
static OverrideEntry area_entries[512];
static OverrideEntry matrix_entries[512];
static OverrideSet override_sets[] = {
    {ArchiveId::kPokemonModel, u"sdmc:/sango/pokemodel/index.bin",
     u"sdmc:/sango/pokemodel/%d.pc", kPokemonToken, false, pokemon_entries,
     6144, 0, false},
    {ArchiveId::kOverworldModel, u"sdmc:/sango/overworld_model/index.bin",
     u"sdmc:/sango/overworld_model/%d.bin", kOverworldToken, true, overworld_entries,
     1024, 0, false},
    {ArchiveId::kMapLandData, u"sdmc:/sango/land/index.bin",
     u"sdmc:/sango/land/%d.bin", kLandToken, true, land_entries, 1024, 0,
     false},
    {ArchiveId::kOverworldArea, u"sdmc:/sango/area/index.bin",
     u"sdmc:/sango/area/%d.bin", kAreaToken, true, area_entries, 512, 0,
     false},
    {ArchiveId::kMapMatrix, u"sdmc:/sango/matrix/index.bin",
     u"sdmc:/sango/matrix/%d.bin", kMatrixToken, true, matrix_entries, 512, 0,
     false},
};

static OverrideSet* FindSet(const u32* archive) {
  for (OverrideSet& set : override_sets) {
    if (Archive::IsArchive(archive, set.archive)) return &set;
  }
  return nullptr;
}

static OverrideSet* FindSetByToken(u32 offset) {
  for (OverrideSet& set : override_sets) {
    if ((offset & kTokenMask) == set.token) return &set;
  }
  return nullptr;
}

static void LoadOverrideIndex(OverrideSet& set) {
  if (set.loaded) return;
  // Try once: with no index file, the game calls this for each file it reads,
  // and a new try on the SD card each time makes the game freeze.
  set.loaded = true;
  sys::File file;
  if (!OpenSd(&file, set.index_path)) return;
  u32 count = 0;
  file.Read(&count, 4);
  if (count > set.capacity) count = set.capacity;
  file.Read(set.entries, count * sizeof(OverrideEntry));
  file.Close();
  set.count = count;
}

// Returns the size of the file of the SD card, or 0 when it is not there.
static u32 OverrideSize(OverrideSet& set, u32 file_id) {
  LoadOverrideIndex(set);
  u32 low = 0;
  u32 high = set.count;
  while (low < high) {
    const u32 mid = low + (high - low) / 2;
    if (set.entries[mid].file_id == file_id) return set.entries[mid].size;
    if (set.entries[mid].file_id < file_id) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return 0;
}

static u32 LzStoreSize(u32 raw) { return 4 + raw + (raw + 7) / 8; }

// Writes a line "code file size" in sdmc:/sango/overworld_model/trace.log. It shows
// how the game reads the files that the SD card replaces. The codes: 1 size
// of a file, 2 information of a file, 3 read in a thread, 4 read with the
// offset.
static void Trace(OverrideSet* set, u32 what, u32 file_id, u32 size) {
  static sys::File file;
  static bool opened = false;
  static u32 lines = 0;
  if (set == nullptr || !set->whole_file || lines >= 400) return;
  if (!opened) {
    opened = true;
    file.Open(u"sdmc:/sango/overworld_model/trace.log");
  }
  if (!file.IsOpen()) return;
  lines++;
  file.WriteText(u"%u %u %u\n", what, file_id, size);
}

static constexpr u32 kSpeciesTableFile = 0;

static void ServeLz(OverrideSet& set, u32 file_id, void* buffer, u32 size) {
  c16 path[64];
  Utils::Format(path, set.file_format, file_id);
  sys::File file;
  if (!OpenSd(&file, path)) return;
  u32 raw = 0;
  file.Read(&raw, 4);
  u8* out = (u8*)buffer;
  u8* end = out + size;
  const u32 header = 0x11 | (raw << 8);
  std::memcpy(out, &header, 4);
  out += 4;
  u8 chunk[512];
  u32 left = raw;
  while (left > 0 && out < end) {
    const u32 n = left < sizeof(chunk) ? left : sizeof(chunk);
    file.Read(chunk, n);
    left -= n;
    for (u32 k = 0; k < n && out < end; k += 8) {
      *out++ = 0;
      const u32 m = (n - k) < 8 ? (n - k) : 8;
      std::memcpy(out, chunk + k, m);
      out += m;
    }
  }
  file.Close();
}

// Copies a file of the SD card (a set with `whole_file`) to the buffer.
static void ServeFile(OverrideSet& set, u32 file_id, void* buffer, u32 size) {
  c16 path[64];
  Utils::Format(path, set.file_format, file_id);
  sys::File file;
  if (!OpenSd(&file, path)) return;
  file.Read(buffer, size);
  file.Close();
}

static constexpr u32 kMaxRawSize = 0x4000;
static u8 raw_table[kMaxRawSize];
static u32 raw_size = 0;
static bool raw_loaded = false;

static void LoadRawTable() {
  if (raw_loaded) return;
  sys::File file;
  if (!OpenSd(&file, u"sdmc:/sango/pokemodel/0.raw")) return;
  u32 size = 0;
  file.Read(&size, 4);
  if (size != 0 && size <= kMaxRawSize) {
    file.Read(raw_table, size);
    raw_size = size;
    raw_loaded = true;
  }
  file.Close();
}

static u32 RawSize() {
  LoadRawTable();
  return raw_size;
}

static bool ServeRaw(void* buffer, u32 size) {
  LoadRawTable();
  if (raw_size == 0) return false;
  if (size > raw_size) size = raw_size;
  std::memcpy(buffer, raw_table, size);
  return true;
}

// The size of a file that the SD card replaces, as the game reads it. Returns
// 0 when the game must read its own file.
static u32 ReplacedSize(OverrideSet* set, u32 file_id) {
  if (set == nullptr) return 0;
  if (set->archive == ArchiveId::kPokemonModel &&
      file_id == kSpeciesTableFile && RawSize()) {
    return RawSize();
  }
  const u32 size = OverrideSize(*set, file_id);
  if (size == 0) return 0;
  return set->whole_file ? size : LzStoreSize(size);
}

u32 Archive::GetFileSizeHook(u32* archive, u32 file_id) {
  if (IsArchive(archive, ArchiveId::kBattleMoveAnimation)) {
    const u32 patched = battle::MoveAnimations::PatchedSize(archive, file_id);
    if (patched) return patched;
  }
  OverrideSet* set = FindSet(archive);
  const u32 replaced = ReplacedSize(set, file_id);
  if (replaced) {
    Trace(set, 1, file_id, replaced);
    return replaced;
  }
  return get_file_size_hook(archive, file_id);
}

u32 Archive::GetFileSizeHook2(u32* archive, u32 file_id) {
  if (IsArchive(archive, ArchiveId::kBattleMoveAnimation)) {
    const u32 patched = battle::MoveAnimations::PatchedSize(archive, file_id);
    if (patched) return patched;
  }
  OverrideSet* set = FindSet(archive);
  const u32 replaced = ReplacedSize(set, file_id);
  if (replaced) {
    Trace(set, 1, file_id, replaced);
    return replaced;
  }
  return get_file_size2_hook(archive, file_id);
}

void Archive::GetInfoHook(u32* archive, u32 file_id, u32* info) {
  OverrideSet* set = FindSet(archive);
  const u32 replaced = info != nullptr ? ReplacedSize(set, file_id) : 0;
  if (replaced) {
    Trace(set, 2, file_id, replaced);
    info[0] = info[1] = replaced;
    info[2] = set->token | file_id;
    return;
  }
  get_info_hook(archive, file_id, info);
}

void Archive::ReadHook(u32* archive, u32 offset, u32 size,
                                void* buffer, u32* read) {
  OverrideSet* set = FindSetByToken(offset);
  if (set != nullptr) {
    const u32 file_id = offset & kFileIdMask;
    if (set->archive == ArchiveId::kPokemonModel &&
        file_id == kSpeciesTableFile) {
      ServeRaw(buffer, size);
    } else if (set->whole_file) {
      Trace(set, 4, file_id, size);
      ServeFile(*set, file_id, buffer, size);
    } else {
      ServeLz(*set, file_id, buffer, size);
    }
    if (read != nullptr) *read = size;
    return;
  }
  read_hook(archive, offset, size, buffer, read);
}

u32 Archive::LoadDataHook2(u32* archive, u32 file_id, void* buffer) {
  if (IsArchive(archive, ArchiveId::kBattleMoveAnimation)) {
    u32 patched = 0;
    if (battle::MoveAnimations::Serve(archive, file_id, buffer, &patched)) {
      return patched;
    }
  }
  OverrideSet* set = FindSet(archive);
  if (set != nullptr && set->whole_file) {
    // The game reads the whole file at once (a small file, for example).
    const u32 size = OverrideSize(*set, file_id);
    if (size) {
      Trace(set, 5, file_id, size);
      ServeFile(*set, file_id, buffer, size);
      return size;
    }
  }
  if (set != nullptr && !set->whole_file) {
    if (set->archive == ArchiveId::kPokemonModel &&
        file_id == kSpeciesTableFile && RawSize() &&
        ServeRaw(buffer, RawSize())) {
      return RawSize();
    }
    const u32 size = OverrideSize(*set, file_id);
    if (size) {
      ServeLz(*set, file_id, buffer, LzStoreSize(size));
      return LzStoreSize(size);
    }
  }
  return load_data2_hook(archive, file_id, buffer);
}

void* Archive::LoadCompressedHook(u32* archive, u32 file_id, void* heap_work,
                                  void* heap_data, s32 align, u32* out_size) {
  OverrideSet* set = FindSet(archive);
  if (set != nullptr && !set->whole_file) {
    const u32 size = OverrideSize(*set, file_id);
    if (size) {
      void* buffer = ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(
          heap_data, size, align ? align : 4);
      if (buffer != nullptr) {
        c16 path[64];
        Utils::Format(path, set->file_format, file_id);
        sys::File file;
        if (OpenSd(&file, path)) {
          u32 raw = 0;
          file.Read(&raw, 4);
          file.Read(buffer, size);
          file.Close();
          if (out_size != nullptr) *out_size = size;
          return buffer;
        }
        ((void (*)(void*))sys::address::kHeapFree)(buffer);
      }
    }
  }
  return load_compressed_hook(archive, file_id, heap_work, heap_data, align,
                              out_size);
}

// The game reads a part of an archive in a thread, with the offset that the
// archive gave. For a file of the SD card, the offset has the token of the set.
// The plugin copies the file to the buffer at once. The thread then reads four
// bytes of the real archive into a small buffer, so that the game sees the
// end of the read as usual.
static bool FileStartAsyncReadHook(void* file, void* heap, const c16* filename,
                                   u32 offset, u32 size, u32 size_again,
                                   void* buffer, u32 priority, void* callback,
                                   void* work) {
  OverrideSet* set = FindSetByToken(offset);
  if (set != nullptr && set->whole_file) {
    static u32 scratch[4];
    const u32 file_id = offset & kFileIdMask;
    Trace(set, 3, file_id, size);
    ServeFile(*set, file_id, buffer, size);
    return file_async_read_hook(file, heap, filename, 0, 4, 4, scratch,
                                priority, callback, work);
  }
  return file_async_read_hook(file, heap, filename, offset, size, size_again,
                              buffer, priority, callback, work);
}

u32 Archive::GetOriginalFileSize(u32* archive, u32 file_id) {
  return get_file_size_hook(archive, file_id);
}

u32 Archive::LoadOriginalData(u32* archive, u32 file_id, void* buffer) {
  return load_data2_hook(archive, file_id, buffer);
}

bool Archive::IsArchive(const u32* archive_data,
                             const ArchiveId archive_id) {
  u32* archive_table = (u32*)sys::address::kArchiveFilenameTable;
  return archive_data[12] == archive_table[static_cast<u32>(archive_id)];
}

bool Archive::IsArchive(const ArchiveInput* input, const ArchiveId archive_id) {
  return input->archive_id == archive_id;
}

bool Archive::ReadFileAsync2(u32* archive, void* heap, u32 file_id,
                                  void* buffer,
                                  u32 p4, u32 p5, u32 p6) {
  auto& feat = GetInstance();
  if (feat.on_stream_file != nullptr) {
    file_id = feat.on_stream_file(archive, file_id);
  }
  return read_file_async2_hook(archive, heap, file_id, buffer, p4, p5, p6);
}

bool Archive::ReadFileAsync(void* file_manager, ArchiveInput* input) {
  auto& feat = GetInstance();
  battle::EffectStyles::TrackRead(input);
  if (feat.on_read_file != nullptr) feat.on_read_file(input);
  return read_file_async_hook(file_manager, input);
}

} // namespace core
