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
 * @file species_packs.cc
 * @brief Reads the species and the forms that Sango Studio installs.
 *
 * The declarations are in pokemon/patch/species_packs.h.
 */

#include "pokemon/patch/species_packs.h"

#include <cstring>

#include "core/hook.h"
#include "core/problems.h"
#include "pokemon/address.h"
#include "system/native/file.h"

namespace pokemon {
namespace {

constexpr u32 kHeaderSize = 16;
constexpr u32 kSignature = 0x54505353; // 'SSPT'
constexpr u32 kVersion = 2;

PackSpecies species_list[SpeciesPacks::kMaxSpecies];
PackForm form_list[SpeciesPacks::kMaxForms];
u32 species_count = 0;
u32 form_count = 0;
bool is_tried = false;

// The function of the game that gives the file of an icon: the species, the
// form, the sex, the egg flag, the animation and the flip flag.
core::Hook<u32(u32, u32, u32, bool, u32, bool)> icon_file_hook;

// The game knows the species up to 721 (and the eggs 722 and 723). For a
// species of a pack the table of the icons gives the file, the file of the
// form 0 when the form has none. Without a file the game gets the icon of the
// species 0.
u32 GetIconFileHook(u32 species, u32 form, u32 sex, bool egg, u32 animation,
                    bool flip) {
  if (species >= SpeciesPacks::kFirstSpecies && !egg) {
    u32 file = SpeciesPacks::IconFile(static_cast<u16>(species),
                                      static_cast<u16>(form));
    if (file == 0xFFFFFFFF && form != 0) {
      file = SpeciesPacks::IconFile(static_cast<u16>(species), 0);
    }
    if (file != 0xFFFFFFFF) return file;
    species = 0;
  }
  return icon_file_hook(species, form, sex, egg, animation, flip);
}

// One entry of the table of the icons: the species, the form and the file.
struct IconEntry {
  u16 species;
  u16 form;
  u16 file;
};
constexpr u32 kMaxIcons = 1024;
IconEntry icon_list[kMaxIcons];
u32 icon_count = 0;

// Reads species/icon_map.bin: 'SICM', the count, then the entries.
void LoadIconMap() {
  sys::File file;
  file.Open(u"sdmc:/sango-studio/species/icon_map.bin", sys::File::kRead);
  if (!file.IsOpen()) return;
  u8 header[8];
  if (file.Read(header, 8) != 8) return;
  u32 count;
  std::memcpy(&count, header + 4, 4);
  if (std::memcmp(header, "SICM", 4) != 0 || count > kMaxIcons) {
    core::Problems::Add(u"species/icon_map.bin: wrong signature or size");
    return;
  }
  const u32 bytes = count * sizeof(IconEntry);
  if (bytes != 0 && file.Read(icon_list, bytes) != static_cast<s32>(bytes)) {
    core::Problems::Add(u"species/icon_map.bin: the entries are cut");
    return;
  }
  icon_count = count;
}

} // namespace

void SpeciesPacks::Load() {
  if (is_tried) return;
  is_tried = true;
  LoadIconMap();
  sys::File file;
  file.Open(u"sdmc:/sango-studio/species/table.bin", sys::File::kRead);
  if (!file.IsOpen()) return;
  u8 header[kHeaderSize];
  if (file.Read(header, kHeaderSize) != static_cast<s32>(kHeaderSize)) return;
  u32 signature;
  u32 version;
  u16 species;
  u16 forms;
  std::memcpy(&signature, header, 4);
  std::memcpy(&version, header + 4, 4);
  std::memcpy(&species, header + 8, 2);
  std::memcpy(&forms, header + 10, 2);
  if (signature != kSignature || version != kVersion) {
    core::Problems::Add(u"species/table.bin: wrong signature or version");
    return;
  }
  if (species > kMaxSpecies || forms > kMaxForms) {
    core::Problems::Add(u"species/table.bin: too many species or forms");
    return;
  }
  const u32 species_bytes = species * sizeof(PackSpecies);
  const u32 form_bytes = forms * sizeof(PackForm);
  if (species_bytes != 0 &&
      file.Read(species_list, species_bytes) !=
          static_cast<s32>(species_bytes)) {
    core::Problems::Add(u"species/table.bin: the species are cut");
    return;
  }
  if (form_bytes != 0 &&
      file.Read(form_list, form_bytes) != static_cast<s32>(form_bytes)) {
    core::Problems::Add(u"species/table.bin: the forms are cut");
    return;
  }
  species_count = species;
  form_count = forms;
}

void SpeciesPacks::InstallIconHook() {
  icon_file_hook.Install(address::kGetIconFile, GetIconFileHook);
}

u32 SpeciesPacks::IconFile(u16 species, u16 form) {
  Load();
  for (u32 i = 0; i < icon_count; i++) {
    if (icon_list[i].species == species && icon_list[i].form == form) {
      return icon_list[i].file;
    }
  }
  return 0xFFFFFFFF;
}

u32 SpeciesPacks::SpeciesCount() {
  return species_count;
}

const PackSpecies* SpeciesPacks::FindSpecies(u16 species) {
  if (species < kFirstSpecies) return nullptr;
  const u32 index = static_cast<u32>(species - kFirstSpecies);
  if (index >= species_count) return nullptr;
  return &species_list[index];
}

const PackSpecies& SpeciesPacks::GetSpecies(u32 index) {
  return species_list[index < species_count ? index : 0];
}

bool SpeciesPacks::IsEmpty(const PackSpecies& species) {
  return species.name[0] == 0;
}

u32 SpeciesPacks::FormCount() {
  return form_count;
}

const PackForm& SpeciesPacks::GetForm(u32 index) {
  return form_list[index < form_count ? index : 0];
}

} // namespace pokemon
