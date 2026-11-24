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
 * @file studio_abilities.cc
 * @brief The new abilities that Sango Studio makes.
 *
 * The declarations and the layout of the files are in
 * battle/patch/studio_abilities.h. The rules are in battle/patch/studio_rules.h.
 */

#include "battle/patch/studio_abilities.h"

#include <cstring>

#include "battle/native/controller.h"
#include "battle/native/listener.h"
#include "battle/native/reaction_table.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/studio_rules.h"
#include "core/problems.h"
#include "system/constant/language.h"
#include "system/native/core.h"
#include "system/native/file.h"

namespace battle {
namespace {

constexpr u32 kHeaderSize = 8;
constexpr u8 kVersion = 1;
/// The biggest file that the feature reads. A bigger file is refused.
constexpr u32 kMaxFileSize = 8 * 1024;

/// Everything that the feature keeps for one ability.
struct Entry {
  bool used;
  u8 id;
  u8 rule_count;
  c16 name[StudioAbilities::kNameSize];
  c16 description[StudioAbilities::kDescriptionSize];
  studio::Rule rules[StudioAbilities::kMaxRules];
  ReactionTable table[StudioAbilities::kMaxRules];
};

Entry entries[StudioAbilities::kMaxAbilities];
u8 file_buffer[kMaxFileSize];
sys::Language loaded_language = sys::Language::kNone;

/// Returns the entry of an ability, or null.
Entry* FindEntry(u32 id) {
  for (Entry& entry : entries) {
    if (entry.used && entry.id == id) return &entry;
  }
  return nullptr;
}

/// The reaction of the rule number kRule of an ability of the studio. The
/// listener keeps the id of the ability.
template <u32 kRule>
void RunRule(Listener* self, Controller* controller, UID owner,
             s32* local_state) {
  const Entry* entry = FindEntry(self->source_id);
  if (entry == nullptr || kRule >= entry->rule_count) return;
  studio::Execute(entry->rules, kRule, controller, owner, local_state);
}

const Reaction kReactions[StudioAbilities::kMaxRules] = {
    RunRule<0>, RunRule<1>, RunRule<2>, RunRule<3>,
    RunRule<4>, RunRule<5>, RunRule<6>, RunRule<7>,
};

/// Puts the entry in the list of the abilities of the game: the first time it
/// registers the ability, then it updates it.
void Publish(Entry* entry) {
  const AbilitySpec spec = {static_cast<AbilityId>(entry->id), entry->name,
                            entry->description, entry->table,
                            entry->rule_count};
  if (GameExtension::FindAbility(spec.id) != nullptr) {
    GameExtension::UpdateAbility(spec);
  } else {
    GameExtension::AddAbility(spec);
  }
}

/// Puts the reactions of an ability of the game in place of its code. Without
/// reactions, the game runs its code again.
void PublishGame(Entry* entry) {
  const AbilitySpec spec = {static_cast<AbilityId>(entry->id), entry->name,
                            entry->description, entry->table,
                            entry->rule_count};
  GameExtension::ReplaceGameAbility(spec);
}

/// True for an ability of the game.
bool IsGameId(u32 id) {
  return id >= StudioAbilities::kFirstGameId &&
         id <= StudioAbilities::kLastGameId;
}

/// True for an id that can have a file.
bool IsFileId(u32 id) {
  return IsGameId(id) ||
         (id >= StudioAbilities::kFirstId && id <= StudioAbilities::kLastId);
}

/// Reads the file of an ability. A missing file removes its reactions: an
/// ability of the game runs its code again.
void Load(u32 id) {
  static const c16 kFolder[] = u"sdmc:/sango-studio/ability/";
  static const c16 kSuffix[] = u".bin";
  c16 path[96];
  studio::MakePath(kFolder, id, kSuffix, path);
  const u32 size = sys::File::ReadAll(path, file_buffer, kMaxFileSize);
  Entry* entry = FindEntry(id);
  if (size == 0) {
    if (entry != nullptr && entry->rule_count != 0) {
      entry->rule_count = 0;
      if (IsGameId(id)) {
        PublishGame(entry);
      } else {
        Publish(entry);
      }
    }
    return;
  }

  if (size < kHeaderSize || std::memcmp(file_buffer, "SABR", 4) != 0 ||
      file_buffer[4] != kVersion || file_buffer[5] != id ||
      file_buffer[6] > StudioAbilities::kMaxRules) {
    core::Problems::Add(u"ability %u: the file is not valid", id);
    return;
  }
  if (entry == nullptr) {
    for (Entry& slot : entries) {
      if (!slot.used) {
        entry = &slot;
        break;
      }
    }
    if (entry == nullptr) {
      core::Problems::Add(u"ability %u: more than %u abilities of the studio",
                          id, StudioAbilities::kMaxAbilities);
      return;
    }
  }

  // Read into a copy: a wrong file must not break an ability that works.
  static Entry fresh;
  fresh = Entry();
  fresh.used = true;
  fresh.id = static_cast<u8>(id);
  fresh.rule_count = file_buffer[6];
  studio::Reader reader(file_buffer + kHeaderSize, size - kHeaderSize);
  c16* texts[2] = {fresh.name, fresh.description};
  const u32 capacities[2] = {StudioAbilities::kNameSize,
                             StudioAbilities::kDescriptionSize};
  if (!studio::ReadTexts(&reader, 2, texts, capacities) ||
      !studio::ReadRules(&reader, fresh.rule_count, fresh.rules)) {
    core::Problems::Add(u"ability %u: the file is cut or has a wrong value",
                        id);
    return;
  }
  for (u32 i = 0; i < fresh.rule_count; i++) {
    fresh.table[i].moment = static_cast<MomentKind>(fresh.rules[i].moment);
    fresh.table[i].reaction = kReactions[i];
  }
  *entry = fresh;
  if (IsGameId(id)) {
    PublishGame(entry);
  } else {
    Publish(entry);
  }
}

/// Reads `ability/game.bin`: the abilities of the game that have a file. The
/// layout: the text "SAGL", the version 1, 3 bytes 0, then one bit for each
/// ability (bit `id % 8` of the byte `id / 8`).
void LoadGameAbilities() {
  static const c16 kListPath[] = u"sdmc:/sango-studio/ability/game.bin";
  constexpr u32 kListHeader = 8;
  constexpr u32 kListSize = kListHeader + 32;
  u8 list[kListSize] = {};
  const u32 size = sys::File::ReadAll(kListPath, list, kListSize);
  if (size < kListSize || std::memcmp(list, "SAGL", 4) != 0 ||
      list[4] != kVersion) {
    return;
  }
  for (u32 id = StudioAbilities::kFirstGameId;
       id <= StudioAbilities::kLastGameId; id++) {
    if ((list[kListHeader + id / 8] >> (id % 8)) & 1) Load(id);
  }
}

} // namespace

void StudioAbilities::LoadAll() {
  loaded_language = sys::Core::GetInstance().GetLanguage();
  for (u32 id = kFirstId; id <= kLastId; id++) Load(id);
  LoadGameAbilities();
}

void StudioAbilities::Apply(const c8* path) {
  // The path is "ability/<id>.bin".
  const c8* at = path + 8;
  u32 id = 0;
  if (*at < '0' || *at > '9') return;
  while (*at >= '0' && *at <= '9') {
    id = id * 10 + static_cast<u32>(*at - '0');
    if (id > 1000) return;
    at++;
  }
  if (std::strcmp(at, ".bin") != 0 || !IsFileId(id)) {
    core::Problems::Add(u"studio: an ability file has a wrong name");
    return;
  }
  Load(id);
}

void StudioAbilities::RefreshLanguage() {
  const sys::Language language = sys::Core::GetInstance().GetLanguage();
  if (language == loaded_language) return;
  loaded_language = language;
  for (u32 i = 0; i < kMaxAbilities; i++) {
    if (entries[i].used) Load(entries[i].id);
  }
}

} // namespace battle
