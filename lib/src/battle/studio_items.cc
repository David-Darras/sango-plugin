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
 * @file studio_items.cc
 * @brief The new items and the changed items of Sango Studio.
 *
 * The declarations and the layout of the files are in
 * battle/patch/studio_items.h.
 */

#include "battle/patch/studio_items.h"

#include <cstring>

#include "battle/native/controller.h"
#include "battle/native/listener.h"
#include "battle/native/reaction_table.h"
#include "battle/patch/item_extension.h"
#include "battle/patch/studio_rules.h"
#include "core/patch/archive.h"
#include "core/problems.h"
#include "pokemon/native/ability_slot.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/item_data.h"
#include "pokemon/native/species_data.h"
#include "pokemon/native/utils.h"
#include "system/constant/language.h"
#include "system/native/core.h"
#include "system/native/file.h"

namespace battle {
namespace {

constexpr u32 kHeaderSize = 8;
constexpr u32 kDataSize = 36;
constexpr u32 kBagActionSize = 5;
constexpr u8 kVersion = 1;
constexpr u32 kMaxFileSize = 8 * 1024;
constexpr s16 kAllHp = 32767;

// The object has the 36 bytes of the file, then the id of the item (4 bytes).
static_assert(sizeof(pokemon::ItemData) == kDataSize + 4,
              "ItemData must have the size of an item file of the studio");

// The kinds of the actions of the Bag.
enum BagKind : u8 {
  kChangeHp = 1,
  kSetShiny = 2,
  kNextAbility = 3,
  kSetLevel = 4,
  kAddEv = 5,
};

struct BagAction {
  u8 kind;
  s16 a;
  s16 b;
};

/// Everything that the feature keeps for one new item.
struct Entry {
  bool used;
  u16 id;
  u8 rule_count;
  bool can_use_in_bag;
  bool changes_hp;
  u16 icon_item;
  u8 bag_action_count;
  c16 name[StudioItems::kNameSize];
  c16 description[StudioItems::kDescriptionSize];
  c16 message[StudioItems::kMessageSize];
  c16 icon_path[72];
  u8 data[kDataSize];
  BagAction bag_actions[StudioItems::kMaxBagActions];
  studio::Rule rules[studio::kMaxRules];
  ReactionTable table[studio::kMaxRules];
};

/// The data that the studio gives to an item of the game.
struct Change {
  bool used;
  u16 id;
  u8 data[kDataSize];
};

Entry entries[StudioItems::kMaxItems];
Change changes[StudioItems::kMaxChanges];
u8 file_buffer[kMaxFileSize];
sys::Language loaded_language = sys::Language::kNone;

Entry* FindEntry(u32 id) {
  for (Entry& entry : entries) {
    if (entry.used && entry.id == id) return &entry;
  }
  return nullptr;
}

/// The reaction of the rule number kRule of a new item that a Pokémon holds.
/// The listener keeps the id of the item.
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

// ---------------------------------------------------------------------------
// The Bag.

/// The slot of the ability that the Pokémon goes to next: the slot after the
/// current one that has a different ability.
u32 FindNextAbilitySlot(const pokemon::CoreData& pokemon, bool* found) {
  const pokemon::SpeciesData& data =
      pokemon::SpeciesData::GetInstance(pokemon.species, pokemon.form);
  u32 current = pokemon::GetAbilitySlot(pokemon);
  for (u32 slot = 0; slot < 3; slot++) {
    if (data.ability[slot] == pokemon.ability) {
      current = slot;
      break;
    }
  }
  for (u32 step = 1; step < 3; step++) {
    const u32 slot = (current + step) % 3;
    if (data.ability[slot] != AbilityId::kNone &&
        data.ability[slot] != pokemon.ability) {
      *found = true;
      return slot;
    }
  }
  *found = false;
  return current;
}

/// The EV of a stat: 0 HP, 1 Attack, 2 Defense, 3 Speed, 4 Sp. Atk, 5 Sp. Def.
u8* EvOf(pokemon::CoreData& pokemon, s16 stat) {
  switch (stat) {
  case 0: return &pokemon.ev_hp;
  case 1: return &pokemon.ev_attack;
  case 2: return &pokemon.ev_defense;
  case 3: return &pokemon.ev_speed;
  case 4: return &pokemon.ev_special_attack;
  case 5: return &pokemon.ev_special_defense;
  default: return nullptr;
  }
}

bool CanUseSlot(const Entry& entry, const pokemon::CoreData& pokemon) {
  for (u32 i = 0; i < entry.bag_action_count; i++) {
    if (entry.bag_actions[i].kind == kNextAbility) {
      bool found = false;
      FindNextAbilitySlot(pokemon, &found);
      if (!found) return false;
    }
  }
  return true;
}

const c16* UseSlot(const Entry& entry, pokemon::CoreData& pokemon) {
  for (u32 i = 0; i < entry.bag_action_count; i++) {
    const BagAction& action = entry.bag_actions[i];
    switch (action.kind) {
    case kChangeHp:
      ItemExtension::ChangeHp(action.a == kAllHp ? 0xFFFF : action.a);
      break;
    case kSetShiny: {
      const bool is_shiny = pokemon::Utils::IsShiny(pokemon.id, pokemon.shiny_id);
      pokemon.SetShiny(action.a == 0 ? !is_shiny : action.a == 1);
      break;
    }
    case kNextAbility: {
      bool found = false;
      const u32 slot = FindNextAbilitySlot(pokemon, &found);
      if (found) pokemon::SetAbilitySlot(pokemon, slot);
      break;
    }
    case kSetLevel:
      if (action.a >= 1 && action.a <= 100) {
        pokemon.SetLevel(static_cast<u8>(action.a));
      }
      break;
    case kAddEv: {
      u8* ev = EvOf(pokemon, action.a);
      if (ev == nullptr || action.b <= 0) break;
      u32 total = pokemon.ev_hp + pokemon.ev_attack + pokemon.ev_defense +
                  pokemon.ev_speed + pokemon.ev_special_attack +
                  pokemon.ev_special_defense;
      u32 add = static_cast<u32>(action.b);
      if (*ev + add > 252) add = 252 - *ev;
      if (total + add > 510) add = total > 510 ? 0 : 510 - total;
      *ev = static_cast<u8>(*ev + add);
      break;
    }
    default:
      break;
    }
  }
  return entry.message[0] != 0 ? entry.message : nullptr;
}

// The game calls these functions without the id of the item: each slot has its
// own functions.
template <u32 kSlot>
bool CanUse(const pokemon::CoreData& pokemon) {
  return entries[kSlot].used && CanUseSlot(entries[kSlot], pokemon);
}

template <u32 kSlot>
const c16* Use(pokemon::CoreData& pokemon) {
  return entries[kSlot].used ? UseSlot(entries[kSlot], pokemon) : nullptr;
}

template <u32 kSlot>
void PatchData(pokemon::ItemData& item) {
  if (!entries[kSlot].used) return;
  std::memcpy(static_cast<void*>(&item), entries[kSlot].data, kDataSize);
}

#define SANGO_ROW(F, N) F<N>
#define SANGO_TABLE(F)                                                        \
  {                                                                           \
    SANGO_ROW(F, 0),  SANGO_ROW(F, 1),  SANGO_ROW(F, 2),  SANGO_ROW(F, 3),   \
    SANGO_ROW(F, 4),  SANGO_ROW(F, 5),  SANGO_ROW(F, 6),  SANGO_ROW(F, 7),   \
    SANGO_ROW(F, 8),  SANGO_ROW(F, 9),  SANGO_ROW(F, 10), SANGO_ROW(F, 11),  \
    SANGO_ROW(F, 12), SANGO_ROW(F, 13), SANGO_ROW(F, 14), SANGO_ROW(F, 15),  \
    SANGO_ROW(F, 16), SANGO_ROW(F, 17), SANGO_ROW(F, 18), SANGO_ROW(F, 19),  \
    SANGO_ROW(F, 20), SANGO_ROW(F, 21), SANGO_ROW(F, 22), SANGO_ROW(F, 23),  \
    SANGO_ROW(F, 24), SANGO_ROW(F, 25), SANGO_ROW(F, 26), SANGO_ROW(F, 27),  \
    SANGO_ROW(F, 28), SANGO_ROW(F, 29), SANGO_ROW(F, 30), SANGO_ROW(F, 31),  \
  }

typedef void (*PatchFunction)(pokemon::ItemData&);
typedef bool (*CanUseFunction)(const pokemon::CoreData&);
typedef const c16* (*UseFunction)(pokemon::CoreData&);

const PatchFunction kPatches[StudioItems::kMaxItems] = SANGO_TABLE(PatchData);
const CanUseFunction kCanUse[StudioItems::kMaxItems] = SANGO_TABLE(CanUse);
const UseFunction kUse[StudioItems::kMaxItems] = SANGO_TABLE(Use);
#undef SANGO_TABLE
#undef SANGO_ROW

// ---------------------------------------------------------------------------
// The files.

void Publish(u32 slot) {
  Entry* entry = &entries[slot];
  ItemSpec spec;
  std::memset(&spec, 0, sizeof(spec));
  spec.id = static_cast<ItemId>(entry->id);
  spec.name = entry->name;
  spec.description = entry->description;
  spec.icon_item = static_cast<ItemId>(entry->icon_item);
  spec.patch_data = kPatches[slot];
  spec.reactions = entry->table;
  spec.reaction_count = entry->rule_count;
  if (entry->can_use_in_bag && entry->bag_action_count != 0) {
    spec.can_use = kCanUse[slot];
    spec.use = kUse[slot];
  }
  spec.changes_hp = entry->changes_hp;
  spec.icon_path = entry->icon_path;
  if (ItemExtension::FindItem(spec.id) != nullptr) {
    ItemExtension::UpdateItem(spec);
  } else {
    ItemExtension::AddItem(spec);
  }
}

/// Reads the file of a new item. A missing file removes its reactions.
void LoadCustom(u32 id) {
  static const c16 kFolder[] = u"sdmc:/sango-studio/item/custom/";
  static const c16 kSuffix[] = u".bin";
  static const c16 kIconSuffix[] = u".bclim";
  c16 path[96];
  studio::MakePath(kFolder, id, kSuffix, path);
  const u32 size = sys::File::ReadAll(path, file_buffer, kMaxFileSize);

  u32 slot = StudioItems::kMaxItems;
  for (u32 i = 0; i < StudioItems::kMaxItems; i++) {
    if (entries[i].used && entries[i].id == id) slot = i;
  }
  if (size == 0) {
    if (slot < StudioItems::kMaxItems && entries[slot].rule_count != 0) {
      entries[slot].rule_count = 0;
      entries[slot].bag_action_count = 0;
      Publish(slot);
    }
    return;
  }

  if (size < kHeaderSize + kDataSize + 4 ||
      std::memcmp(file_buffer, "SITM", 4) != 0 ||
      file_buffer[4] != kVersion || studio::Read16(file_buffer + 5) != id ||
      file_buffer[7] > studio::kMaxRules) {
    core::Problems::Add(u"item %u: the file is not valid", id);
    return;
  }
  if (slot == StudioItems::kMaxItems) {
    for (u32 i = 0; i < StudioItems::kMaxItems; i++) {
      if (!entries[i].used) {
        slot = i;
        break;
      }
    }
    if (slot == StudioItems::kMaxItems) {
      core::Problems::Add(u"item %u: more than %u items of the studio", id,
                          StudioItems::kMaxItems);
      return;
    }
  }

  // Read into a copy: a wrong file must not break an item that works.
  static Entry fresh;
  fresh = Entry();
  fresh.used = true;
  fresh.id = static_cast<u16>(id);
  fresh.rule_count = file_buffer[7];
  studio::Reader reader(file_buffer + kHeaderSize, size - kHeaderSize);
  c16* texts[3] = {fresh.name, fresh.description, fresh.message};
  const u32 capacities[3] = {StudioItems::kNameSize,
                             StudioItems::kDescriptionSize,
                             StudioItems::kMessageSize};
  const u8* data = nullptr;
  const u8* tail = nullptr;
  if (!studio::ReadTexts(&reader, 3, texts, capacities) ||
      (data = reader.Take(kDataSize)) == nullptr ||
      (tail = reader.Take(4)) == nullptr) {
    core::Problems::Add(u"item %u: the file is cut", id);
    return;
  }
  std::memcpy(fresh.data, data, kDataSize);
  fresh.can_use_in_bag = (tail[0] & 1) != 0;
  fresh.icon_item = studio::Read16(tail + 1);
  fresh.bag_action_count = tail[3];
  if (fresh.bag_action_count > StudioItems::kMaxBagActions) {
    core::Problems::Add(u"item %u: too many actions of the Bag", id);
    return;
  }
  for (u32 i = 0; i < fresh.bag_action_count; i++) {
    const u8* raw = reader.Take(kBagActionSize);
    if (raw == nullptr || raw[0] < kChangeHp || raw[0] > kAddEv) {
      core::Problems::Add(u"item %u: wrong action of the Bag", id);
      return;
    }
    fresh.bag_actions[i].kind = raw[0];
    fresh.bag_actions[i].a = static_cast<s16>(studio::Read16(raw + 1));
    fresh.bag_actions[i].b = static_cast<s16>(studio::Read16(raw + 3));
    if (raw[0] == kChangeHp) fresh.changes_hp = true;
  }
  if (!studio::ReadRules(&reader, fresh.rule_count, fresh.rules)) {
    core::Problems::Add(u"item %u: the rules have a wrong value", id);
    return;
  }
  for (u32 i = 0; i < fresh.rule_count; i++) {
    fresh.table[i].moment = static_cast<MomentKind>(fresh.rules[i].moment);
    fresh.table[i].reaction = kReactions[i];
  }
  studio::MakePath(kFolder, id, kIconSuffix, fresh.icon_path);
  entries[slot] = fresh;
  Publish(slot);
}

/// Reads the data of an item of the game ("item/data/<id>.bin").
void LoadChange(u32 id) {
  static const c16 kFolder[] = u"sdmc:/sango-studio/item/data/";
  static const c16 kSuffix[] = u".bin";
  c16 path[96];
  studio::MakePath(kFolder, id, kSuffix, path);
  u8 bytes[kDataSize + 4];
  const u32 size = sys::File::ReadAll(path, bytes, sizeof(bytes));
  if (size == 0) return;
  if (size != kDataSize) {
    core::Problems::Add(u"item %u: the file has %u bytes", id, size);
    return;
  }
  Change* change = nullptr;
  for (Change& slot : changes) {
    if (slot.used && slot.id == id) change = &slot;
  }
  if (change == nullptr) {
    for (Change& slot : changes) {
      if (!slot.used) {
        change = &slot;
        break;
      }
    }
    if (change == nullptr) {
      core::Problems::Add(u"item %u: too many changed items", id);
      return;
    }
  }
  change->used = true;
  change->id = static_cast<u16>(id);
  std::memcpy(change->data, bytes, kDataSize);
  core::Archive::RegisterItemData(id, change->data);
}

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

void StudioItems::LoadAll() {
  loaded_language = sys::Core::GetInstance().GetLanguage();
  for (u32 id = kFirstId; id <= kLastId; id++) LoadCustom(id);
}

void StudioItems::Apply(const c8* path) {
  static const c8 kCustom[] = "item/custom/";
  static const c8 kData[] = "item/data/";
  u32 id = 0;
  if (std::memcmp(path, kCustom, sizeof(kCustom) - 1) == 0) {
    // The icons ("<id>.bclim") are read by the game when it needs them.
    if (!ReadId(path + sizeof(kCustom) - 1, &id)) return;
    if (id < kFirstId || id > kLastId) {
      core::Problems::Add(u"studio: an item file has a wrong name");
      return;
    }
    LoadCustom(id);
  } else if (std::memcmp(path, kData, sizeof(kData) - 1) == 0) {
    if (!ReadId(path + sizeof(kData) - 1, &id) || id == 0 ||
        id >= ItemExtension::kGameItemCount) {
      return;
    }
    LoadChange(id);
  }
}

void StudioItems::RefreshLanguage() {
  const sys::Language language = sys::Core::GetInstance().GetLanguage();
  if (language == loaded_language) return;
  loaded_language = language;
  for (u32 i = 0; i < kMaxItems; i++) {
    if (entries[i].used) LoadCustom(entries[i].id);
  }
}

} // namespace battle
