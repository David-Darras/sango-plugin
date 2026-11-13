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
 * battle/patch/studio_abilities.h.
 */

#include "battle/patch/studio_abilities.h"

#include <cstring>

#include "battle/constant/field_effect_kind.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/situation_key.h"
#include "battle/constant/stat_stage_effect_kind.h"
#include "battle/constant/status_condition.h"
#include "battle/constant/terrain_kind.h"
#include "battle/constant/weather.h"
#include "battle/native/controller.h"
#include "battle/native/listener.h"
#include "battle/native/mutation.h"
#include "battle/native/pokemon.h"
#include "battle/native/reaction_table.h"
#include "battle/native/situation.h"
#include "battle/patch/game_extension.h"
#include "core/problems.h"
#include "system/constant/language.h"
#include "system/native/core.h"
#include "system/native/file.h"

namespace battle {
namespace {

// The kinds of condition and of action in the files.
enum ConditionKind : u8 {
  kPokemonIsOwner = 1,
  kMoveUserIsOwner = 2,
  kMoveTypeIs = 3,
  kWeatherIs = 4,
  kTargetFainted = 5,
  kOnlyOnce = 6,
};

enum ActionKind : u8 {
  kChangeStat = 1,
  kSetWeather = 2,
  kSetTerrain = 3,
  kGiveStatus = 4,
  kHeal = 5,
  kChangeType = 6,
  kUseMove = 7,
};

// The targets of an action in the files.
enum Target : u8 {
  kTargetOwner = 0,
  kTargetPokemonOfMoment = 1,
  kTargetMoveUser = 2,
  kTargetFirstTarget = 3,
};

constexpr u32 kLanguageCount = 8;
constexpr u32 kHeaderSize = 8;
constexpr u32 kConditionSize = 3;
constexpr u32 kActionSize = 7;
constexpr u8 kVersion = 1;
/// The biggest file that the feature reads. A bigger file is refused.
constexpr u32 kMaxFileSize = 8 * 1024;

struct Condition {
  s16 arg;
  u8 kind;
};

struct Action {
  s16 a;
  s16 b;
  u8 kind;
  u8 target;
  u8 banner;
};

struct Rule {
  u16 moment;
  u8 condition_count;
  u8 action_count;
  Condition conditions[StudioAbilities::kMaxConditions];
  Action actions[StudioAbilities::kMaxActions];
};

/// Everything that the feature keeps for one ability.
struct Entry {
  bool used;
  u8 id;
  u8 rule_count;
  c16 name[StudioAbilities::kNameSize];
  c16 description[StudioAbilities::kDescriptionSize];
  Rule rules[StudioAbilities::kMaxRules];
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

/// Returns the position of the language of the game in the files.
u32 LanguageIndex(sys::Language language) {
  switch (language) {
  case sys::Language::kJapanese: return 0;
  case sys::Language::kFrench: return 3;
  case sys::Language::kItalian: return 4;
  case sys::Language::kGerman: return 5;
  case sys::Language::kSpanish: return 6;
  case sys::Language::kKorean: return 7;
  default: return 2; // English.
  }
}

u16 Read16(const u8* at) { return static_cast<u16>(at[0] | (at[1] << 8)); }

/// Reads the texts of the files with a position. Keeps the text of one
/// language: the other texts are skipped.
class Reader {
public:
  Reader(const u8* bytes, u32 size) : bytes_(bytes), size_(size) {}

  bool Skip(u32 count) {
    if (count > size_ - at_) return false;
    at_ += count;
    return true;
  }

  const u8* Take(u32 count) {
    if (count > size_ - at_) return nullptr;
    const u8* out = bytes_ + at_;
    at_ += count;
    return out;
  }

  /// Reads a text: copies at most `capacity - 1` units when `out` is not null.
  bool Text(c16* out, u32 capacity) {
    const u8* length_bytes = Take(2);
    if (length_bytes == nullptr) return false;
    const u32 length = Read16(length_bytes);
    const u8* units = Take(length * 2);
    if (units == nullptr) return false;
    if (out != nullptr) {
      u32 copied = 0;
      while (copied < length && copied + 1 < capacity) {
        out[copied] = static_cast<c16>(Read16(units + copied * 2));
        copied++;
      }
      out[copied] = 0;
    }
    return true;
  }

private:
  const u8* bytes_;
  u32 size_;
  u32 at_ = 0;
};

/// Reads the texts of a file into an entry. The rules are not read.
bool ReadTexts(Entry* entry, Reader* reader) {
  const u32 wanted = LanguageIndex(sys::Core::GetInstance().GetLanguage());
  for (u32 language = 0; language < kLanguageCount; language++) {
    const bool keep = language == wanted;
    if (!reader->Text(keep ? entry->name : nullptr, StudioAbilities::kNameSize))
      return false;
    if (!reader->Text(keep ? entry->description : nullptr,
                      StudioAbilities::kDescriptionSize))
      return false;
  }
  return true;
}

bool IsKnownCondition(u8 kind) {
  return kind >= kPokemonIsOwner && kind <= kOnlyOnce;
}

bool IsKnownAction(u8 kind) { return kind >= kChangeStat && kind <= kUseMove; }

/// Reads the rules of a file into an entry. Gives false for a wrong file.
bool ReadRules(Reader* reader, u32 count, Entry* entry) {
  for (u32 i = 0; i < count; i++) {
    const u8* head = reader->Take(4);
    if (head == nullptr) return false;
    Rule& rule = entry->rules[i];
    rule.moment = Read16(head);
    rule.condition_count = head[2];
    rule.action_count = head[3];
    if (rule.condition_count > StudioAbilities::kMaxConditions ||
        rule.action_count > StudioAbilities::kMaxActions) {
      return false;
    }
    for (u32 c = 0; c < rule.condition_count; c++) {
      const u8* raw = reader->Take(kConditionSize);
      if (raw == nullptr || !IsKnownCondition(raw[0])) return false;
      rule.conditions[c].kind = raw[0];
      rule.conditions[c].arg = static_cast<s16>(Read16(raw + 1));
    }
    for (u32 a = 0; a < rule.action_count; a++) {
      const u8* raw = reader->Take(kActionSize);
      if (raw == nullptr || !IsKnownAction(raw[0])) return false;
      rule.actions[a].kind = raw[0];
      rule.actions[a].target = raw[1];
      rule.actions[a].a = static_cast<s16>(Read16(raw + 2));
      rule.actions[a].b = static_cast<s16>(Read16(raw + 4));
      rule.actions[a].banner = raw[6] & 1;
    }
  }
  return true;
}

// ---------------------------------------------------------------------------
// The rules in battle.

/// Returns the battle id that a target of an action means.
UID ResolveTarget(u8 target, UID owner) {
  switch (target) {
  case kTargetPokemonOfMoment:
    return UID{static_cast<u8>(Situation::Get(SituationKey::kPokemonId))};
  case kTargetMoveUser:
    return UID{static_cast<u8>(Situation::Get(SituationKey::kMoveUserId))};
  case kTargetFirstTarget:
    return UID{static_cast<u8>(Situation::Get(SituationKey::kTargetId1))};
  default:
    return owner;
  }
}

/// Returns true when a condition is true. OnlyOnce is handled by the caller.
bool Test(const Condition& condition, Controller* controller, UID owner) {
  switch (condition.kind) {
  case kPokemonIsOwner:
    return Situation::Get(SituationKey::kPokemonId) == owner.value;
  case kMoveUserIsOwner:
    return Situation::Get(SituationKey::kMoveUserId) == owner.value;
  case kMoveTypeIs:
    return Situation::Get(SituationKey::kMoveType) == condition.arg;
  case kWeatherIs:
    return Situation::Get(SituationKey::kWeather) == condition.arg;
  case kTargetFainted: {
    const u32 count = Situation::Get(SituationKey::kTargetCount);
    for (u32 i = 0; i < count && i < 6; i++) {
      const u32 key = static_cast<u32>(SituationKey::kTargetId1) + i;
      UID id{static_cast<u8>(Situation::Get(static_cast<SituationKey>(key)))};
      Pokemon* target = controller->GetPokemon(id);
      if (target != nullptr && target->hp == 0) return true;
    }
    return false;
  }
  default:
    return true;
  }
}

/// Makes a mutation with the banner flag of an action.
template <typename T>
T* Make(Controller* controller, MutationKind kind, UID owner,
        const Action& action) {
  auto* mutation = static_cast<T*>(controller->Create(kind, owner));
  mutation->show_ability_banner = action.banner;
  return mutation;
}

void Run(const Action& action, Controller* controller, UID owner) {
  const UID target = ResolveTarget(action.target, owner);
  switch (action.kind) {
  case kChangeStat: {
    auto* mutation = Make<AdjustStatStageMutation>(
        controller, MutationKind::kAdjustStatStage, owner, action);
    mutation->target_count = 1;
    mutation->target_ids[0] = target;
    mutation->stage_kind = static_cast<StatStageEffectKind>(action.a);
    mutation->stage_delta = static_cast<s8>(action.b);
    controller->Apply(mutation);
    break;
  }
  case kSetWeather:
    controller->SetWeather(owner, static_cast<Weather>(action.a),
                           ItemId::kNone, action.b != 0);
    break;
  case kSetTerrain: {
    auto* mutation = Make<AddFieldEffectMutation>(
        controller, MutationKind::kAddFieldEffect, owner, action);
    mutation->effect = FieldEffectKind::kTerrain;
    mutation->terrain = static_cast<TerrainKind>(action.a);
    mutation->duration.raw = 1;
    mutation->message.id = MutationMessageId::kNone;
    controller->Apply(mutation);
    break;
  }
  case kGiveStatus: {
    auto* mutation = Make<InflictStatusMutation>(
        controller, MutationKind::kInflictStatus, owner, action);
    mutation->status = static_cast<StatusCondition>(action.a);
    mutation->status_data.raw = 1;
    mutation->show_failure_message = 0;
    mutation->suppress_default_message = 0;
    mutation->suppress_item_reaction = 0;
    mutation->target_id = target;
    mutation->overwrite_mode = StatusOverwriteMode::kNone;
    controller->Apply(mutation);
    break;
  }
  case kHeal: {
    Pokemon* pokemon = controller->GetPokemon(target);
    if (pokemon == nullptr || pokemon->hp == 0) break;
    u32 amount = static_cast<u32>(pokemon->max_hp) * action.a / 100;
    if (amount == 0) amount = 1;
    auto* mutation = Make<RecoverHpMutation>(
        controller, MutationKind::kRecoverHp, owner, action);
    mutation->heal_amount = static_cast<u16>(amount);
    mutation->target_id = target;
    mutation->ignore_heal_block = 0;
    mutation->message.id = MutationMessageId::kNone;
    controller->Apply(mutation);
    break;
  }
  case kChangeType: {
    auto* mutation = Make<ChangeTypeMutation>(
        controller, MutationKind::kChangeType, owner, action);
    mutation->next_type = static_cast<TypeId>(action.a);
    mutation->target_id = target;
    mutation->suppress_default_message = 0;
    mutation->show_failure_message_if_unchanged = 0;
    controller->Apply(mutation);
    break;
  }
  case kUseMove:
    controller->ExecuteMove(controller->GetPokemon(owner),
                            static_cast<MoveId>(action.a));
    break;
  default:
    break;
  }
}

/// The reaction of the rule number kRule of an ability of the studio. The
/// listener keeps the id of the ability.
template <u32 kRule>
void RunRule(Listener* self, Controller* controller, UID owner,
             s32* local_state) {
  const Entry* entry = FindEntry(self->source_id);
  if (entry == nullptr || kRule >= entry->rule_count) return;
  const Rule& rule = entry->rules[kRule];

  bool only_once = false;
  for (u32 i = 0; i < rule.condition_count; i++) {
    const Condition& condition = rule.conditions[i];
    if (condition.kind == kOnlyOnce) {
      only_once = true;
      // The listener keeps one bit for each rule in its first number.
      if ((local_state[0] & (1 << kRule)) != 0) return;
      continue;
    }
    if (!Test(condition, controller, owner)) return;
  }
  if (only_once) local_state[0] |= (1 << kRule);

  for (u32 i = 0; i < rule.action_count; i++) {
    Run(rule.actions[i], controller, owner);
  }
}

const Reaction kReactions[StudioAbilities::kMaxRules] = {
    RunRule<0>, RunRule<1>, RunRule<2>, RunRule<3>,
    RunRule<4>, RunRule<5>, RunRule<6>, RunRule<7>,
};

// ---------------------------------------------------------------------------
// The files.

/// Writes "sdmc:/sango-studio/ability/<id>.bin".
void FullPath(u32 id, c16* out) {
  static const c16 kPrefix[] = u"sdmc:/sango-studio/ability/";
  u32 at = 0;
  for (const c16* c = kPrefix; *c != 0; c++) out[at++] = *c;
  c16 digits[4];
  u32 count = 0;
  do {
    digits[count++] = static_cast<c16>(u'0' + id % 10);
    id /= 10;
  } while (id != 0);
  while (count > 0) out[at++] = digits[--count];
  static const c16 kSuffix[] = u".bin";
  for (const c16* c = kSuffix; *c != 0; c++) out[at++] = *c;
  out[at] = 0;
}

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

/// Reads the file of an ability. A missing file removes its reactions.
void Load(u32 id) {
  c16 path[96];
  FullPath(id, path);
  const u32 size = sys::File::ReadAll(path, file_buffer, kMaxFileSize);
  Entry* entry = FindEntry(id);
  if (size == 0) {
    if (entry != nullptr && entry->rule_count != 0) {
      entry->rule_count = 0;
      Publish(entry);
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
  Reader reader(file_buffer + kHeaderSize, size - kHeaderSize);
  if (!ReadTexts(&fresh, &reader) ||
      !ReadRules(&reader, fresh.rule_count, &fresh)) {
    core::Problems::Add(u"ability %u: the file is cut or has a wrong value",
                        id);
    return;
  }
  for (u32 i = 0; i < fresh.rule_count; i++) {
    fresh.table[i].moment = static_cast<MomentKind>(fresh.rules[i].moment);
    fresh.table[i].reaction = kReactions[i];
  }
  *entry = fresh;
  Publish(entry);
}

} // namespace

void StudioAbilities::LoadAll() {
  loaded_language = sys::Core::GetInstance().GetLanguage();
  for (u32 id = kFirstId; id <= kLastId; id++) Load(id);
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
  if (std::strcmp(at, ".bin") != 0 || id < kFirstId || id > kLastId) {
    core::Problems::Add(u"studio: an ability file has a wrong name");
    return;
  }
  Load(id);
}

void StudioAbilities::RefreshLanguage() {
  const sys::Language language = sys::Core::GetInstance().GetLanguage();
  if (language == loaded_language) return;
  loaded_language = language;
  for (const Entry& entry : entries) {
    if (entry.used) Load(entry.id);
  }
}

} // namespace battle
