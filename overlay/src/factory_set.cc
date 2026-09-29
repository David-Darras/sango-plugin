#include "factory_set.h"

#include "core/utils.h"
#include "pokemon/address.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/type.h"
#include "pokemon/native/core_data.h"
#include "pokemon/native/database.h"
#include "pokemon/native/mega_evolution_table.h"
#include "pokemon/native/move_data.h"
#include "pokemon/native/movepool.h"
#include "pokemon/native/species_data.h"
#include "system/native/string.h"

namespace factory {
namespace {
using pokemon::AbilityId;
using pokemon::Ball;
using pokemon::FormId;
using pokemon::ItemId;
using pokemon::MoveData;
using pokemon::MoveId;
using pokemon::Nature;
using pokemon::SpeciesData;
using pokemon::SpeciesId;
using pokemon::TypeId;

constexpr u32 kLevel = 50;
constexpr u32 kMaxLearnable = 128;
constexpr u32 kMinPower = 40;
constexpr u8 kPhysical = 1;
constexpr u8 kSpecial = 2;

constexpr u32 kStatHp = 0;
constexpr u32 kStatAttack = 1;
constexpr u32 kStatDefense = 2;
constexpr u32 kStatSpecialAttack = 3;
constexpr u32 kStatSpecialDefense = 4;
constexpr u32 kStatSpeed = 5;

const SpeciesId kSpeciesPool[] = {
    SpeciesId::kVenusaur, SpeciesId::kCharizard, SpeciesId::kBlastoise,
    SpeciesId::kAlakazam, SpeciesId::kMachamp, SpeciesId::kGengar,
    SpeciesId::kGyarados, SpeciesId::kLapras, SpeciesId::kSnorlax,
    SpeciesId::kDragonite, SpeciesId::kJolteon, SpeciesId::kVaporeon,
    SpeciesId::kStarmie, SpeciesId::kArcanine, SpeciesId::kCloyster,
    SpeciesId::kNidoking, SpeciesId::kNidoqueen, SpeciesId::kAerodactyl,
    SpeciesId::kMeganium, SpeciesId::kTyphlosion, SpeciesId::kFeraligrator,
    SpeciesId::kCrobat, SpeciesId::kScizor, SpeciesId::kHeracross,
    SpeciesId::kSkarmory, SpeciesId::kKingdra, SpeciesId::kTyranitar,
    SpeciesId::kHoundoom, SpeciesId::kBlissey, SpeciesId::kDonphan,
    SpeciesId::kSceptile, SpeciesId::kBlaziken, SpeciesId::kSwampert,
    SpeciesId::kGardevoir, SpeciesId::kBreloom, SpeciesId::kSlaking,
    SpeciesId::kAggron, SpeciesId::kMedicham, SpeciesId::kFlygon,
    SpeciesId::kAltaria, SpeciesId::kMilotic, SpeciesId::kSalamence,
    SpeciesId::kMetagross, SpeciesId::kLudicolo, SpeciesId::kSwellow,
    SpeciesId::kTorterra, SpeciesId::kInfernape, SpeciesId::kEmpoleon,
    SpeciesId::kStaraptor, SpeciesId::kLuxray, SpeciesId::kRoserade,
    SpeciesId::kGarchomp, SpeciesId::kLucario, SpeciesId::kWeavile,
    SpeciesId::kMagnezone, SpeciesId::kRhyperior, SpeciesId::kTogekiss,
    SpeciesId::kGliscor, SpeciesId::kMamoswine, SpeciesId::kGallade,
    SpeciesId::kSerperior, SpeciesId::kEmboar, SpeciesId::kSamurott,
    SpeciesId::kExcadrill, SpeciesId::kConkeldurr, SpeciesId::kKrookodile,
    SpeciesId::kDarmanitan, SpeciesId::kZoroark, SpeciesId::kReuniclus,
    SpeciesId::kAmoonguss, SpeciesId::kJellicent, SpeciesId::kFerrothorn,
    SpeciesId::kEelektross, SpeciesId::kChandelure, SpeciesId::kHaxorus,
    SpeciesId::kMienshao, SpeciesId::kBraviary, SpeciesId::kHydreigon,
    SpeciesId::kVolcarona, SpeciesId::kChesnaught, SpeciesId::kDelphox,
    SpeciesId::kGreninja, SpeciesId::kTalonflame, SpeciesId::kAegislash,
    SpeciesId::kBarbaracle, SpeciesId::kDragalge, SpeciesId::kClawitzer,
    SpeciesId::kHeliolisk, SpeciesId::kTyrantrum, SpeciesId::kAurorus,
    SpeciesId::kHawlucha, SpeciesId::kGoodra, SpeciesId::kKlefki,
    SpeciesId::kTrevenant, SpeciesId::kNoivern,
    SpeciesId::kNinetales, SpeciesId::kGolem, SpeciesId::kRapidash,
    SpeciesId::kSlowbro, SpeciesId::kSlowking, SpeciesId::kDodrio,
    SpeciesId::kExeggutor, SpeciesId::kHitmonlee, SpeciesId::kHitmonchan,
    SpeciesId::kHitmontop, SpeciesId::kWeezing, SpeciesId::kRhydon,
    SpeciesId::kKangaskhan, SpeciesId::kScyther, SpeciesId::kPinsir,
    SpeciesId::kTauros, SpeciesId::kKabutops, SpeciesId::kOmastar,
    SpeciesId::kEspeon, SpeciesId::kUmbreon, SpeciesId::kFlareon,
    SpeciesId::kLeafeon, SpeciesId::kGlaceon, SpeciesId::kSylveon,
    SpeciesId::kSteelix, SpeciesId::kForretress, SpeciesId::kUrsaring,
    SpeciesId::kMagcargo, SpeciesId::kManectric, SpeciesId::kSharpedo,
    SpeciesId::kCamerupt, SpeciesId::kGlalie, SpeciesId::kWalrein,
    SpeciesId::kAbsol, SpeciesId::kBanette, SpeciesId::kVileplume,
    SpeciesId::kBellossom, SpeciesId::kVictreebel, SpeciesId::kTentacruel,
    SpeciesId::kHonchkrow, SpeciesId::kMismagius, SpeciesId::kDrifblim,
    SpeciesId::kLopunny, SpeciesId::kSpiritomb, SpeciesId::kGastrodon,
    SpeciesId::kYanmega, SpeciesId::kElectivire, SpeciesId::kMagmortar,
    SpeciesId::kPorygon2, SpeciesId::kPorygonZ, SpeciesId::kProbopass,
    SpeciesId::kFroslass, SpeciesId::kScolipede, SpeciesId::kWhimsicott,
    SpeciesId::kLilligant, SpeciesId::kCofagrigus, SpeciesId::kCarracosta,
    SpeciesId::kArcheops, SpeciesId::kEscavalier, SpeciesId::kGalvantula,
    SpeciesId::kAccelgor, SpeciesId::kBisharp, SpeciesId::kMandibuzz,
    SpeciesId::kStunfisk, SpeciesId::kGogoat, SpeciesId::kPangoro,
    SpeciesId::kMalamar, SpeciesId::kDiggersby, SpeciesId::kAvalugg,
    SpeciesId::kIncineroar, SpeciesId::kPrimarina, SpeciesId::kDecidueye,
    SpeciesId::kToxapex, SpeciesId::kSalazzle, SpeciesId::kBewear,
    SpeciesId::kGolisopod, SpeciesId::kMudsdale, SpeciesId::kTurtonator,
    SpeciesId::kDrampa, SpeciesId::kAraquanid, SpeciesId::kVikavolt,
    SpeciesId::kCrabominable, SpeciesId::kRibombee, SpeciesId::kTogedemaru,
    SpeciesId::kKommoO, SpeciesId::kPalossand, SpeciesId::kOranguru,
    SpeciesId::kPassimian, SpeciesId::kBruxish
};

const Ball kBallPool[] = {
    Ball::kPokeBall, Ball::kGreatBall, Ball::kUltraBall, Ball::kNetBall,
    Ball::kDiveBall, Ball::kNestBall, Ball::kRepeatBall, Ball::kTimerBall,
    Ball::kLuxuryBall, Ball::kPremierBall, Ball::kDuskBall, Ball::kHealBall,
    Ball::kQuickBall,
};

const MoveId kBannedMoves[] = {
    MoveId::kHyperBeam, MoveId::kGigaImpact, MoveId::kBlastBurn,
    MoveId::kFrenzyPlant, MoveId::kHydroCannon, MoveId::kRoarOfTime,
    MoveId::kRockWrecker, MoveId::kExplosion, MoveId::kSelfDestruct,
    MoveId::kFocusPunch, MoveId::kDreamEater, MoveId::kSolarBeam,
    MoveId::kSkyAttack, MoveId::kSkullBash, MoveId::kRazorWind,
    MoveId::kFly, MoveId::kDig, MoveId::kDive, MoveId::kBounce,
    MoveId::kLastResort, MoveId::kFling, MoveId::kNaturalGift,
    MoveId::kBelch, MoveId::kSnore, MoveId::kSpitUp, MoveId::kFakeOut,
    MoveId::kSkyDrop, MoveId::kShadowForce, MoveId::kPhantomForce,
    MoveId::kFreezeShock, MoveId::kIceBurn, MoveId::kStruggle,
};

const MoveId kPhysicalSetup[] = {
    MoveId::kSwordsDance, MoveId::kDragonDance, MoveId::kBulkUp,
    MoveId::kShellSmash,
};
const MoveId kSpecialSetup[] = {
    MoveId::kNastyPlot, MoveId::kCalmMind, MoveId::kQuiverDance,
    MoveId::kShellSmash,
};
const MoveId kRecovery[] = {
    MoveId::kRecover, MoveId::kRoost, MoveId::kSynthesis, MoveId::kMoonlight,
};
const MoveId kUtility[] = {
    MoveId::kProtect, MoveId::kSubstitute, MoveId::kThunderWave,
    MoveId::kWillOWisp, MoveId::kToxic,
};

template <typename T, u32 N>
bool Contains(const T (&list)[N], T value) {
  for (u32 i = 0; i < N; i++) {
    if (list[i] == value) return true;
  }
  return false;
}

bool Contains(const MoveId* list, u32 count, MoveId move) {
  for (u32 i = 0; i < count; i++) {
    if (list[i] == move) return true;
  }
  return false;
}

struct Profile {
  u32 base[6];
  TypeId types[2];
  bool is_physical;
  bool is_fast;
  bool is_bulky;

  u32 Offense() const {
    return is_physical ? base[kStatAttack] : base[kStatSpecialAttack];
  }
};

Profile MakeProfile(const SpeciesData& data) {
  Profile profile;
  profile.base[kStatHp] = data.base_hp;
  profile.base[kStatAttack] = data.base_attack;
  profile.base[kStatDefense] = data.base_defense;
  profile.base[kStatSpecialAttack] = data.base_special_attack;
  profile.base[kStatSpecialDefense] = data.base_special_defense;
  profile.base[kStatSpeed] = data.base_speed;
  profile.types[0] = data.type[0];
  profile.types[1] = data.type[1];

  const u32 attack = data.base_attack;
  const u32 special = data.base_special_attack;
  if (attack + 15 < special) {
    profile.is_physical = false;
  } else if (special + 15 < attack) {
    profile.is_physical = true;
  } else {
    profile.is_physical = core::Utils::GetRandomValue(2) == 0;
  }
  profile.is_fast = data.base_speed >= 80;
  profile.is_bulky = data.base_hp >= 90 ||
                     data.base_hp + data.base_defense +
                     data.base_special_defense >= 260;
  return profile;
}

bool IsStab(const Profile& profile, TypeId type) {
  return type == profile.types[0] || type == profile.types[1];
}

u32 CollectMoves(SpeciesId species, MoveId* out, u32 capacity, u32 max_level) {
  auto& pool = pokemon::Movepool::GetInstance(species, FormId::kNormal);
  u32 count = 0;
  for (u32 i = 0; i < pool.count && count < capacity; i++) {
    const MoveId move = pool.entry[i].move;
    if (pool.entry[i].level > max_level || move == MoveId::kNone) continue;
    if (Contains(kBannedMoves, move)) continue;
    if (Contains(out, count, move)) continue;
    out[count++] = move;
  }
  return count;
}

u32 ScoreMove(const Profile& profile, const MoveData& data) {
  const u32 accuracy = data.accuracy == 0 ? 100 : data.accuracy;
  u32 score = data.power * accuracy / 100;
  if (IsStab(profile, data.type)) score = score * 3 / 2;
  const u8 wanted = profile.is_physical ? kPhysical : kSpecial;
  if (data.damage_category != wanted) score /= 4;
  return score * (80 + core::Utils::GetRandomValue(40)) / 100;
}

struct MoveChoice {
  MoveId moves[4];
  u32 count = 0;

  bool Has(MoveId move) const { return Contains(moves, count, move); }

  bool Add(MoveId move) {
    if (count >= 4 || move == MoveId::kNone || Has(move)) return false;
    moves[count++] = move;
    return true;
  }

  bool HasType(TypeId type) const {
    for (u32 i = 0; i < count; i++) {
      if (MoveData::GetInstance(moves[i]).type == type) return true;
    }
    return false;
  }
};

enum class Want { kAny, kStab, kCoverage };

MoveId PickAttack(const Profile& profile, const MoveId* learnable, u32 count,
                  const MoveChoice& chosen, Want want, bool same_category,
                  TypeId only_type = TypeId::kCount) {
  const u8 wanted = profile.is_physical ? kPhysical : kSpecial;
  MoveId best = MoveId::kNone;
  u32 best_score = 0;
  for (u32 i = 0; i < count; i++) {
    const MoveId move = learnable[i];
    if (chosen.Has(move)) continue;
    const MoveData& data = MoveData::GetInstance(move);
    if (data.damage_category == 0 || data.power < kMinPower) continue;
    if (same_category && data.damage_category != wanted) continue;
    if (only_type != TypeId::kCount && data.type != only_type) continue;
    const bool stab = IsStab(profile, data.type);
    if (want == Want::kStab && !stab) continue;
    if (want == Want::kCoverage && (stab || chosen.HasType(data.type)))
      continue
          ;
    const u32 score = ScoreMove(profile, data);
    if (score > best_score) {
      best_score = score;
      best = move;
    }
  }
  return best;
}

MoveId PickFrom(const MoveId* list, u32 list_count, const MoveId* learnable,
                u32 count, const MoveChoice& chosen) {
  MoveId found[8];
  u32 found_count = 0;
  for (u32 i = 0; i < list_count && found_count < 8; i++) {
    if (Contains(learnable, count, list[i]) && !chosen.Has(list[i])) {
      found[found_count++] = list[i];
    }
  }
  if (found_count == 0) return MoveId::kNone;
  return found[core::Utils::GetRandomValue(found_count)];
}

bool IsAttackOnlyItem(ItemId item) {
  return item == ItemId::kChoiceBand || item == ItemId::kChoiceSpecs ||
         item == ItemId::kChoiceScarf || item == ItemId::kAssaultVest;
}

MoveChoice ChooseMoves(SpeciesId species, const Profile& profile, ItemId item) {
  MoveId learnable[kMaxLearnable];
  u32 count = CollectMoves(species, learnable, kMaxLearnable, kLevel);
  MoveChoice chosen;

  if (PickAttack(profile, learnable, count, chosen, Want::kStab, false) ==
      MoveId::kNone) {
    count = CollectMoves(species, learnable, kMaxLearnable, 100);
  }

  MoveId stab = PickAttack(profile, learnable, count, chosen, Want::kStab,
                           true);
  if (stab == MoveId::kNone) {
    stab = PickAttack(profile, learnable, count, chosen, Want::kStab, false);
  }
  chosen.Add(stab);

  if (profile.types[0] != profile.types[1]) {
    const TypeId other = chosen.HasType(profile.types[0])
                           ? profile.types[1]
                           : profile.types[0];
    chosen.Add(PickAttack(profile, learnable, count, chosen, Want::kStab, false,
                          other));
  }

  chosen.Add(PickAttack(profile, learnable, count, chosen, Want::kCoverage,
                        true));

  if (!IsAttackOnlyItem(item) && chosen.count < 4) {
    MoveId support = MoveId::kNone;
    if (profile.is_bulky) {
      support = PickFrom(kRecovery, SIZE(kRecovery), learnable, count, chosen);
    }
    if (support == MoveId::kNone && profile.Offense() >= 90) {
      support = profile.is_physical
                  ? PickFrom(kPhysicalSetup, SIZE(kPhysicalSetup), learnable,
                             count,
                             chosen)
                  : PickFrom(kSpecialSetup, SIZE(kSpecialSetup), learnable,
                             count,
                             chosen);
    }
    if (support == MoveId::kNone) {
      support = PickFrom(kUtility, SIZE(kUtility), learnable, count, chosen);
    }
    chosen.Add(support);
  }

  for (u32 pass = 0; pass < 3 && chosen.count < 4; pass++) {
    while (chosen.count < 4) {
      const MoveId move = PickAttack(profile, learnable, count, chosen,
                                     pass == 0 ? Want::kCoverage : Want::kAny,
                                     pass < 2);
      if (move == MoveId::kNone) break;
      chosen.Add(move);
    }
  }
  for (u32 i = 0; i < count && chosen.count < 4; i++) chosen.Add(learnable[i]);
  if (chosen.count == 0) chosen.Add(MoveId::kTackle);
  return chosen;
}

ItemId GetMegaStone(SpeciesId species) {
  ((void (*)(SpeciesId))pokemon::address::kLoadMegaEvolutionTable)(species);
  const auto& table = *pokemon::Database::GetInstance().mega_evolution->data;
  ItemId stones[3];
  u32 count = 0;
  for (const auto& entry : table.entry) {
    if (entry.method != MegaEvolutionMethod::kItem) continue;
    if (entry.item == ItemId::kNone || entry.form == FormId::kNormal) continue;
    stones[count++] = entry.item;
  }
  if (count == 0) return ItemId::kNone;
  return stones[core::Utils::GetRandomValue(count)];
}

bool IsUsed(const ItemId* used, u32 count, ItemId item) {
  for (u32 i = 0; i < count; i++) {
    if (used[i] == item) return true;
  }
  return false;
}

ItemId ChooseItem(const Profile& profile, const ItemId* used, u32 used_count) {
  ItemId prefs[16];
  u32 count = 0;
  auto add = [&](ItemId item) { prefs[count++] = item; };

  const ItemId choice = profile.is_physical
                          ? ItemId::kChoiceBand
                          : ItemId::kChoiceSpecs;
  const u32 roll = core::Utils::GetRandomValue(3);
  if (profile.Offense() >= 95) {
    if (profile.is_fast && roll == 0) add(ItemId::kChoiceScarf);
    if (roll == 1 || !profile.is_fast) add(choice);
    add(ItemId::kLifeOrb);
    add(choice);
  }
  if (profile.is_bulky) {
    if (!profile.is_physical && profile.Offense() < 95) {
      add(ItemId::kAssaultVest);
    }
    add(ItemId::kLeftovers);
    add(ItemId::kSitrusBerry);
  } else if (profile.is_fast) {
    add(ItemId::kFocusSash);
  }
  add(ItemId::kLifeOrb);
  add(ItemId::kExpertBelt);
  add(ItemId::kLumBerry);
  add(ItemId::kLeftovers);
  add(ItemId::kSitrusBerry);
  add(ItemId::kWeaknessPolicy);

  for (u32 i = 0; i < count; i++) {
    if (!IsUsed(used, used_count, prefs[i])) return prefs[i];
  }
  return ItemId::kNone;
}

Nature ChooseNature(const Profile& profile) {
  const bool slow = profile.base[kStatSpeed] < 50;
  if (profile.is_physical) {
    if (slow) return Nature::kBrave;
    return profile.is_fast ? Nature::kJolly : Nature::kAdamant;
  }
  if (slow) return Nature::kQuiet;
  return profile.is_fast ? Nature::kTimid : Nature::kModest;
}

void ChooseEvs(const Profile& profile, u8* evs) {
  for (u32 i = 0; i < 6; i++) evs[i] = 0;
  evs[profile.is_physical ? kStatAttack : kStatSpecialAttack] = 252;
  if (profile.is_fast) {
    evs[kStatSpeed] = 252;
    evs[kStatHp] = 4;
  } else {
    evs[kStatHp] = 252;
    evs[profile.base[kStatDefense] <= profile.base[kStatSpecialDefense]
          ? kStatDefense
          : kStatSpecialDefense] = 4;
  }
}

void GetSpeciesName(SpeciesId species, c16* out, u32 length) {
  sys::String str;
  str.buffer = out;
  str.capacity = length;
  str.size = 0;
  ((void (*)(sys::String*, SpeciesId))pokemon::address::kGetSpeciesName)(
      &str, species);
}
} // namespace

SpeciesId GetRandomSpecies(const SpeciesId* excluded, u32 excluded_count) {
  while (true) {
    const SpeciesId candidate =
        kSpeciesPool[core::Utils::GetRandomValue(SIZE(kSpeciesPool))];
    bool conflict = false;
    for (u32 i = 0; i < excluded_count; i++) {
      if (excluded[i] == candidate) conflict = true;
    }
    if (!conflict) return candidate;
  }
}

PokemonSpec BuildSpec(SpeciesId species, const ItemId* used_items,
                      u32 used_count) {
  const SpeciesData& data = SpeciesData::GetInstance(species, FormId::kNormal);
  const Profile profile = MakeProfile(data);

  PokemonSpec spec;
  spec.species = species;

  ItemId stone = GetMegaStone(species);
  if (stone != ItemId::kNone && IsUsed(used_items, used_count, stone)) {
    stone = ItemId::kNone;
  }
  spec.item = stone != ItemId::kNone
                ? stone
                : ChooseItem(profile, used_items, used_count);

  spec.nature = ChooseNature(profile);
  ChooseEvs(profile, spec.evs);
  spec.ball = kBallPool[core::Utils::GetRandomValue(SIZE(kBallPool))];

  spec.ability = data.ability[0];
  const u32 slot = core::Utils::GetRandomValue(3);
  if (data.ability[slot] != AbilityId::kNone) spec.ability = data.ability[slot];

  const MoveChoice moves = ChooseMoves(species, profile, spec.item);
  for (u32 i = 0; i < 4; i++) {
    spec.moves[i] = i < moves.count ? moves.moves[i] : MoveId::kNone;
  }
  return spec;
}

void ApplySpec(savedata::PokemonParam* pokemon, const PokemonSpec& spec) {
  pokemon->accessor->Decrypt();
  pokemon::CoreData* core = pokemon->core;

  core->Set(spec.species, spec.item, spec.ability, spec.nature, false);
  core->form = FormId::kNormal;
  core->SetStats(spec.evs[kStatHp], spec.evs[kStatAttack],
                 spec.evs[kStatDefense], spec.evs[kStatSpecialAttack],
                 spec.evs[kStatSpecialDefense], spec.evs[kStatSpeed]);
  core->SetLevel(kLevel);
  core->is_egg = 0;
  core->is_illegal_egg = 0;
  core->ball = spec.ball;
  core->SetMoves(spec.moves[0], spec.moves[1], spec.moves[2], spec.moves[3]);
  for (u32 i = 0; i < 4; i++) {
    core->pp[i] = spec.moves[i] == MoveId::kNone
                    ? 0
                    : MoveData::GetInstance(spec.moves[i]).base_pp;
    core->pp_up_count[i] = 0;
  }

  c16 name[13] = {};
  GetSpeciesName(spec.species, name, 13);
  core->SetNickname(name);
  core->use_nickname = false;

  pokemon->accessor->Encrypt();
  pokemon->UpdateRuntimeData();
}
} // namespace factory