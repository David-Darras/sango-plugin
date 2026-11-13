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
 * @file studio_files.cc
 * @brief Loads the files that Sango Studio writes on the SD card.
 *
 * The declarations are in core/patch/studio_files.h.
 */

#include "core/patch/studio_files.h"

#include <cstring>

#include "battle/patch/studio_abilities.h"
#include "battle/patch/studio_types.h"
#include "core/patch/archive.h"
#include "core/problems.h"
#include "pokemon/native/species_data.h"
#include "pokemon/patch/species_editor.h"
#include "plugin.h"
#include "system/native/file.h"

namespace core {
namespace {

/// The folder of Sango Studio on the SD card.
constexpr const c16* kReloadPath = u"sdmc:/sango-studio/reload.bin";
constexpr const c16* kPokedexPath =
    u"sdmc:/sango-studio/pokemon/pokedex_data.bin";
const c16 kFolderPrefix[] = u"sdmc:/sango-studio/";
const c8 kSpeciesFolder[] = "pokemon/data/";
const c8 kPokedexFile[] = "pokemon/pokedex_data.bin";
const c8 kAbilityFolder[] = "ability/";
const c8 kTypeFolder[] = "type/";

/// The header of reload.bin: "SREL", the counter, the number of paths.
constexpr u32 kSignature = 0x4C455253;
constexpr u32 kHeaderSize = 12;

/// The whole reload.bin at the last load.
u8 reload_bytes[kHeaderSize + 256 * 64];

u32 frame = 0;

/// Opens a file of the SD card for a read. The SD card is mounted at the
/// first miss.
bool OpenStudio(sys::File* file, const c16* path) {
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

/// Returns the size of a file of the SD card, or 0 when it is not there.
u32 StudioFileSize(const c16* path) {
  sys::File file;
  if (!OpenStudio(&file, path)) return 0;
  u8 chunk[512];
  u32 size = 0;
  while (true) {
    const s32 read = file.Read(chunk, sizeof(chunk));
    if (read <= 0) break;
    size += static_cast<u32>(read);
    if (static_cast<u32>(read) < sizeof(chunk)) break;
  }
  file.Close();
  return size;
}

/// Reads the number at the start of `text` and moves `text` after it.
bool ReadNumber(const c8** text, u32* value) {
  const c8* at = *text;
  u32 out = 0;
  if (*at < '0' || *at > '9') return false;
  while (*at >= '0' && *at <= '9') {
    out = out * 10 + static_cast<u32>(*at - '0');
    if (out > 100000) return false;
    at++;
  }
  *text = at;
  *value = out;
  return true;
}

/// Writes the full path of a file of the folder of the studio.
/// The paths of reload.bin have ASCII characters only.
void FullPath(const c8* relative, c16* out, u32 capacity) {
  u32 at = 0;
  while (kFolderPrefix[at] != 0 && at + 1 < capacity) {
    out[at] = kFolderPrefix[at];
    at++;
  }
  for (u32 i = 0; relative[i] != 0 && at + 1 < capacity; i++) {
    out[at++] = static_cast<c16>(static_cast<u8>(relative[i]));
  }
  out[at] = 0;
}

} // namespace

void StudioFiles::Load() {
#ifdef GAME_ORAS
  RefreshPokedexData();
  plugin::AddFrameListener(Update);
#endif
}

void StudioFiles::Update() {
  // The data of the species is ready in the overworld or in a battle.
  if (!pokemon::SpeciesChanges::IsReady()) return;
  if (++frame % kPollFrames != 0) return;
  battle::StudioAbilities::RefreshLanguage();
  battle::StudioTypes::RefreshLanguage();
  sys::File file;
  if (!OpenStudio(&file, kReloadPath)) return;
  u8 header[kHeaderSize];
  const s32 read = file.Read(header, kHeaderSize);
  file.Close();
  if (read < static_cast<s32>(kHeaderSize)) return;
  u32 signature = 0;
  u32 counter = 0;
  std::memcpy(&signature, header, 4);
  std::memcpy(&counter, header + 4, 4);
  if (signature != kSignature) return;
  auto& self = GetInstance();
  if (self.loaded_ && counter == self.counter_) return;
  const bool first_load = !self.loaded_;
  self.loaded_ = true;
  self.counter_ = counter;
  // reload.bin keeps the last paths only: the first load reads every ability
  // file, so an old ability is not forgotten.
  if (first_load) {
    battle::StudioTypes::LoadAll();
    battle::StudioAbilities::LoadAll();
  }
  LoadAll();
}

void StudioFiles::LoadAll() {
  const u32 size =
      sys::File::ReadAll(kReloadPath, reload_bytes, sizeof(reload_bytes));
  if (size < kHeaderSize) return;
  u32 count = 0;
  std::memcpy(&count, reload_bytes + 8, 4);
  const u32 listed = (size - kHeaderSize) / kPathSize;
  if (count > listed) count = listed;
  if (count > kMaxPaths) count = kMaxPaths;
  for (u32 i = 0; i < count; i++) {
    c8 path[kPathSize + 1];
    std::memcpy(path, reload_bytes + kHeaderSize + i * kPathSize, kPathSize);
    path[kPathSize] = 0;
    Apply(path);
  }
}

void StudioFiles::Apply(const c8* path) {
  const u32 folder_length = sizeof(kSpeciesFolder) - 1;
  if (std::memcmp(path, kSpeciesFolder, folder_length) == 0) {
    ApplySpeciesData(path);
    return;
  }
  if (std::strcmp(path, kPokedexFile) == 0) {
    RefreshPokedexData();
    return;
  }
  if (std::memcmp(path, kTypeFolder, sizeof(kTypeFolder) - 1) == 0) {
    battle::StudioTypes::Apply(path);
    return;
  }
  if (std::memcmp(path, kAbilityFolder, sizeof(kAbilityFolder) - 1) == 0) {
    battle::StudioAbilities::Apply(path);
    return;
  }
  // A file of a later version of the studio: this plugin does not read it.
}

void StudioFiles::ApplySpeciesData(const c8* path) {
#ifdef GAME_ORAS
  using pokemon::SpeciesData;
  u32 species = 0;
  u32 form = 0;
  const c8* at = path + sizeof(kSpeciesFolder) - 1;
  const bool parsed = ReadNumber(&at, &species) && *at == '_' &&
                      (at++, ReadNumber(&at, &form)) &&
                      std::strcmp(at, ".bin") == 0;
  if (!parsed) {
    Problems::Add(u"studio: a species file has a wrong name");
    return;
  }
  const SpeciesData* table = SpeciesData::GetTable();
  if (table == nullptr) return;
  if (species == 0 || species > 1020) {
    Problems::Add(u"studio: the species %u does not exist", species);
    return;
  }
  // A form that the table does not have would write over the base entry.
  if (form != 0) {
    const SpeciesData& base = table[species];
    if (base.form_index == 0 || form >= base.total_form_count) {
      Problems::Add(u"studio: the species %u has no form %u", species, form);
      return;
    }
  }
  c16 full[96];
  FullPath(path, full, 96);
  u8 bytes[sizeof(SpeciesData) + 4];
  const u32 size = sys::File::ReadAll(full, bytes, sizeof(bytes));
  if (size != sizeof(SpeciesData)) {
    Problems::Add(u"studio: the file of the species %u has %u bytes", species,
                  size);
    return;
  }
  auto& entry = SpeciesData::GetInstance(static_cast<SpeciesId>(species),
                                         static_cast<FormId>(form));
  // The places of the forms (and of their models) belong to the table of the plugin: the plugin
  // can move the forms (for example for the forms that it adds).
  const u16 form_index = entry.form_index;
  const u16 form_model_index = entry.form_model_index;
  const u8 total_form_count = entry.total_form_count;
  std::memcpy(&entry, bytes, sizeof(SpeciesData));
  entry.form_index = form_index;
  entry.form_model_index = form_model_index;
  entry.total_form_count = total_form_count;
#else
  (void)path;
#endif
}

void StudioFiles::RefreshPokedexData() {
  Archive::SetPokedexDataFile(StudioFileSize(kPokedexPath));
}

} // namespace core
