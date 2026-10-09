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
static constexpr u32 kMaxOverrides = 6144;
static OverrideEntry overrides[kMaxOverrides];
static u32 override_count = 0;
static bool override_loaded = false;

static void LoadOverrideIndex() {
  if (override_loaded) return;
  sys::File file;
  if (!OpenSd(&file, u"sdmc:/sango/pokemodel/index.bin")) return;
  override_loaded = true;
  u32 count = 0;
  file.Read(&count, 4);
  if (count > kMaxOverrides) count = kMaxOverrides;
  file.Read(overrides, count * sizeof(OverrideEntry));
  file.Close();
  override_count = count;
}

static u32 OverrideSize(u32 file_id) {
  LoadOverrideIndex();
  u32 low = 0;
  u32 high = override_count;
  while (low < high) {
    const u32 mid = low + (high - low) / 2;
    if (overrides[mid].file_id == file_id) return overrides[mid].size;
    if (overrides[mid].file_id < file_id) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }
  return 0;
}

static u32 LzStoreSize(u32 raw) { return 4 + raw + (raw + 7) / 8; }

static constexpr u32 kOverrideOffset = 0x80000000;
static constexpr u32 kSpeciesTableFile = 0;

static void ServeLz(u32 file_id, void* buffer, u32 size) {
  c16 path[64];
  Utils::Format(path, u"sdmc:/sango/pokemodel/%d.pc", file_id);
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

u32 Archive::GetFileSizeHook(u32* archive, u32 file_id) {
  if (IsArchive(archive, ArchiveId::kBattleMoveAnimation)) {
    const u32 patched = battle::MoveAnimations::PatchedSize(archive, file_id);
    if (patched) return patched;
  }
  if (IsArchive(archive, ArchiveId::kPokemonModel)) {
    if (file_id == kSpeciesTableFile && RawSize()) return RawSize();
    const u32 size = OverrideSize(file_id);
    if (size) return LzStoreSize(size);
  }
  return get_file_size_hook(archive, file_id);
}

u32 Archive::GetFileSizeHook2(u32* archive, u32 file_id) {
  if (IsArchive(archive, ArchiveId::kBattleMoveAnimation)) {
    const u32 patched = battle::MoveAnimations::PatchedSize(archive, file_id);
    if (patched) return patched;
  }
  if (IsArchive(archive, ArchiveId::kPokemonModel)) {
    if (file_id == kSpeciesTableFile && RawSize()) return RawSize();
    const u32 size = OverrideSize(file_id);
    if (size) return LzStoreSize(size);
  }
  return get_file_size2_hook(archive, file_id);
}

void Archive::GetInfoHook(u32* archive, u32 file_id, u32* info) {
  if (info != nullptr && IsArchive(archive, ArchiveId::kPokemonModel)) {
    const u32 size = file_id == kSpeciesTableFile
                       ? RawSize()
                       : LzStoreSize(OverrideSize(file_id));
    if ((file_id == kSpeciesTableFile && RawSize()) || OverrideSize(file_id)) {
      info[0] = info[1] = size;
      info[2] = kOverrideOffset | file_id;
      return;
    }
  }
  get_info_hook(archive, file_id, info);
}

void Archive::ReadHook(u32* archive, u32 offset, u32 size,
                                void* buffer, u32* read) {
  if ((offset & kOverrideOffset) != 0) {
    const u32 file_id = offset & ~kOverrideOffset;
    if (file_id == kSpeciesTableFile) {
      ServeRaw(buffer, size);
    } else {
      ServeLz(file_id, buffer, size);
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
  if (IsArchive(archive, ArchiveId::kPokemonModel)) {
    if (file_id == kSpeciesTableFile && RawSize() &&
        ServeRaw(buffer, RawSize())) {
      return RawSize();
    }
    const u32 size = OverrideSize(file_id);
    if (size) {
      ServeLz(file_id, buffer, LzStoreSize(size));
      return LzStoreSize(size);
    }
  }
  return load_data2_hook(archive, file_id, buffer);
}

void* Archive::LoadCompressedHook(u32* archive, u32 file_id, void* heap_work,
                                  void* heap_data, s32 align, u32* out_size) {
  if (IsArchive(archive, ArchiveId::kPokemonModel)) {
    const u32 size = OverrideSize(file_id);
    if (size) {
      void* buffer = ((void* (*)(void*, u32, u32))sys::address::kHeapAlloc)(
          heap_data, size, align ? align : 4);
      if (buffer != nullptr) {
        c16 path[64];
        Utils::Format(path, u"sdmc:/sango/pokemodel/%d.pc", file_id);
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
