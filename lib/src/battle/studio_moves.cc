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
 * @file studio_moves.cc
 * @brief The new moves and the changed moves of Sango Studio.
 *
 * The declarations and the layout of the files are in
 * battle/patch/studio_moves.h.
 */

#include "battle/patch/studio_moves.h"

#include <cstring>

#include "battle/native/controller.h"
#include "battle/native/listener.h"
#include "battle/native/reaction_table.h"
#include "battle/patch/game_extension.h"
#include "battle/patch/studio_rules.h"
#include "core/problems.h"
#include "pokemon/native/move_data.h"
#include "system/constant/language.h"
#include "system/native/core.h"
#include "system/native/file.h"

namespace battle {
namespace {

constexpr u32 kHeaderSize = 8;
constexpr u32 kDataSize = 36;
constexpr u8 kVersion = 1;
constexpr u32 kMaxFileSize = 8 * 1024;

static_assert(sizeof(pokemon::MoveData) == kDataSize,
              "MoveData must have the size of a move file of the studio");

/// Everything that the feature keeps for one new move.
struct Entry {
  bool used;
  u16 id;
  u8 rule_count;
  c16 name[StudioMoves::kNameSize];
  c16 description[StudioMoves::kDescriptionSize];
  u8 data[kDataSize];
  studio::Rule rules[studio::kMaxRules];
  ReactionTable table[studio::kMaxRules];
};

/// The data that the studio gives to a move of the game.
struct Change {
  bool used;
  u16 id;
  u8 data[kDataSize];
};

Entry entries[StudioMoves::kMaxMoves];
Change changes[StudioMoves::kMaxChanges];
u8 file_buffer[kMaxFileSize];
sys::Language loaded_language = sys::Language::kNone;
bool is_listening = false;

Entry* FindEntry(u32 id) {
  for (Entry& entry : entries) {
    if (entry.used && entry.id == id) return &entry;
  }
  return nullptr;
}

/// The reaction of the rule number kRule of a new move. The listener keeps the
/// id of the move.
template <u32 kRule>
void RunRule(Listener* self, Controller* controller, UID owner,
             s32* local_state) {
  const Entry* entry = FindEntry(self->source_id);
  if (entry == nullptr || kRule >= entry->rule_count) return;
  studio::Execute(entry->rules[kRule], kRule, controller, owner, local_state);
}

const Reaction kReactions[studio::kMaxRules] = {
    RunRule<0>, RunRule<1>, RunRule<2>, RunRule<3>,
    RunRule<4>, RunRule<5>, RunRule<6>, RunRule<7>,
};

/// Gives the data of the studio to a new move. The function of a slot knows
/// its entry: the game calls it without the id.
template <u32 kSlot>
void PatchData(pokemon::MoveData& move) {
  if (!entries[kSlot].used) return;
  std::memcpy(&move, entries[kSlot].data, kDataSize);
}

typedef void (*PatchFunction)(pokemon::MoveData&);

#define SANGO_PATCH_ROW(N) PatchData<N>
const PatchFunction kPatches[StudioMoves::kMaxMoves] = {
    SANGO_PATCH_ROW(0),  SANGO_PATCH_ROW(1),  SANGO_PATCH_ROW(2),
    SANGO_PATCH_ROW(3),  SANGO_PATCH_ROW(4),  SANGO_PATCH_ROW(5),
    SANGO_PATCH_ROW(6),  SANGO_PATCH_ROW(7),  SANGO_PATCH_ROW(8),
    SANGO_PATCH_ROW(9),  SANGO_PATCH_ROW(10), SANGO_PATCH_ROW(11),
    SANGO_PATCH_ROW(12), SANGO_PATCH_ROW(13), SANGO_PATCH_ROW(14),
    SANGO_PATCH_ROW(15), SANGO_PATCH_ROW(16), SANGO_PATCH_ROW(17),
    SANGO_PATCH_ROW(18), SANGO_PATCH_ROW(19), SANGO_PATCH_ROW(20),
    SANGO_PATCH_ROW(21), SANGO_PATCH_ROW(22), SANGO_PATCH_ROW(23),
    SANGO_PATCH_ROW(24), SANGO_PATCH_ROW(25), SANGO_PATCH_ROW(26),
    SANGO_PATCH_ROW(27), SANGO_PATCH_ROW(28), SANGO_PATCH_ROW(29),
    SANGO_PATCH_ROW(30), SANGO_PATCH_ROW(31),
};
#undef SANGO_PATCH_ROW

/// The game loads the data of a move: a move of the game that the studio
/// changed gets its data.
void OnMoveData(MoveId move, pokemon::MoveData* data) {
  const u32 id = static_cast<u32>(move);
  if (id >= StudioMoves::kFirstId) return;
  for (const Change& change : changes) {
    if (change.used && change.id == id) {
      std::memcpy(data, change.data, kDataSize);
      return;
    }
  }
}

void Listen() {
  if (is_listening) return;
  is_listening = true;
  GameExtension::GetInstance().on_move_data.Subscribe(OnMoveData);
}

void Publish(u32 slot) {
  Entry* entry = &entries[slot];
  const MoveSpec spec = {static_cast<MoveId>(entry->id), entry->name,
                         entry->description, kPatches[slot], nullptr,
                         entry->table, entry->rule_count};
  if (GameExtension::FindMove(spec.id) != nullptr) {
    GameExtension::UpdateMove(spec);
  } else {
    GameExtension::AddMove(spec);
  }
}

/// Reads the file of a new move. A missing file removes its reactions.
void LoadCustom(u32 id) {
  static const c16 kFolder[] = u"sdmc:/sango-studio/move/custom/";
  static const c16 kSuffix[] = u".bin";
  c16 path[96];
  studio::MakePath(kFolder, id, kSuffix, path);
  const u32 size = sys::File::ReadAll(path, file_buffer, kMaxFileSize);

  u32 slot = StudioMoves::kMaxMoves;
  for (u32 i = 0; i < StudioMoves::kMaxMoves; i++) {
    if (entries[i].used && entries[i].id == id) slot = i;
  }
  if (size == 0) {
    if (slot < StudioMoves::kMaxMoves && entries[slot].rule_count != 0) {
      entries[slot].rule_count = 0;
      Publish(slot);
    }
    return;
  }

  if (size < kHeaderSize + kDataSize ||
      std::memcmp(file_buffer, "SMVR", 4) != 0 ||
      file_buffer[4] != kVersion || studio::Read16(file_buffer + 5) != id ||
      file_buffer[7] > studio::kMaxRules) {
    core::Problems::Add(u"move %u: the file is not valid", id);
    return;
  }
  if (slot == StudioMoves::kMaxMoves) {
    for (u32 i = 0; i < StudioMoves::kMaxMoves; i++) {
      if (!entries[i].used) {
        slot = i;
        break;
      }
    }
    if (slot == StudioMoves::kMaxMoves) {
      core::Problems::Add(u"move %u: more than %u moves of the studio", id,
                          StudioMoves::kMaxMoves);
      return;
    }
  }

  // Read into a copy: a wrong file must not break a move that works.
  static Entry fresh;
  fresh = Entry();
  fresh.used = true;
  fresh.id = static_cast<u16>(id);
  fresh.rule_count = file_buffer[7];
  studio::Reader reader(file_buffer + kHeaderSize, size - kHeaderSize);
  c16* texts[2] = {fresh.name, fresh.description};
  const u32 capacities[2] = {StudioMoves::kNameSize,
                             StudioMoves::kDescriptionSize};
  const u8* data = nullptr;
  if (!studio::ReadTexts(&reader, 2, texts, capacities) ||
      (data = reader.Take(kDataSize)) == nullptr ||
      !studio::ReadRules(&reader, fresh.rule_count, fresh.rules)) {
    core::Problems::Add(u"move %u: the file is cut or has a wrong value", id);
    return;
  }
  std::memcpy(fresh.data, data, kDataSize);
  for (u32 i = 0; i < fresh.rule_count; i++) {
    fresh.table[i].moment = static_cast<MomentKind>(fresh.rules[i].moment);
    fresh.table[i].reaction = kReactions[i];
  }
  entries[slot] = fresh;
  Listen();
  Publish(slot);
}

/// Reads the data of a move of the game ("move/data/<id>.bin").
void LoadChange(u32 id) {
  static const c16 kFolder[] = u"sdmc:/sango-studio/move/data/";
  static const c16 kSuffix[] = u".bin";
  c16 path[96];
  studio::MakePath(kFolder, id, kSuffix, path);
  u8 bytes[kDataSize + 4];
  const u32 size = sys::File::ReadAll(path, bytes, sizeof(bytes));

  Change* change = nullptr;
  for (Change& slot : changes) {
    if (slot.used && slot.id == id) change = &slot;
  }
  if (size == 0) {
    // A file that is gone: the move has its own data again at the next load.
    if (change != nullptr) change->used = false;
    return;
  }
  if (size != kDataSize) {
    core::Problems::Add(u"move %u: the file has %u bytes", id, size);
    return;
  }
  if (change == nullptr) {
    for (Change& slot : changes) {
      if (!slot.used) {
        change = &slot;
        break;
      }
    }
    if (change == nullptr) {
      core::Problems::Add(u"move %u: too many changed moves", id);
      return;
    }
  }
  change->used = true;
  change->id = static_cast<u16>(id);
  std::memcpy(change->data, bytes, kDataSize);
  Listen();
}

/// Reads the number of a path: "<folder>/<id>.bin". Gives false for a wrong
/// path.
bool ReadId(const c8* at, u32* id) {
  u32 out = 0;
  if (*at < '0' || *at > '9') return false;
  while (*at >= '0' && *at <= '9') {
    out = out * 10 + static_cast<u32>(*at - '0');
    if (out > 10000) return false;
    at++;
  }
  if (std::strcmp(at, ".bin") != 0) return false;
  *id = out;
  return true;
}

} // namespace

void StudioMoves::LoadAll() {
  loaded_language = sys::Core::GetInstance().GetLanguage();
  for (u32 id = kFirstId; id <= kLastId; id++) LoadCustom(id);
}

void StudioMoves::Apply(const c8* path) {
  static const c8 kCustom[] = "move/custom/";
  static const c8 kData[] = "move/data/";
  u32 id = 0;
  if (std::memcmp(path, kCustom, sizeof(kCustom) - 1) == 0) {
    if (!ReadId(path + sizeof(kCustom) - 1, &id) || id < kFirstId ||
        id > kLastId) {
      core::Problems::Add(u"studio: a move file has a wrong name");
      return;
    }
    LoadCustom(id);
  } else if (std::memcmp(path, kData, sizeof(kData) - 1) == 0) {
    if (!ReadId(path + sizeof(kData) - 1, &id) || id == 0 || id >= kFirstId) {
      core::Problems::Add(u"studio: a move file has a wrong name");
      return;
    }
    LoadChange(id);
  }
}

void StudioMoves::RefreshLanguage() {
  const sys::Language language = sys::Core::GetInstance().GetLanguage();
  if (language == loaded_language) return;
  loaded_language = language;
  for (u32 i = 0; i < kMaxMoves; i++) {
    if (entries[i].used) LoadCustom(entries[i].id);
  }
}

} // namespace battle
