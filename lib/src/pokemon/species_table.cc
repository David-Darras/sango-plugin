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
 * @file species_table.cc
 * @brief Adds the species of generations VII, VIII and IX to the game.
 *
 * The declarations are in pokemon/patch/species_table.h.
 */

#include "pokemon/patch/species_table.h"
#include "pokemon/patch/alolan_forms.h"
#include "pokemon/patch/species_editor.h"
#include <cstring>
#include "pokemon/constant/ability.h"
#include "pokemon/native/database.h"
#include "pokemon/native/poke_info.h"
#include "pokemon/native/evolution_data.h"
#include "pokemon/native/movepool.h"
#include "pokemon/native/species_data.h"
#include "core/hook.h"
#include "core/problems.h"
#include "core/memory.h"
#include "core/native/process_manager.h"
#include "system/native/core.h"
#include "system/native/file.h"
#include "overworld/patch/whos_that_pokemon.h"
#include "pokemon/address.h"

namespace pokemon {

namespace {
core::Hook<void(String*, u16)> get_species_name_hook;
core::Hook<void(c16*, u16)> get_default_nickname_hook;
core::Hook<void(u16, u8)> load_movepool_hook;
} // namespace

static constexpr u16 kGen9Carrier = 721;
static constexpr u32 kSecondCarrierBase = 256;
static constexpr u32 kNativeModel = 0x8000;
static constexpr u32 kGen7Base = SpeciesTable::kSpeciesCount + 3;
static constexpr u16 kGen7Carrier = 494;

// The names of the species of the game that the user changed in Sango Studio:
// the file of the texts of the language (the file 98 of its archive) that the
// studio copies in sdmc:/sango-studio/archive/<archive>/98.bin. The game reads
// its list of names once, before the plugin can change what it reads, so the
// names are replaced here, when the game asks for one.
namespace {
constexpr u32 kNameCharacters = 24;
constexpr u32 kNameCacheSize = 64;
bool text_file_tried = false;
bool text_file_there = false;
sys::Language text_file_language = sys::Language::kNone;
c16 text_file_path[64];

// A name that was read: the species, and the name (empty when the studio has
// no name for it).
struct CachedName {
  u16 species;
  c16 name[kNameCharacters + 1];
};
CachedName name_cache[kNameCacheSize];

// The archive of the texts of a language (71 Japanese, 72 Japanese with
// kanji, then English, French, Italian, German, Spanish and Korean).
u32 TextArchive(sys::Language language) {
  switch (language) {
  case sys::Language::kJapanese: return 71;
  case sys::Language::kEnglish: return 73;
  case sys::Language::kFrench: return 74;
  case sys::Language::kItalian: return 75;
  case sys::Language::kGerman: return 76;
  case sys::Language::kSpanish: return 77;
  case sys::Language::kKorean: return 78;
  default: return 0;
  }
}

// Looks for the file of the language that the game reads. The cache of the
// names is emptied when the language changes.
void FindNameFile() {
  const sys::Language language = sys::Core::GetInstance().GetLanguage();
  if (text_file_tried && language == text_file_language) return;
  text_file_tried = true;
  text_file_language = language;
  text_file_there = false;
  std::memset(name_cache, 0, sizeof(name_cache));
  const u32 archive = TextArchive(language);
  if (archive == 0) return;
  const c16 prefix[] = u"sdmc:/sango-studio/archive/";
  u32 at = 0;
  for (u32 i = 0; prefix[i] != 0; i++) text_file_path[at++] = prefix[i];
  text_file_path[at++] = u'0' + (archive / 10);
  text_file_path[at++] = u'0' + (archive % 10);
  const c16 suffix[] = u"/98.bin";
  for (u32 i = 0; suffix[i] != 0; i++) text_file_path[at++] = suffix[i];
  text_file_path[at] = 0;
  sys::File file;
  file.Open(text_file_path, sys::File::kRead);
  text_file_there = file.IsOpen();
}

// Reads some bytes of the file of the studio, at a place of the file.
bool ReadNameFile(u32 place, void* buffer, u32 size) {
  sys::File file;
  file.Open(text_file_path, sys::File::kRead);
  if (!file.IsOpen()) return false;
  const s32 read = file.Read(buffer, size, place);
  return read == static_cast<s32>(size);
}

// Reads the name of a species in the file of the studio: the lines are
// hidden with a key (see the file format in Sango Studio).
bool ReadStudioName(u16 species, c16* out) {
  u8 header[4];
  if (!ReadNameFile(0, header, 4)) return false;
  u16 lines;
  std::memcpy(&lines, header + 2, 2);
  if (species == 0 || species >= lines) return false;
  u8 entry[8];
  if (!ReadNameFile(20 + 8 * static_cast<u32>(species), entry, 8)) return false;
  u32 start;
  u16 length;
  std::memcpy(&start, entry, 4);
  std::memcpy(&length, entry + 4, 2);
  if (length > kNameCharacters + 8) length = kNameCharacters + 8;
  u8 raw[2 * (kNameCharacters + 8)];
  if (!ReadNameFile(16 + start, raw, 2 * static_cast<u32>(length))) return false;
  u16 key = static_cast<u16>(0x7C89 + 0x2983 * species);
  u32 count = 0;
  for (u32 i = 0; i < length; i++) {
    u16 character;
    std::memcpy(&character, raw + 2 * i, 2);
    character ^= key;
    key = static_cast<u16>((key << 3) | (key >> 13));
    if (character == 0 || count >= kNameCharacters) break;
    out[count++] = character;
  }
  out[count] = 0;
  return count != 0;
}

// Writes the name of a species of the game from the file of the studio.
// Returns false when the studio has no name for it.
bool StudioGameName(u16 species, c16* out, u32 capacity) {
  FindNameFile();
  if (!text_file_there || capacity < kNameCharacters + 1) return false;
  CachedName& slot = name_cache[species % kNameCacheSize];
  if (slot.species != species) {
    slot.species = species;
    slot.name[0] = 0;
    ReadStudioName(species, slot.name);
  }
  if (slot.name[0] == 0) return false;
  u32 i = 0;
  while (slot.name[i] != 0) {
    out[i] = slot.name[i];
    i++;
  }
  out[i] = 0;
  return true;
}
} // namespace

// Returns the data of a species of a pack, or null. The ids follow the order
// of the file: the index is the id minus the first id.
static const PackSpecies* FindExtra(u16 species) {
  SpeciesPacks::Load();
  return SpeciesPacks::FindSpecies(species);
}

bool SpeciesTable::IsAddedSpecies(u16 species) {
  return FindExtra(species) != nullptr || IsNewSpecies(species);
}

bool SpeciesTable::IsNewSpecies(u16 species) {
  return FindNewSpecies(species) != nullptr;
}

const SpeciesTable::NewSpeciesEntry* SpeciesTable::FindNewSpecies(u16 species) {
  auto& feat = GetInstance();
  if (species < kFirstNewSpecies) return nullptr;
  const u32 index = species - kFirstNewSpecies;
  if (index >= feat.new_species_.Count()) return nullptr;
  return &feat.new_species_[index];
}

SpeciesId SpeciesTable::AddSpecies(const c16* name, SpeciesId data_from,
                                   SpeciesId model, FormId model_form) {
#ifdef GAME_ORAS
  auto& feat = GetInstance();
  if (feat.is_built_) {
    core::Problems::Add(u"NewSpecies: call it in SetUpHack()");
    return SpeciesId::kNone;
  }
  if (feat.new_species_.IsFull()) {
    core::Problems::Add(u"NewSpecies: more than %u new species",
                        kMaxNewSpecies);
    return SpeciesId::kNone;
  }
  if (name == nullptr || data_from == SpeciesId::kNone ||
      static_cast<u16>(data_from) >= kFirstNewSpecies) {
    core::Problems::Add(u"NewSpecies: the name or the first data is not correct");
    return SpeciesId::kNone;
  }
  const NewSpeciesEntry entry = {
      name, data_from, model != SpeciesId::kNone ? model : data_from,
      model_form};
  feat.new_species_.Add(entry);
  return static_cast<SpeciesId>(kFirstNewSpecies + feat.new_species_.Count() -
                                1);
#else
  (void)name;
  (void)data_from;
  (void)model;
  (void)model_form;
  core::Problems::Add(u"NewSpecies: Alpha Sapphire only");
  return SpeciesId::kNone;
#endif
}

void SpeciesTable::PatchModelRequest(PokeInfo* info) {
  if (info == nullptr) return;
  const u16 species = static_cast<u16>(info->species);
  const NewSpeciesEntry* added = FindNewSpecies(species);
  if (added != nullptr) {
    info->species = added->model;
    info->form = added->model_form;
    return;
  }
  const PackSpecies* extra = FindExtra(species);
  if (extra != nullptr) {
    const u32 form_model = info->form != FormId::kNormal
        ? AlolanForms::GetModelForm(species, static_cast<u32>(info->form)) : 0;
    const u32 model = form_model != 0 ? form_model : extra->model_form;
    if ((model & kNativeModel) != 0) {
      info->species = static_cast<SpeciesId>(model & ~kNativeModel);
      info->form = FormId::kNormal;
    } else if (model >= kSecondCarrierBase) {
      info->species = static_cast<SpeciesId>(kGen9Carrier);
      info->form = static_cast<FormId>(model - kSecondCarrierBase);
    } else {
      info->species = static_cast<SpeciesId>(kGen7Carrier);
      info->form = static_cast<FormId>(model);
    }
    return;
  }
}

static void FixAbilities(SpeciesData* entry, const SpeciesData* fallback) {
  for (u32 k = 0; k < 3; k++) {
    if (static_cast<u8>(entry->ability[k]) < static_cast<u8>(AbilityId::kCount)) {
      continue;
    }
    entry->ability[k] =
        static_cast<u8>(entry->ability[0]) < static_cast<u8>(AbilityId::kCount)
          ? entry->ability[0]
          : fallback->ability[0];
  }
}

void SpeciesTable::BuildTable(u8* game_table) {
  auto& feat = GetInstance();
  u8* out = feat.table_;

  SpeciesPacks::Load();
  const u32 species_entries = kGen7Base;
  const u32 form_entries = kEntries - species_entries;
  // The places of the species of the product stay empty when they are not used.
  std::memset(out, 0, sizeof(feat.table_));
  std::memcpy(out, game_table, species_entries * kEntrySize);
  std::memcpy(out + (species_entries + SpeciesTable::kAddedCount) * kEntrySize,
              game_table + species_entries * kEntrySize,
              form_entries * kEntrySize);

  for (u32 i = 0; i < species_entries; i++) {
    auto* entry = (SpeciesData*)(out + i * kEntrySize);
    if (entry->form_index != 0) entry->form_index += SpeciesTable::kAddedCount;
    if (entry->form_model_index != 0) {
      entry->form_model_index += SpeciesTable::kAddedCount;
    }
  }

  for (u32 i = 0; i < SpeciesPacks::SpeciesCount(); i++) {
    auto* entry = (SpeciesData*)(out + (kGen7Base + i) * kEntrySize);
    std::memcpy(entry, SpeciesPacks::GetSpecies(i).data, kEntrySize);
    FixAbilities(entry, (SpeciesData*)out);
    entry->form_index = 0;
    entry->form_model_index = 0;
    entry->total_form_count = 1;
  }

  // The species of the product start with the data of an other species.
  for (u32 i = 0; i < feat.new_species_.Count(); i++) {
    const NewSpeciesEntry& added = feat.new_species_[i];
    auto* entry =
        (SpeciesData*)(out + (SpeciesTable::kFirstNewSpecies + i) * kEntrySize);
    std::memcpy(entry, out + static_cast<u16>(added.data_from) * kEntrySize,
                kEntrySize);
    entry->form_index = 0;
    entry->form_model_index = 0;
    entry->total_form_count = 1;
  }

  u32 next = kEntries + SpeciesTable::kAddedCount;
  for (u32 i = 0; i < SpeciesPacks::FormCount(); i++) {
    const PackForm& form = SpeciesPacks::GetForm(i);
    if (form.species == 0 || form.species >= kEntries + kAddedCount) continue;
    auto* base = (SpeciesData*)(out + form.species * kEntrySize);
    const u32 existing = base->total_form_count > 1 ? base->total_form_count - 1u : 0u;
    if (next + existing + 1 > kTotalEntries) {
      core::Problems::Add(u"species table: no room for the forms of %u",
                          form.species);
      break;
    }
    const u32 first = next;
    for (u32 k = 0; k < existing; k++) {
      std::memcpy(out + next * kEntrySize,
                  out + (base->form_index + k) * kEntrySize, kEntrySize);
      next++;
    }
    auto* entry = (SpeciesData*)(out + next * kEntrySize);
    std::memcpy(entry, form.data, kEntrySize);
    FixAbilities(entry, base);
    entry->form_index = 0;
    entry->form_model_index = 0;
    entry->total_form_count = 1;
    base->form_index = (u16)first;
    base->form_model_index = (u16)first;
    base->total_form_count = (u8)(existing + 2);
    next++;
  }
}

void SpeciesTable::Update() {
#ifdef GAME_ORAS
  auto& feat = GetInstance();
  if (feat.is_built_) return;
  // The game loads its data of Pokémon again when the player chooses the
  // language of a new game, and it frees the table that it had: the table of
  // the plugin must not be there yet. Wait for the overworld or a battle.
  if (!core::ProcessManager::IsOverworldActive() &&
      !core::ProcessManager::IsBattleActive()) {
    return;
  }
  auto& db = Database::GetInstance();
  u8* game_table = (u8*)db.species;
  if (game_table == nullptr) return;
  feat.is_built_ = true;
  BuildTable(game_table);
  db.species = (SpeciesData*)feat.table_;
  // The new species and forms exist now: apply the changes of the product.
  SpeciesChanges::ApplyPending();
#endif
}

// Writes the first calls of the name hook in sdmc:/sango/name_trace.log: the
// species, the language, the size of the file of the studio and whether a name
// was replaced. It shows if the game uses this function for the names it shows.
static void TraceName(u16 species, bool replaced) {
  static sys::File file;
  static bool opened = false;
  static u32 lines = 0;
  if (lines >= 40) return;
  if (!opened) {
    opened = true;
    sys::File::CreateDirectory(u"sdmc:/sango");
    file.Open(u"sdmc:/sango/name_trace.log");
  }
  if (!file.IsOpen()) return;
  lines++;
  file.WriteText(u"species %u language %u file %u replaced %u\n", species,
                 static_cast<u32>(text_file_language),
                 text_file_there ? 1 : 0,
                 replaced ? 1 : 0);
}

void SpeciesTable::GetSpeciesNameHook(String* output, u16 species) {
  if (output != nullptr && overworld::WhosThatPokemon::AreNamesHidden()) {
    output->Set(u"???");
    return;
  }
  const PackSpecies* extra = FindExtra(species);
  if (extra != nullptr) {
    if (output != nullptr) output->Set(extra->name);
    return;
  }
  const NewSpeciesEntry* added = FindNewSpecies(species);
  if (added != nullptr) {
    if (output != nullptr) output->Set(added->name);
    return;
  }
  get_species_name_hook(output, species);
  c16 changed[kNameCharacters + 1];
  const bool replaced = output != nullptr && species <= kSpeciesCount &&
                        StudioGameName(species, changed, kNameCharacters + 1);
  if (replaced) output->Set(changed);
  TraceName(species, replaced);
}

void SpeciesTable::GetDefaultNicknameHook(c16* output, u16 species) {
  // The game reads the name in its list of 721 names: for a species of a pack
  // or of the product, the nickname is the name of the species.
  const c16* name = nullptr;
  const PackSpecies* extra = FindExtra(species);
  if (extra != nullptr) {
    name = extra->name;
  } else {
    const NewSpeciesEntry* added = FindNewSpecies(species);
    if (added != nullptr) name = added->name;
  }
  c16 changed[kNameCharacters + 1];
  if (name == nullptr && output != nullptr && species <= kSpeciesCount &&
      StudioGameName(species, changed, kNameCharacters + 1)) {
    name = changed;
  }
  if (name == nullptr || output == nullptr) {
    get_default_nickname_hook(output, species);
    return;
  }
  u32 length = 0;
  while (name[length] != 0 && length < 12) {
    output[length] = name[length];
    length++;
  }
  output[length] = 0;
}

void SpeciesTable::LoadMovepoolHook(u16 species, u8 form) {
  const PackSpecies* extra = FindExtra(species);
  if (extra != nullptr) {
    load_movepool_hook(1, 0);
    auto& pool = Movepool::Object();
    pool.species = static_cast<SpeciesId>(species);
    pool.form = static_cast<FormId>(form);
    u32 count = 0;
    for (u32 k = 0; k < PackSpecies::kMoveMax; k++) {
      const u16 move = extra->moves[k].move;
      if (move == 0) break;
      if (move >= static_cast<u16>(MoveId::kCount)) continue;
      pool.entry[count].move = static_cast<MoveId>(move);
      pool.entry[count].level = extra->moves[k].level;
      pool.entry[count].padding = 0;
      count++;
    }
    pool.count = count;
    SpeciesChanges::PatchLearnset(static_cast<SpeciesId>(species),
                                  static_cast<FormId>(form));
    return;
  }
  const NewSpeciesEntry* added = FindNewSpecies(species);
  if (added != nullptr) {
    // The learnset of the first species, until the product changes it.
    LoadMovepoolHook(static_cast<u16>(added->data_from), 0);
    auto& pool = Movepool::Object();
    pool.species = static_cast<SpeciesId>(species);
    pool.form = static_cast<FormId>(form);
    SpeciesChanges::PatchLearnset(static_cast<SpeciesId>(species),
                                  static_cast<FormId>(form));
    return;
  }
  load_movepool_hook(species, form);
  SpeciesChanges::PatchLearnset(static_cast<SpeciesId>(species),
                                static_cast<FormId>(form));
}

bool SpeciesTable::PatchEvolutionTable(u16 species, EvolutionData* table) {
  const PackSpecies* extra = FindExtra(species);
  if (extra != nullptr) {
    std::memset(table, 0, sizeof(*table));
    for (u32 k = 0; k < PackSpecies::kEvolutionMax; k++) {
      const PackEvolution& evolution = extra->evolutions[k];
      table->data[k].method = static_cast<EvolutionMethod>(evolution.method);
      table->data[k].arg = evolution.arg;
      table->data[k].species = static_cast<SpeciesId>(evolution.species);
    }
    return true;
  }
  if (IsNewSpecies(species)) {
    // No evolution, until the product gives the evolutions.
    std::memset(table, 0, sizeof(*table));
    return true;
  }
  return false;
}

void SpeciesTable::Initialize() {
  for (u32 i = 0; i < SIZE(address::kSpeciesBound); i++) {
    if (address::kSpeciesBound[i] == 0) continue;
    WRITE32(address::kSpeciesBound[i], kTotalEntries - 1);
  }
  get_species_name_hook.Install(address::kGetSpeciesName, GetSpeciesNameHook);
  get_default_nickname_hook.Install(address::kGetDefaultNickname,
                                    GetDefaultNicknameHook);
  load_movepool_hook.Install(address::kLoadMovepool, LoadMovepoolHook);
}

} // namespace pokemon
