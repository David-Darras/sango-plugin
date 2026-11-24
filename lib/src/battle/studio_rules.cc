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
 * @file studio_rules.cc
 * @brief The rules of the new abilities, moves and items of Sango Studio.
 *
 * The declarations are in battle/patch/studio_rules.h.
 */

#include "battle/patch/studio_rules.h"

#include "battle/constant/field_effect_kind.h"
#include "battle/constant/mutation_kind.h"
#include "battle/constant/situation_key.h"
#include "battle/constant/stat_stage_effect_kind.h"
#include "battle/constant/status_condition.h"
#include "battle/constant/terrain_kind.h"
#include "battle/constant/weather.h"
#include "battle/constant/field_position.h"
#include "battle/native/controller.h"
#include "battle/native/mutation.h"
#include "battle/native/pokemon.h"
#include "battle/native/situation.h"
#include "pokemon/native/move_data.h"
#include "core/utils.h"
#include "system/native/core.h"

namespace battle {
namespace studio {
namespace {

// The kinds of condition and of action in the files.
enum ConditionKind : u8 {
  kPokemonIsOwner = 1,
  kMoveUserIsOwner = 2,
  kMoveTypeIs = 3,
  kWeatherIs = 4,
  kTargetFainted = 5,
  kOnlyOnce = 6,
  kHpBelow = 7,
  kDefenderIsOwner = 8,
  kMoveCategoryIs = 9,
  kSuperEffective = 10,
  kNoSubstitute = 11,
  kHpNotFull = 12,
  kMoveMakesContact = 13,
  kPokemonIsFoe = 14,
  kCausedByOther = 15,
  kOwnerHasStatus = 16,
  kOtherHasStatus = 17,
  kOtherHasAbility = 18,
  kPowerAtMost = 19,
  kMoveIsStab = 20,
  kCriticalHit = 21,
  kMoveHasFlag = 22,
  kMoveInGroup = 23,
  kChance = 24,
  kNotVeryEffective = 25,
  kHpFull = 26,
  kMoveTypeWasChanged = 27,
};
constexpr u8 kLastCondition = kMoveTypeWasChanged;

enum ActionKind : u8 {
  kChangeStat = 1,
  kSetWeather = 2,
  kSetTerrain = 3,
  kGiveStatus = 4,
  kHeal = 5,
  kChangeType = 6,
  kUseMove = 7,
  kMultiplyValue = 8,
  kDamageHp = 9,
  kConsumeItem = 10,
  kRaiseCritical = 11,
  kBlockStatDrop = 12,
  kBlockStatus = 13,
  kBlockMove = 14,
  kBlockFlinch = 15,
  kBlockCritical = 16,
  kChangeMoveType = 17,
  kRaisePriority = 18,
  kMultiplyEffectChance = 19,
  kForceCritical = 20,
};
constexpr u8 kLastAction = kForceCritical;

// The targets of an action in the files.
enum Target : u8 {
  kTargetOwner = 0,
  kTargetPokemonOfMoment = 1,
  kTargetMoveUser = 2,
  kTargetFirstTarget = 3,
  kTargetFoes = 4,
};

// The moments that some actions need (see MomentKind).
constexpr u16 kMomentMovePower = 61;
constexpr u16 kMomentMoveData = 40;
constexpr u16 kMomentFlinchChance = 117;

// The places in the memory of the listener (local_state).
constexpr u32 kStateOnce = 0; ///< One bit for each rule: "only the first time".
constexpr u32 kStateMoveType = 1; ///< 1 + the type that the ability gave.

// The value of the type effectiveness for a normal hit (x1). A smaller value
// is not very effective, 0 is no effect.
constexpr s32 kNormalEffectiveness = 7;
// A Pokémon id from 0 to 11 is on the first side, from 12 on the second side.
constexpr u8 kFirstSideIds = 12;
// The number of status conditions of a Pokémon in battle.
constexpr u32 kStatusCount = 38;
// The critical-hit stage that always gives a critical hit.
constexpr s32 kAlwaysCritical = 3;
// The groups of moves that the game keeps in its code: biting (Strong Jaw),
// aura and pulse (Mega Launcher), ball and bomb (Bulletproof), powder
// (Overcoat).
const u16 kBitingMoves[] = {44, 242, 305, 422, 423, 424, 158};
const u16 kPulseMoves[] = {399, 396, 505, 618, 352, 406};
const u16 kBallMoves[] = {140, 411, 247, 301, 311, 360, 412, 486, 121, 188, 396,
                          402, 426, 545, 296, 331, 443, 491, 190, 192, 439};
const u16 kPowderMoves[] = {78, 79, 77, 147, 600, 178};

constexpr u32 kConditionSize = 3;
constexpr u32 kActionSize = 7;

/// The numbers of the moment that a reaction can change: the power of the
/// move, the damage, the speed, the accuracy, the critical hit stage...
///
/// A reaction reads them with Situation::Get(). Situation::Set() adds a new
/// number, it does not change the one that the battle is using. So this class
/// changes the table of the numbers itself. ORAS only.
class MomentValues {
public:
  /// What a reaction may do with a number.
  static constexpr u8 kFree = 0; ///< Any new value.
  static constexpr u8 kOnce = 1; ///< One new value, one time.
  static constexpr u8 kDone = 2; ///< The one new value was written.
  static constexpr u8 kRatio = 3; ///< A multiplication only.

  /// Multiplies a number that accepts it, by `percent` / 100.
  static void Multiply(SituationKey key, s32 percent) {
    Table* table = GetTable();
    const s32 place = Find(table, key);
    if (place < 0 || table->state[place] != kRatio) return;
    // The number has 12 bits of fraction.
    const s64 ratio = (static_cast<s64>(percent) * 4096) / 100;
    s64 value = (static_cast<s64>(table->value[place]) * ratio + 0x800) >> 12;
    if (table->largest[place] != 0 || table->smallest[place] != 0) {
      if (value < table->smallest[place]) value = table->smallest[place];
      if (value > table->largest[place]) value = table->largest[place];
    }
    table->value[place] = static_cast<s32>(value);
  }

  /// Writes a number that the battle lets the reactions write.
  static bool Write(SituationKey key, s32 value) {
    Table* table = GetTable();
    const s32 place = Find(table, key);
    if (place < 0) return false;
    if (table->state[place] != kFree && table->state[place] != kOnce) {
      return false;
    }
    table->value[place] = value;
    if (table->state[place] == kOnce) table->state[place] = kDone;
    return true;
  }

  /// True when this plugin knows the table of the numbers of the game.
  static bool Available() { return kTable != 0; }

  /// Reads a number. Returns false when the moment has no such number.
  static bool Read(SituationKey key, s32* value) {
    Table* table = GetTable();
    const s32 place = Find(table, key);
    if (place < 0) return false;
    *value = table->value[place];
    return true;
  }

private:
  static constexpr u32 kSize = 96;
  struct Table {
    u32 first; ///< The first place of the numbers of the current moment.
    u16 key[kSize];
    s32 value[kSize];
    s32 largest[kSize]; ///< The largest result of a multiplication.
    s32 smallest[kSize]; ///< The smallest result of a multiplication.
    u8 state[kSize];
  };
  static_assert(sizeof(Table) == 1444, "Table must have the layout of the game");

  // The table of the battle code of ORAS. The game of X and Y has another
  // address that this plugin does not know: the functions do nothing there.
  static constexpr uptr kTable = GAME_ADDRESS(0, 0x0815A628);

  static Table* GetTable() {
    static Table empty = {};
    return kTable == 0 ? &empty : reinterpret_cast<Table*>(kTable);
  }

  /// The place of a number in the part of the table of the moment.
  static s32 Find(const Table* table, SituationKey key) {
    for (u32 p = table->first; p < kSize && table->key[p] != 0; p++) {
      if (table->key[p] == static_cast<u16>(key)) return static_cast<s32>(p);
    }
    return -1;
  }
};

/// Reads a number of the moment. A number that the moment does not have gives
/// 0: the read of the game reports an error for it (and costs time).
s32 Value(SituationKey key) {
  if (!MomentValues::Available()) return Situation::Get(key);
  s32 value = 0;
  MomentValues::Read(key, &value);
  return value;
}

/// Reads a number of the moment. Returns false when the moment has no such
/// number (on X and Y, always true: the game gives 0).
bool Has(SituationKey key, s32* value) {
  if (!MomentValues::Available()) {
    *value = Situation::Get(key);
    return true;
  }
  return MomentValues::Read(key, value);
}

/// Reads the id of a Pokémon of the moment. Returns false when the moment has
/// no such Pokémon.
bool PokemonId(SituationKey key, UID* id) {
  s32 value = 0;
  if (!Has(key, &value) || value < 0 || value >= 0xFF) return false;
  *id = UID{static_cast<u8>(value)};
  return true;
}

/// The number that a moment multiplies: the power of a move has its own,
/// all the other moments use the shared multiplier.
SituationKey MultiplierKey(u16 moment) {
  return moment == kMomentMovePower ? SituationKey::kMovePowerMultiplier
                                    : SituationKey::kMultiplier;
}

/// Returns the data of the move of the moment, or null.
const pokemon::MoveData* CurrentMove() {
  s32 id = 0;
  if (!Has(SituationKey::kMoveId, &id) || id <= 0 || id >= 1000) {
    return nullptr;
  }
  return &pokemon::MoveData::GetInstance(static_cast<MoveId>(id));
}

/// Returns the type of the move of the moment: the type of the moment (it can
/// change, like with Refrigerate), else the type of the data. -1 for none.
s32 MoveType() {
  s32 type = 0;
  if (Has(SituationKey::kMoveType, &type)) return type;
  const pokemon::MoveData* move = CurrentMove();
  return move == nullptr ? -1 : static_cast<s32>(move->type);
}

/// True when two Pokémon are on the two sides of the battle.
bool AreFoes(UID a, UID b) {
  return (a.value < kFirstSideIds) != (b.value < kFirstSideIds);
}

/// The other Pokémon of the move: the target when the owner uses the move,
/// the user when the owner is hit. Returns false when there is none.
bool OtherPokemon(UID owner, UID* other) {
  UID user{0};
  if (!PokemonId(SituationKey::kMoveUserId, &user)) return false;
  if (user.value != owner.value) {
    *other = user;
    return true;
  }
  return PokemonId(SituationKey::kMoveRecipientId, other) ||
         PokemonId(SituationKey::kTargetId1, other);
}

/// True when a Pokémon has a status condition. 0 is any of paralysis, sleep,
/// freeze, burn and poison.
bool HasStatus(const Pokemon* pokemon, s32 status) {
  if (pokemon == nullptr) return false;
  // The low 3 bits of a status condition tell how it lasts: 0 is "absent".
  if (status == 0) {
    for (u32 i = static_cast<u32>(StatusCondition::kParalysis);
         i <= static_cast<u32>(StatusCondition::kPoison); i++) {
      if ((pokemon->status_condition[i] & 7) != 0) return true;
    }
    return false;
  }
  if (status < 0 || static_cast<u32>(status) >= kStatusCount) return false;
  return (pokemon->status_condition[status] & 7) != 0;
}

/// True when a status condition of the moment is the one of an action. 0 is
/// any of paralysis, sleep, freeze, burn and poison.
bool StatusMatches(s32 status, s32 wanted) {
  if (wanted == 0) {
    return status >= static_cast<s32>(StatusCondition::kParalysis) &&
           status <= static_cast<s32>(StatusCondition::kPoison);
  }
  return status == wanted;
}

/// True when a move is in a list.
template <u32 kCount>
bool InList(const u16 (&list)[kCount], s32 move) {
  for (u16 entry : list) {
    if (entry == move) return true;
  }
  return false;
}

/// True when the move of the moment is in a group of moves of the game.
bool MoveInGroup(s32 group) {
  s32 move = 0;
  if (!Has(SituationKey::kMoveId, &move)) return false;
  switch (group) {
  case 1: return InList(kBitingMoves, move);
  case 2: return InList(kPulseMoves, move);
  case 3: return InList(kBallMoves, move);
  case 4: return InList(kPowderMoves, move);
  default: return false;
  }
}

/// Returns true when a condition is true. OnlyOnce is handled by the caller.
bool Test(const Condition& condition, Controller* controller, UID owner,
          const s32* local_state) {
  switch (condition.kind) {
  case kPokemonIsOwner:
    return Value(SituationKey::kPokemonId) == owner.value;
  case kMoveUserIsOwner:
    return Value(SituationKey::kMoveUserId) == owner.value;
  case kMoveTypeIs:
    return MoveType() == condition.arg;
  case kWeatherIs:
    return Value(SituationKey::kWeather) == condition.arg;
  case kTargetFainted: {
    const u32 count = Value(SituationKey::kTargetCount);
    for (u32 i = 0; i < count && i < 6; i++) {
      const u32 key = static_cast<u32>(SituationKey::kTargetId1) + i;
      UID id{static_cast<u8>(Value(static_cast<SituationKey>(key)))};
      Pokemon* target = controller->GetPokemon(id);
      if (target != nullptr && target->hp == 0) return true;
    }
    return false;
  }
  case kHpBelow: {
    // The HP of the owner are under a part of its maximum HP.
    Pokemon* owner_pokemon = controller->GetPokemon(owner);
    if (owner_pokemon == nullptr || owner_pokemon->max_hp == 0) return false;
    return static_cast<u32>(owner_pokemon->hp) * 100 <
           static_cast<u32>(owner_pokemon->max_hp) *
               static_cast<u32>(condition.arg < 0 ? 0 : condition.arg);
  }
  case kDefenderIsOwner:
    return Value(SituationKey::kMoveRecipientId) == owner.value;
  case kMoveCategoryIs: {
    const pokemon::MoveData* move = CurrentMove();
    return move != nullptr &&
           static_cast<s32>(move->damage_category) == condition.arg;
  }
  case kSuperEffective:
    return Value(SituationKey::kTypeEffectiveness) > kNormalEffectiveness;
  case kNoSubstitute:
    return Value(SituationKey::kSubstituteFlag) == 0;
  case kHpNotFull: {
    Pokemon* owner_pokemon = controller->GetPokemon(owner);
    return owner_pokemon != nullptr && owner_pokemon->hp < owner_pokemon->max_hp;
  }
  case kMoveMakesContact: {
    const pokemon::MoveData* move = CurrentMove();
    return move != nullptr && (move->flags & 1) != 0;
  }
  case kPokemonIsFoe: {
    UID id{0};
    return PokemonId(SituationKey::kPokemonId, &id) && AreFoes(id, owner);
  }
  case kCausedByOther: {
    UID user{0};
    return PokemonId(SituationKey::kMoveUserId, &user) &&
           user.value != owner.value;
  }
  case kOwnerHasStatus:
    return HasStatus(controller->GetPokemon(owner), condition.arg);
  case kOtherHasStatus: {
    UID other{0};
    return OtherPokemon(owner, &other) &&
           HasStatus(controller->GetPokemon(other), condition.arg);
  }
  case kOtherHasAbility: {
    UID other{0};
    if (!OtherPokemon(owner, &other)) return false;
    const Pokemon* pokemon = controller->GetPokemon(other);
    return pokemon != nullptr &&
           static_cast<s32>(pokemon->ability) == condition.arg;
  }
  case kPowerAtMost: {
    s32 power = 0;
    if (!Has(SituationKey::kMovePower, &power)) {
      const pokemon::MoveData* move = CurrentMove();
      if (move == nullptr) return false;
      power = move->power;
    }
    return power > 0 && power <= condition.arg;
  }
  case kMoveIsStab: {
    UID user{0};
    if (!PokemonId(SituationKey::kMoveUserId, &user) &&
        !PokemonId(SituationKey::kPokemonId, &user)) {
      return false;
    }
    const Pokemon* pokemon = controller->GetPokemon(user);
    const s32 type = MoveType();
    return pokemon != nullptr && type >= 0 &&
           (static_cast<s32>(pokemon->type_1) == type ||
            static_cast<s32>(pokemon->type_2) == type ||
            static_cast<s32>(pokemon->type_3) == type);
  }
  case kCriticalHit:
    return Value(SituationKey::kCriticalHitFlag) != 0;
  case kMoveHasFlag: {
    const pokemon::MoveData* move = CurrentMove();
    return move != nullptr && condition.arg >= 0 && condition.arg < 32 &&
           ((move->flags >> condition.arg) & 1) != 0;
  }
  case kMoveInGroup:
    return MoveInGroup(condition.arg);
  case kChance:
    return static_cast<s32>(core::Utils::GetRandomValue(100)) < condition.arg;
  case kNotVeryEffective: {
    const s32 effectiveness = Value(SituationKey::kTypeEffectiveness);
    return effectiveness > 0 && effectiveness < kNormalEffectiveness;
  }
  case kHpFull: {
    Pokemon* owner_pokemon = controller->GetPokemon(owner);
    return owner_pokemon != nullptr && owner_pokemon->hp == owner_pokemon->max_hp;
  }
  case kMoveTypeWasChanged:
    return local_state[kStateMoveType] != 0 &&
           MoveType() == local_state[kStateMoveType] - 1;
  default:
    return true;
  }
}

/// Writes the Pokémon that a target of an action means. Returns their number.
u32 ResolveTargets(u8 target, Controller* controller, UID owner,
                   UID (&out)[Controller::kPositionCount]) {
  switch (target) {
  case kTargetPokemonOfMoment:
    return PokemonId(SituationKey::kPokemonId, &out[0]) ? 1 : 0;
  case kTargetMoveUser:
    return PokemonId(SituationKey::kMoveUserId, &out[0]) ? 1 : 0;
  case kTargetFirstTarget:
    return PokemonId(SituationKey::kTargetId1, &out[0]) ? 1 : 0;
  case kTargetFoes: {
    u8 ids[Controller::kPositionCount] = {};
    const u32 count =
        controller->ListFoesNextTo(controller->GetFrontPosition(owner), ids);
    u32 kept = 0;
    for (u32 i = 0; i < count && i < Controller::kPositionCount; i++) {
      out[kept++] = UID{ids[i]};
    }
    return kept;
  }
  default:
    out[0] = owner;
    return 1;
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

/// Shows or hides the banner of the ability of the owner.
void Banner(Controller* controller, UID owner, bool show) {
  controller->Apply(controller->Create(
      show ? MutationKind::kShowAbilityBanner : MutationKind::kHideAbilityBanner,
      owner));
}

/// The item that makes a weather of an ability last 8 turns, as in the game.
ItemId WeatherRock(s32 weather) {
  switch (static_cast<Weather>(weather)) {
  case Weather::kHarshSunlight: return ItemId::kHeatRock;
  case Weather::kRain: return ItemId::kDampRock;
  case Weather::kHail: return ItemId::kIcyRock;
  case Weather::kSandstorm: return ItemId::kSmoothRock;
  default: return ItemId::kNone;
  }
}

/// Runs the action on one target.
void RunOn(const Action& action, Controller* controller, UID owner,
           UID target) {
  switch (action.kind) {
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
  case kDamageHp: {
    Pokemon* pokemon = controller->GetPokemon(target);
    if (pokemon == nullptr || pokemon->hp == 0) break;
    u32 amount = static_cast<u32>(pokemon->max_hp) *
                 static_cast<u32>(action.a < 0 ? 0 : action.a) / 100;
    if (amount == 0) amount = 1;
    if (amount > pokemon->hp) amount = pokemon->hp;
    auto* mutation = Make<DealDamageMutation>(
        controller, MutationKind::kDealDamage, owner, action);
    mutation->damage_amount = static_cast<u16>(amount);
    mutation->target_id = target;
    mutation->ignore_if_semi_invulnerable = 0;
    mutation->play_visual_effect = 0;
    mutation->visual_effect_id = 0;
    mutation->effect_start_position = FieldPosition::kNone;
    mutation->effect_end_position = FieldPosition::kNone;
    mutation->message.id = MutationMessageId::kNone;
    controller->Apply(mutation);
    break;
  }
  default:
    break;
  }
}

/// Runs an action. Returns false when the next actions of the rule must not
/// run (a move that "Block the move" could not block).
bool Run(const Action& action, u16 moment, Controller* controller, UID owner,
         s32* local_state, bool more_actions) {
  switch (action.kind) {
  case kChangeStat: {
    UID targets[Controller::kPositionCount];
    const u32 count = ResolveTargets(action.target, controller, owner, targets);
    if (count == 0) break;
    auto* mutation = Make<AdjustStatStageMutation>(
        controller, MutationKind::kAdjustStatStage, owner, action);
    mutation->target_count = static_cast<u8>(count);
    for (u32 i = 0; i < count; i++) mutation->target_ids[i] = targets[i];
    mutation->stage_kind = static_cast<StatStageEffectKind>(action.a);
    mutation->stage_delta = static_cast<s8>(action.b);
    // Intimidate of the game shows "won't go any lower" for each foe.
    if (action.target == kTargetFoes) mutation->show_failure_message = 1;
    controller->Apply(mutation);
    break;
  }
  case kGiveStatus:
  case kHeal:
  case kChangeType:
  case kDamageHp: {
    UID targets[Controller::kPositionCount];
    const u32 count = ResolveTargets(action.target, controller, owner, targets);
    for (u32 i = 0; i < count; i++) RunOn(action, controller, owner, targets[i]);
    break;
  }
  case kSetWeather:
    controller->SetWeather(owner, static_cast<Weather>(action.a),
                           WeatherRock(action.a), action.b != 0);
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
  case kUseMove:
    controller->ExecuteMove(controller->GetPokemon(owner),
                            static_cast<MoveId>(action.a));
    break;
  case kMultiplyValue:
    // b is the multiplier in percent (150 is x1.5).
    if (action.b > 0) MomentValues::Multiply(MultiplierKey(moment), action.b);
    break;
  case kConsumeItem: {
    auto* mutation = Make<ConsumeItemMutation>(
        controller, MutationKind::kConsumeItem, owner, action);
    mutation->skip_action = 0;
    mutation->skip_berry_eaten_flag = 0;
    mutation->message.id = MutationMessageId::kNone;
    controller->Apply(mutation);
    break;
  }
  case kRaiseCritical: {
    // a is the number of stages to add to the critical hit stage.
    s32 stage = 0;
    if (MomentValues::Read(SituationKey::kCriticalHitStage, &stage)) {
      MomentValues::Write(SituationKey::kCriticalHitStage, stage + action.a);
    }
    break;
  }
  case kBlockStatDrop: {
    // The moment has the change (a negative amount is a drop) and the stat.
    s32 amount = 0;
    s32 stat = 0;
    if (!Has(SituationKey::kAmount, &amount) || amount >= 0) break;
    if (action.a != 0 &&
        (!Has(SituationKey::kConditionCategory, &stat) || stat != action.a)) {
      break;
    }
    MomentValues::Write(SituationKey::kFailedFlag, 1);
    break;
  }
  case kBlockStatus: {
    s32 status = 0;
    if (Has(SituationKey::kStatusId, &status) &&
        StatusMatches(status, action.a)) {
      MomentValues::Write(SituationKey::kFailedFlag, 1);
    }
    break;
  }
  case kBlockMove:
    if (!MomentValues::Write(SituationKey::kNoEffectFlag, 1)) return false;
    // The next actions show their own result: the game skips its message
    // "It doesn't affect...".
    if (more_actions) MomentValues::Write(SituationKey::kCheckResultFlag, 1);
    if (action.banner) {
      Banner(controller, owner, true);
      Banner(controller, owner, false);
    }
    break;
  case kBlockFlinch:
  case kBlockCritical:
    MomentValues::Write(SituationKey::kFailedFlag, 1);
    break;
  case kChangeMoveType:
    if (MomentValues::Write(SituationKey::kMoveType, action.a)) {
      local_state[kStateMoveType] = action.a + 1;
    }
    break;
  case kRaisePriority: {
    s32 priority = 0;
    if (MomentValues::Read(SituationKey::kMovePriority, &priority)) {
      MomentValues::Write(SituationKey::kMovePriority, priority + action.a);
    }
    break;
  }
  case kMultiplyEffectChance: {
    if (action.b <= 0) break;
    // The chance of a flinch has one doubling flag only (like Serene Grace).
    if (moment == kMomentFlinchChance) {
      if (action.b >= 200) {
        MomentValues::Write(SituationKey::kCriticalHitFlag, 1);
      }
      break;
    }
    s32 chance = 0;
    if (MomentValues::Read(SituationKey::kEffectChance, &chance)) {
      s32 next = chance * action.b / 100;
      if (next > 100) next = 100;
      MomentValues::Write(SituationKey::kEffectChance, next);
    }
    break;
  }
  case kForceCritical:
    MomentValues::Write(SituationKey::kCriticalHitStage, kAlwaysCritical);
    break;
  default:
    break;
  }
  return true;
}

/// True when the rule has the action "Change the type of the move".
bool ChangesMoveType(const Rule& rule) {
  for (u32 i = 0; i < rule.action_count; i++) {
    if (rule.actions[i].kind == kChangeMoveType) return true;
  }
  return false;
}

bool IsKnownCondition(u8 kind) {
  return kind >= kPokemonIsOwner && kind <= kLastCondition;
}

bool IsKnownAction(u8 kind) { return kind >= kChangeStat && kind <= kLastAction; }

} // namespace

bool Reader::Skip(u32 count) {
  if (count > size_ - at_) return false;
  at_ += count;
  return true;
}

const u8* Reader::Take(u32 count) {
  if (count > size_ - at_) return nullptr;
  const u8* out = bytes_ + at_;
  at_ += count;
  return out;
}

bool Reader::Text(c16* out, u32 capacity) {
  const u8* length_bytes = Take(2);
  if (length_bytes == nullptr) return false;
  const u32 length = Read16(length_bytes);
  const u8* units = Take(length * 2);
  if (units == nullptr) return false;
  if (out != nullptr && capacity != 0) {
    u32 copied = 0;
    while (copied < length && copied + 1 < capacity) {
      out[copied] = static_cast<c16>(Read16(units + copied * 2));
      copied++;
    }
    out[copied] = 0;
  }
  return true;
}

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

bool ReadTexts(Reader* reader, u32 text_count, c16* const* outs,
               const u32* capacities) {
  const u32 wanted = LanguageIndex(sys::Core::GetInstance().GetLanguage());
  for (u32 language = 0; language < kLanguageCount; language++) {
    for (u32 text = 0; text < text_count; text++) {
      const bool keep = language == wanted;
      if (!reader->Text(keep ? outs[text] : nullptr,
                        keep ? capacities[text] : 0)) {
        return false;
      }
    }
  }
  return true;
}

bool ReadRules(Reader* reader, u32 count, Rule* rules) {
  for (u32 i = 0; i < count; i++) {
    const u8* head = reader->Take(4);
    if (head == nullptr) return false;
    Rule& rule = rules[i];
    rule.moment = Read16(head);
    rule.condition_count = head[2];
    rule.action_count = head[3];
    if (rule.condition_count > kMaxConditions ||
        rule.action_count > kMaxActions) {
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

void Execute(const Rule& rule, u32 rule_index, Controller* controller,
             UID owner, s32* local_state) {
  // A new move of the owner: the type that the ability gave to the last move
  // is forgotten, before the conditions (the move can keep its type).
  if (rule.moment == kMomentMoveData && ChangesMoveType(rule) &&
      Value(SituationKey::kPokemonId) == owner.value) {
    local_state[kStateMoveType] = 0;
  }
  bool only_once = false;
  for (u32 i = 0; i < rule.condition_count; i++) {
    const Condition& condition = rule.conditions[i];
    if (condition.kind == kOnlyOnce) {
      only_once = true;
      // The listener keeps one bit for each rule in its first number.
      if ((local_state[kStateOnce] & (1 << rule_index)) != 0) return;
      continue;
    }
    if (!Test(condition, controller, owner, local_state)) return;
  }
  if (only_once) local_state[kStateOnce] |= (1 << rule_index);

  for (u32 i = 0; i < rule.action_count; i++) {
    const bool more = i + 1 < rule.action_count;
    if (!Run(rule.actions[i], rule.moment, controller, owner, local_state,
             more)) {
      return;
    }
  }
}

void MakePath(const c16* folder, u32 id, const c16* suffix, c16* out) {
  u32 at = 0;
  for (const c16* c = folder; *c != 0; c++) out[at++] = *c;
  c16 digits[6];
  u32 count = 0;
  do {
    digits[count++] = static_cast<c16>(u'0' + id % 10);
    id /= 10;
  } while (id != 0);
  while (count > 0) out[at++] = digits[--count];
  for (const c16* c = suffix; *c != 0; c++) out[at++] = *c;
  out[at] = 0;
}

} // namespace studio
} // namespace battle
