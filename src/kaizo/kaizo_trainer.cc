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

#include <initializer_list>

#include "utils.h"
#include "feature/battle/feature_battle.h"
#include "game/battle/config.h"
#include "game/constant/battle_format.h"
#include "game/constant/battle_trainer.h"
#include "game/constant/move.h"
#include "game/constant/ability.h"
#include "game/constant/battle_ai.h"
#include "game/constant/battle_background.h"
#include "game/constant/battle_encounter_animation.h"
#include "game/constant/battle_ground.h"
#include "game/constant/battle_platform.h"
#include "game/constant/weather.h"
#include "game/constant/form.h"
#include "game/constant/item.h"
#include "game/constant/nature.h"
#include "game/constant/species.h"
#include "game/global_data/pokemon.h"
#include "game/savedata/pokemon_data_accessor.h"
#include "game/savedata/pokemon_team.h"
#include "ui/log_application.h"

namespace kaizo {
static u8 s_count = 0;
static PokemonCoreData s_backup[6];

void SaveTeamBeforeBattle() {
  auto& team = savedata::PokemonTeam::GetInstance();
  s_count = team.count;
  for (u32 i = 0; i < team.count; i++) {
    auto* core = team.pokemons[i]->core;
    std::memcpy(&s_backup[i], core, sizeof(*core));
  }
}

void RestoreTeamAfterBattle() {
  if (s_count == 0) return;
  auto& team = savedata::PokemonTeam::GetInstance();
  team.count = s_count;
  for (u32 i = 0; i < team.count; i++) {
    auto* core = team.pokemons[i]->core;
    std::memcpy(core, &s_backup[i], sizeof(*core));
  }
  s_count = 0;
}

// ----------------------------------------------------------------------
// Trainer data tables.
//
// Every hand-picked trainer battle used to be a `PatchTrainer_Xxx()`
// function that imperatively called `.Set()` / `.SetStats()` /
// `.SetMoves()` on each opponent slot: ~550 lines that were really just
// data (species/item/ability/nature/EVs/moves per opponent) written as
// code. This is now a table of `TrainerSpec` values consumed by a single
// `ApplyTrainerSpec()` function, so adding/rebalancing a trainer is a
// one-line data change instead of copy-pasting a whole function.
// ----------------------------------------------------------------------

/**
* @brief One opponent slot's data: mirrors, in order, the arguments
* originally passed to PokemonCoreData::Set(), ::SetStats() and
* ::SetMoves(), plus the optional form/nickname/level overrides some
* opponents used.
*/
struct TrainerOpponentSpec {
  Species species;
  ItemId item;
  Ability ability;
  Nature nature;
  bool is_shiny;

  u8 ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense, ev_speed;

  MoveId move1, move2, move3, move4;

  u8 form; // 0 = leave the species' default form untouched
  const c16* nickname; // nullptr = keep the species' default name
  u8 forced_level; // 0 = don't force a level (PatchTrainer_Level decides)

  TrainerOpponentSpec()
    : species(Species::kNone), item(ItemId::kNone), ability(Ability::kNone),
      nature(Nature::kHardy), is_shiny(false), ev_hp(0),
      ev_attack(0), ev_defense(0), ev_sp_attack(0), ev_sp_defense(0),
      ev_speed(0), move1(MoveId::kNone), move2(MoveId::kNone),
      move3(MoveId::kNone), move4(MoveId::kNone), form(0),
      nickname(nullptr), forced_level(0) {
  }

  TrainerOpponentSpec(Species species, ItemId item, Ability ability,
                      Nature nature,
                      bool is_shiny, u8 ev_hp, u8 ev_attack, u8 ev_defense,
                      u8 ev_sp_attack, u8 ev_sp_defense, u8 ev_speed,
                      MoveId move1, MoveId move2, MoveId move3, MoveId move4,
                      u8 form = 0, const c16* nickname = nullptr,
                      u8 forced_level = 0)
    : species(species), item(item), ability(ability), nature(nature),
      is_shiny(is_shiny), ev_hp(ev_hp), ev_attack(ev_attack),
      ev_defense(ev_defense), ev_sp_attack(ev_sp_attack),
      ev_sp_defense(ev_sp_defense), ev_speed(ev_speed), move1(move1),
      move2(move2), move3(move3), move4(move4), form(form),
      nickname(nickname), forced_level(forced_level) {
  }

  void ApplyTo(PokemonCoreData& pkm) const {
    pkm.Set(species, item, ability, nature, is_shiny);
    // `form` is a sentinel-carrying raw value here (0 = keep the default).
    if (form != 0) pkm.form = static_cast<Form>(form);
    pkm.SetStats(ev_hp, ev_attack, ev_defense, ev_sp_attack, ev_sp_defense,
                ev_speed);
    pkm.SetMoves(move1, move2, move3, move4);
    if (nickname != nullptr) pkm.SetNickname(nickname);
    if (forced_level != 0) pkm.SetLevel(forced_level);
  }
};

/**
* @brief A whole trainer battle: mirrors the arguments originally passed
* to battle::Config::Set(), plus the list of opponents and the one-off
* `config.battle_type = 0;` override that Georgia used.
*/
struct TrainerSpec {
  u8 opponent_count;
  BattleFormat format;
  BattleBackground background;
  BattleGround ground;
  BattlePlatform platform;
  BattleEncounterAnimation encounter_animation;
  BattleWeather weather;
  bool force_wild_battle_type;
  TrainerOpponentSpec opponents[6];

  TrainerSpec(u8 opponent_count, BattleFormat format,
             BattleBackground background, BattleGround ground,
             BattlePlatform platform,
             BattleEncounterAnimation encounter_animation,
             BattleWeather weather,
             std::initializer_list<TrainerOpponentSpec> opponent_list,
             bool force_wild_battle_type = false)
    : opponent_count(opponent_count), format(format), background(background),
      ground(ground), platform(platform),
      encounter_animation(encounter_animation), weather(weather),
      force_wild_battle_type(force_wild_battle_type) {
    u32 i = 0;
    for (const auto& opponent : opponent_list) {
      opponents[i++] = opponent;
    }
  }
};

void ApplyTrainerSpec(battle::Config& config, const TrainerSpec& spec) {
  if (spec.force_wild_battle_type) config.battle_type = 0;
  config.Set(spec.opponent_count, spec.format, spec.background, spec.ground,
             spec.platform, spec.encounter_animation, spec.weather);
  for (u32 i = 0; i < spec.opponent_count; i++) {
    spec.opponents[i].ApplyTo(config.GetOpponent(i));
  }
}

static const TrainerSpec kMaySpec(
    5, BattleFormat::kHorde, BattleBackground::kSkyPillarTop,
    BattleGround::kSkyPillarTop, BattlePlatform::kSkyPillarTop,
    BattleEncounterAnimation::kRayquaza, BattleWeather::kStrongWinds,
    {
        {Species::kWhismur, ItemId::kNone, Ability::kRattled, Nature::kModest, false,
         0, 0, 0, 0, 0, 0, MoveId::kNone, MoveId::kNone, MoveId::kNone, MoveId::kNone, 0,
         nullptr, 1},
        {Species::kWhismur, ItemId::kLifeOrb, Ability::kSoundproof, Nature::kModest,
         true, 244, 0, 12, 188, 0, 12, MoveId::kHyperVoice, MoveId::kFireBlast,
         MoveId::kShadowBall, MoveId::kExtrasensory, 0, u"Hash"},
        {Species::kWhismur, ItemId::kNone, Ability::kRattled, Nature::kModest, false,
         0, 0, 0, 0, 0, 0, MoveId::kNone, MoveId::kNone, MoveId::kNone, MoveId::kNone, 0,
         nullptr, 1},
        {Species::kWhismur, ItemId::kNone, Ability::kRattled, Nature::kModest, false,
         0, 0, 0, 0, 0, 0, MoveId::kNone, MoveId::kNone, MoveId::kNone, MoveId::kNone, 0,
         nullptr, 1},
        {Species::kWhismur, ItemId::kNone, Ability::kRattled, Nature::kModest, false,
         0, 0, 0, 0, 0, 0, MoveId::kNone, MoveId::kNone, MoveId::kNone, MoveId::kNone, 0,
         nullptr, 1},
    });

static const TrainerSpec kRoute102Kid1Spec(
    2, BattleFormat::kDouble, BattleBackground::kAquaBoss,
    BattleGround::kAquaBoss, BattlePlatform::kWater,
    BattleEncounterAnimation::kKyogre, BattleWeather::kHeavyRain,
    {
        {Species::kFurfrou, ItemId::kLeftovers, Ability::kFurCoat, Nature::kJolly,
         true, 0, 252, 0, 0, 4, 252, MoveId::kUTurn, MoveId::kThunderWave,
         MoveId::kReturn, MoveId::kSuckerPunch,
         static_cast<u8>(Form::kFurfrouHeart)},
        {Species::kPoochyena, ItemId::kToxicOrb, Ability::kQuickFeet, Nature::kJolly,
         true, 0, 236, 0, 0, 36, 236, MoveId::kCrunch, MoveId::kPlayRough,
         MoveId::kFacade, MoveId::kFireFang},
    });

static const TrainerSpec kRoute102Kid2Spec(
    3, BattleFormat::kTriple, BattleBackground::kMagmaBoss,
    BattleGround::kMagmaBoss, BattlePlatform::kVolcano,
    BattleEncounterAnimation::kGroudon, BattleWeather::kExtremelyHarshSunlight,
    {
        {Species::kAzurill, ItemId::kEviolite, Ability::kHugePower, Nature::kAdamant,
         false, 116, 196, 116, 0, 36, 0, MoveId::kReturn, MoveId::kKnockOff,
         MoveId::kWaterfall, MoveId::kIronTail},
        {Species::kPichu, ItemId::kEviolite, Ability::kLightningRod, Nature::kTimid,
         true, 36, 0, 0, 236, 0, 196, MoveId::kNastyPlot, MoveId::kSubstitute,
         MoveId::kThunderbolt, MoveId::kHiddenPower},
        {Species::kRattata, ItemId::kFlameOrb, Ability::kGuts, Nature::kJolly, false,
         0, 228, 76, 0, 0, 180, MoveId::kFacade, MoveId::kSuckerPunch,
         MoveId::kFlameWheel, MoveId::kUTurn},
    });

static const TrainerSpec kRoute102Kid3Spec(
    2, BattleFormat::kSingle, BattleBackground::kDeoxys, BattleGround::kDeoxys,
    BattlePlatform::kFlyingGymLeader, BattleEncounterAnimation::kDeoxys,
    BattleWeather::kHail,
    {
        {Species::kPurrloin, ItemId::kEviolite, Ability::kPrankster, Nature::kCareful,
         false, 188, 0, 180, 0, 60, 68, MoveId::kEncore, MoveId::kUTurn,
         MoveId::kKnockOff, MoveId::kThunderWave},
        {Species::kGlameow, ItemId::kLifeOrb, Ability::kLimber, Nature::kJolly, false,
         0, 236, 20, 0, 0, 236, MoveId::kReturn, MoveId::kKnockOff, MoveId::kFakeOut,
         MoveId::kUTurn},
    });

// Tiana Lass
static const TrainerSpec kRoute102GirlSpec(
    3, BattleFormat::kRotation, BattleBackground::kAbandonedShip,
    BattleGround::kAbandonedShip, BattlePlatform::kShip,
    BattleEncounterAnimation::kHoopa, BattleWeather::kNone,
    {
        {Species::kRaikou, ItemId::kChoiceSpecs, Ability::kPressure, Nature::kTimid,
         false, 0, 0, 0, 252, 4, 252, MoveId::kVoltSwitch, MoveId::kThunderbolt,
         MoveId::kShadowBall, MoveId::kHiddenPower},
        {Species::kEntei, ItemId::kChoiceBand, Ability::kPressure, Nature::kAdamant,
         true, 0, 252, 0, 0, 4, 252, MoveId::kSacredFire, MoveId::kFlareBlitz,
         MoveId::kExtremeSpeed, MoveId::kStoneEdge},
        {Species::kSuicune, ItemId::kLeftovers, Ability::kPressure, Nature::kTimid,
         false, 252, 0, 0, 0, 40, 216, MoveId::kSubstitute, MoveId::kProtect,
         MoveId::kCalmMind, MoveId::kScald},
    });

static const TrainerSpec kRoute104YoungsterBillySpec(
    2, BattleFormat::kSingle, BattleBackground::kSkyBattle,
    BattleGround::kSkyBattle, BattlePlatform::kSky,
    BattleEncounterAnimation::kGymLeaderFlying, BattleWeather::kInvalid,
    {
        {Species::kXatu, ItemId::kRockyHelmet, Ability::kMagicBounce, Nature::kTimid,
         true, 252, 0, 240, 0, 0, 16, MoveId::kPsychic, MoveId::kRoost,
         MoveId::kGrassKnot, MoveId::kUTurn},
        {Species::kTaillow, ItemId::kLifeOrb, Ability::kScrappy, Nature::kNaive,
         false, 0, 0, 36, 196, 36, 236, MoveId::kBoomburst, MoveId::kHeatWave,
         MoveId::kHiddenPower, MoveId::kBraveBird},
    });

static const TrainerSpec kRoute104RichBoyWinstonSpec(
    2, BattleFormat::kDouble, BattleBackground::kSkyBattle,
    BattleGround::kSkyBattle, BattlePlatform::kSky,
    BattleEncounterAnimation::kGymLeaderFlying, BattleWeather::kInvalid,
    {
        {Species::kPidove, ItemId::kLifeOrb, Ability::kSuperLuck, Nature::kNaive,
         false, 0, 236, 0, 0, 0, 252, MoveId::kTailwind, MoveId::kFacade,
         MoveId::kAerialAce, MoveId::kHeatWave},
        {Species::kDucklett, ItemId::kEviolite, Ability::kHydration, Nature::kCalm,
         true, 180, 0, 196, 4, 116, 0, MoveId::kAirSlash, MoveId::kDefog,
         MoveId::kRoost, MoveId::kScald},
    });

static const TrainerSpec kPetalburgWoodsBugCatcherLyleSpec(
    4, BattleFormat::kDouble, BattleBackground::kForest, BattleGround::kForest,
    BattlePlatform::kForest, BattleEncounterAnimation::kRayquaza,
    BattleWeather::kInvalid,
    {
        {Species::kMetapod, ItemId::kEviolite, Ability::kShedSkin, Nature::kNaughty,
         false, 248, 252, 0, 8, 0, 0, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kIronDefense, MoveId::kTackle},
        {Species::kKakuna, ItemId::kEviolite, Ability::kShedSkin, Nature::kNaughty,
         true, 248, 252, 0, 8, 0, 0, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kIronDefense, MoveId::kPoisonSting},
        {Species::kSilcoon, ItemId::kEviolite, Ability::kShedSkin, Nature::kNaughty,
         false, 248, 252, 0, 8, 0, 0, MoveId::kElectroweb, MoveId::kIronDefense,
         MoveId::kBugBite, MoveId::kPoisonSting},
        {Species::kCascoon, ItemId::kEviolite, Ability::kShedSkin, Nature::kNaughty,
         true, 248, 252, 0, 8, 0, 0, MoveId::kElectroweb, MoveId::kIronDefense,
         MoveId::kBugBite, MoveId::kPoisonSting},
    });

static const TrainerSpec kPetalburgWoodsTeamAquaGruntSpec(
    5, BattleFormat::kHorde, BattleBackground::kForest, BattleGround::kForest,
    BattlePlatform::kForest, BattleEncounterAnimation::kRayquaza,
    BattleWeather::kInvalid,
    {
        {Species::kScatterbug, ItemId::kEviolite, Ability::kCompoundEyes,
         Nature::kCareful, true, 132, 76, 116, 0, 156, 0, MoveId::kStunSpore,
         MoveId::kPoisonPowder, MoveId::kBugBite, MoveId::kTackle},
        {Species::kSewaddle, ItemId::kEviolite, Ability::kChlorophyll, Nature::kBold,
         true, 156, 0, 196, 40, 116, 0, MoveId::kStickyWeb, MoveId::kGigaDrain,
         MoveId::kHiddenPower, MoveId::kAirSlash},
        {Species::kWurmple, ItemId::kEviolite, Ability::kShieldDust, Nature::kRelaxed,
         true, 156, 76, 76, 0, 196, 0, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kPoisonSting, MoveId::kStringShot},
        {Species::kWeedle, ItemId::kEviolite, Ability::kShieldDust, Nature::kAdamant,
         true, 36, 236, 36, 36, 116, 36, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kPoisonSting, MoveId::kStringShot},
        {Species::kCaterpie, ItemId::kEviolite, Ability::kShieldDust, Nature::kRelaxed,
         true, 156, 0, 236, 0, 116, 0, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kStringShot, MoveId::kTackle},
    });

static const TrainerSpec kPetalburgWoodsBugCatcherJamesSpec(
    2, BattleFormat::kSingle, BattleBackground::kForest, BattleGround::kForest,
    BattlePlatform::kForest, BattleEncounterAnimation::kRayquaza,
    BattleWeather::kInvalid,
    {
        {Species::kButterfree, ItemId::kLifeOrb, Ability::kTintedLens,
         Nature::kTimid, false, 0, 0, 4, 252, 0, 252, MoveId::kSleepPowder,
         MoveId::kQuiverDance, MoveId::kBugBuzz, MoveId::kEnergyBall},
        {Species::kBeedrill, ItemId::kBeedrillite, Ability::kSwarm, Nature::kJolly,
         false, 0, 252, 4, 0, 0, 252, MoveId::kUTurn, MoveId::kPoisonJab,
         MoveId::kDrillRun, MoveId::kKnockOff},
    });

static const TrainerSpec kRoute104LadyCindySpec(
    1, BattleFormat::kSingle, BattleBackground::kShore, BattleGround::kShore,
    BattlePlatform::kShore, BattleEncounterAnimation::kKyogre, BattleWeather::kRain,
    {
        {Species::kTotodile, ItemId::kEviolite, Ability::kSheerForce,
         Nature::kAdamant, false, 0, 236, 4, 0, 0, 252, MoveId::kDragonDance,
         MoveId::kWaterfall, MoveId::kIcePunch, MoveId::kSuperpower},
    });

static const TrainerSpec kRoute104LassHaleySpec(
    2, BattleFormat::kSingle, BattleBackground::kShore, BattleGround::kShore,
    BattlePlatform::kShore, BattleEncounterAnimation::kKyogre, BattleWeather::kRain,
    {
        {Species::kPiplup, ItemId::kEviolite, Ability::kTorrent, Nature::kBold, true,
         92, 0, 252, 0, 148, 0, MoveId::kStealthRock, MoveId::kDefog, MoveId::kScald,
         MoveId::kIceBeam},
        {Species::kOshawott, ItemId::kLifeOrb, Ability::kTorrent, Nature::kRash, true,
         0, 0, 0, 252, 0, 236, MoveId::kHydroPump, MoveId::kIceBeam, MoveId::kAirSlash,
         MoveId::kAquaJet},
    });

static const TrainerSpec kRoute104TwinsGinaAndMiaSpec(
    2, BattleFormat::kDouble, BattleBackground::kShore, BattleGround::kShore,
    BattlePlatform::kShore, BattleEncounterAnimation::kKyogre,
    BattleWeather::kHeavyRain,
    {
        {Species::kBlastoise, ItemId::kBlastoisinite, Ability::kRainDish,
         Nature::kModest, false, 248, 0, 0, 252, 8, 0, MoveId::kRapidSpin,
         MoveId::kWaterPulse, MoveId::kDarkPulse, MoveId::kAuraSphere},
        {Species::kSwampert, ItemId::kSwampertite, Ability::kDamp, Nature::kAdamant,
         false, 0, 252, 0, 0, 4, 252, MoveId::kWaterfall, MoveId::kEarthquake,
         MoveId::kIcePunch, MoveId::kPowerUpPunch},
    });

static const TrainerSpec kRoute104FishermanIvanSpec(
    5, BattleFormat::kHorde, BattleBackground::kShore, BattleGround::kShore,
    BattlePlatform::kShore, BattleEncounterAnimation::kKyogre,
    BattleWeather::kHeavyRain,
    {
        {Species::kMagikarp, ItemId::kNone, Ability::kRattled, Nature::kJolly, false,
         0, 196, 0, 0, 116, 196, MoveId::kSplash, MoveId::kTackle, MoveId::kFlail,
         MoveId::kBounce, 0, nullptr, 1},
        {Species::kMagikarp, ItemId::kNone, Ability::kRattled, Nature::kJolly, false,
         0, 196, 0, 0, 116, 196, MoveId::kSplash, MoveId::kTackle, MoveId::kFlail,
         MoveId::kBounce, 0, nullptr, 1},
        {Species::kGyarados, ItemId::kGyaradosite, Ability::kIntimidate,
         Nature::kJolly, true, 0, 252, 4, 0, 0, 252, MoveId::kDragonDance,
         MoveId::kCrunch, MoveId::kWaterfall, MoveId::kEarthquake},
        {Species::kMagikarp, ItemId::kNone, Ability::kRattled, Nature::kJolly, false,
         0, 196, 0, 0, 116, 196, MoveId::kSplash, MoveId::kTackle, MoveId::kFlail,
         MoveId::kBounce, 0, nullptr, 1},
        {Species::kMagikarp, ItemId::kNone, Ability::kRattled, Nature::kJolly, false,
         0, 196, 0, 0, 116, 196, MoveId::kSplash, MoveId::kTackle, MoveId::kFlail,
         MoveId::kBounce, 0, nullptr, 1},
    });

////////////////////////////////////////////////////////////////////////////////

static const TrainerSpec kRustboroCityYoungsterJoshSpec(
    4, BattleFormat::kDouble, BattleBackground::kRockGymLeader2,
    BattleGround::kRockGymLeader2, BattlePlatform::kRockGymLeader,
    BattleEncounterAnimation::kGymLeaderRock, BattleWeather::kSandstorm,
    {
        {Species::kOmanyte, ItemId::kEviolite, Ability::kShellArmor, Nature::kModest,
         false, 76, 0, 0, 196, 0, 236, MoveId::kShellSmash, MoveId::kHydroPump,
         MoveId::kIceBeam, MoveId::kEarthPower},
        {Species::kKabuto, ItemId::kEviolite, Ability::kWeakArmor, Nature::kAdamant,
         false, 116, 196, 196, 0, 0, 0, MoveId::kRapidSpin, MoveId::kRockSlide,
         MoveId::kKnockOff, MoveId::kWaterfall},
        {Species::kLileep, ItemId::kEviolite, Ability::kStormDrain, Nature::kCalm,
         false, 228, 0, 140, 0, 140, 0, MoveId::kStealthRock, MoveId::kGigaDrain,
         MoveId::kRecover, MoveId::kAncientPower},
        {Species::kAnorith, ItemId::kBerryJuice, Ability::kBattleArmor,
         Nature::kJolly, false, 0, 236, 36, 0, 0, 236, MoveId::kStealthRock,
         MoveId::kRapidSpin, MoveId::kRockBlast, MoveId::kKnockOff},
    });

static const TrainerSpec kRustboroCityYoungsterTommySpec(
    4, BattleFormat::kDouble, BattleBackground::kRockGymLeader2,
    BattleGround::kRockGymLeader2, BattlePlatform::kRockGymLeader,
    BattleEncounterAnimation::kGymLeaderRock, BattleWeather::kSandstorm,
    {
        {Species::kCranidos, ItemId::kLifeOrb, Ability::kSheerForce, Nature::kJolly,
         true, 0, 236, 36, 0, 0, 212, MoveId::kRockSlide, MoveId::kSuperpower,
         MoveId::kZenHeadbutt, MoveId::kCrunch},
        {Species::kShieldon, ItemId::kBerryJuice, Ability::kSturdy, Nature::kImpish,
         true, 116, 20, 212, 0, 132, 0, MoveId::kStealthRock, MoveId::kHeavySlam,
         MoveId::kRockBlast, MoveId::kEarthquake},
        {Species::kTirtouga, ItemId::kBerryJuice, Ability::kSturdy, Nature::kJolly,
         false, 0, 212, 92, 0, 0, 180, MoveId::kShellSmash, MoveId::kWaterfall,
         MoveId::kRockSlide, MoveId::kZenHeadbutt},
        {Species::kArchen, ItemId::kBerryJuice, Ability::kDefeatist, Nature::kJolly,
         false, 76, 20, 76, 0, 76, 196, MoveId::kStealthRock, MoveId::kRockSlide,
         MoveId::kAcrobatics, MoveId::kEarthquake},
    });

// This trainer forces a wild-style battle_type (see the original
// PatchTrainer_Rustboro_City_Schoolkid_Georgia, which set
// `config.battle_type = 0;` before calling `config.Set()`).
static const TrainerSpec kRustboroCitySchoolkidGeorgiaSpec(
    1, BattleFormat::kSingle, BattleBackground::kRockGymLeader2,
    BattleGround::kRockGymLeader2, BattlePlatform::kRockGymLeader,
    BattleEncounterAnimation::kRegirock, BattleWeather::kSandstorm,
    {
        {Species::kRegirock, ItemId::kLeftovers, Ability::kClearBody,
         Nature::kCareful, false, 252, 4, 0, 0, 252, 0, MoveId::kRockSlide,
         MoveId::kToxic, MoveId::kStealthRock, MoveId::kProtect},
    },
    /* force_wild_battle_type = */ true);

static const TrainerSpec kRustboroCityLeaderRoxanneSpec(
    3, BattleFormat::kRotation, BattleBackground::kRockGymLeader2,
    BattleGround::kRockGymLeader2, BattlePlatform::kRockGymLeader,
    BattleEncounterAnimation::kGymLeaderRock, BattleWeather::kSandstorm,
    {
        {Species::kAerodactyl, ItemId::kAerodactylite, Ability::kRockHead,
         Nature::kJolly, true, 0, 252, 0, 0, 4, 252, MoveId::kStoneEdge,
         MoveId::kAerialAce, MoveId::kEarthquake, MoveId::kPursuit},
        {Species::kAerodactyl, ItemId::kAerodactylite, Ability::kRockHead,
         Nature::kJolly, false, 0, 252, 0, 0, 4, 252, MoveId::kStoneEdge,
         MoveId::kAerialAce, MoveId::kEarthquake, MoveId::kPursuit},
        {Species::kAerodactyl, ItemId::kAerodactylite, Ability::kRockHead,
         Nature::kJolly, false, 0, 252, 0, 0, 4, 252, MoveId::kStoneEdge,
         MoveId::kAerialAce, MoveId::kEarthquake, MoveId::kPursuit},
    });

struct TrainerEntry {
  BattleTrainer id;
  const TrainerSpec* spec;
};

static const TrainerEntry TRAINERS[] = {
    {BattleTrainer::kRoute103May1, &kMaySpec},
    {BattleTrainer::kRoute103May2, &kMaySpec},
    {BattleTrainer::kRoute103May3, &kMaySpec},
    {BattleTrainer::kRoute102Kid1, &kRoute102Kid1Spec},
    {BattleTrainer::kRoute102Kid2, &kRoute102Kid2Spec},
    {BattleTrainer::kRoute102Kid3, &kRoute102Kid3Spec},
    {BattleTrainer::kRoute102Girl, &kRoute102GirlSpec},
    {BattleTrainer::kRoute104YoungsterBilly,
     &kRoute104YoungsterBillySpec},
    {BattleTrainer::kRoute104RichBoyWinston,
     &kRoute104RichBoyWinstonSpec},
    {BattleTrainer::kPetalburgWoodsBugCatcherLyle,
     &kPetalburgWoodsBugCatcherLyleSpec},
    {BattleTrainer::kPetalburgWoodsTeamAquaGrunt,
     &kPetalburgWoodsTeamAquaGruntSpec},
    {BattleTrainer::kPetalburgWoodsBugCatcherJames,
     &kPetalburgWoodsBugCatcherJamesSpec},
    {BattleTrainer::kRoute104LadyCindy, &kRoute104LadyCindySpec},
    {BattleTrainer::kRoute104LassHaley, &kRoute104LassHaleySpec},
    {BattleTrainer::kRoute104TwinsGinaAndMia,
     &kRoute104TwinsGinaAndMiaSpec},
    {BattleTrainer::kRoute104FishermanIvan, &kRoute104FishermanIvanSpec},

    {BattleTrainer::kRustboroCityYoungsterJosh,
     &kRustboroCityYoungsterJoshSpec},
    {BattleTrainer::kRustboroCityYoungsterTommy,
     &kRustboroCityYoungsterTommySpec},
    {BattleTrainer::kRustboroCitySchoolkidGeorgia,
     &kRustboroCitySchoolkidGeorgiaSpec},
    {BattleTrainer::kRustboroCityLeaderRoxanne,
     &kRustboroCityLeaderRoxanneSpec},
};

void PatchTrainer_AI(battle::Config& config) {
  // Best AI
  BattleAi flags;
  switch (static_cast<BattleFormat>(config.battle_format)) {
    case BattleFormat::kDouble:
    case BattleFormat::kTriple:
    case BattleFormat::kRotation:
      flags = BattleAi::kCasual | BattleAi::kCompetitive | BattleAi::kStrategist |
              BattleAi::kMulti;
      break;
    case BattleFormat::kHorde:
      flags = BattleAi::kCasual | BattleAi::kCompetitive | BattleAi::kStrategist |
              BattleAi::kHorde;
      break;
    case BattleFormat::kSingle:
    default:
      flags = BattleAi::kCasual | BattleAi::kCompetitive | BattleAi::kStrategist;
      break;
  }

  if (config.trainer_data[0] != nullptr)
    config.trainer_data[0]->ai_flags = static_cast<u32>(flags);
  if (config.trainer_data[1] != nullptr)
    config.trainer_data[1]->ai_flags = static_cast<u32>(flags);
  if (config.trainer_data[2] != nullptr)
    config.trainer_data[2]->ai_flags = static_cast<u32>(flags);
  if (config.trainer_data[3] != nullptr)
    config.trainer_data[3]->ai_flags = static_cast<u32>(flags);
}

u32 GetPokemonBST(PokemonCoreData* core) {
  global_data::Pokemon& data =
      global_data::Pokemon::GetInstance(core->species, core->form);
  u32 bst = data.base_hp + data.base_attack + data.base_defense +
            data.base_speed + data.base_special_attack +
            data.base_special_defense;
  return bst;
}

struct TeamStats {
  u8 max_level;
  f32 avg_level;
  u32 max_bst;
  f32 avg_bst;
};

TeamStats GetTeamStats(savedata::PokemonTeam* team) {
  TeamStats stats = {0, 0.0f, 0, 0.0f};
  u32 sum_level = 0, sum_bst = 0, count = 0;

  for (u32 i = 0; i < team->count; i++) {
    if (!team->pokemons[i]) continue;

    u8 level = team->pokemons[i]->runtime->level;
    u32 bst = GetPokemonBST(team->pokemons[i]->core);

    sum_level += level;
    sum_bst += bst;
    count++;

    if (level > stats.max_level) stats.max_level = level;
    if (bst > stats.max_bst) stats.max_bst = bst;
  }

  if (count > 0) {
    stats.avg_level = (f32)sum_level / count;
    stats.avg_bst = (f32)sum_bst / count;
  }

  return stats;
}

void PatchTrainer_Level(battle::Config& config,
                        f32 nerf_strength = 0.5f,
                        f32 team_weight = 0.5f) {
  savedata::PokemonTeam* my_team = config.pokemon_teams[0];
  savedata::PokemonTeam* enemy_team = config.pokemon_teams[1];
  if (!my_team || !enemy_team) {
    return;
  }

  TeamStats my_stats = GetTeamStats(my_team);
  u8 max_level_cap = savedata::PokemonTeam::GetInstance().GetMaxLevel();

  f32 ref_level = my_stats.avg_level +
                  (my_stats.max_level - my_stats.avg_level) * team_weight;
  f32 ref_bst = my_stats.avg_bst +
                (my_stats.max_bst - my_stats.avg_bst) * team_weight;

  for (u32 i = 0; i < enemy_team->count; i++) {
    savedata::PokemonParam* enemy = enemy_team->pokemons[i];
    if (!enemy) {
      continue;
    }
    if (enemy->core->experience != 0xFFFFFFFF) {
      continue;
    }

    u32 enemy_bst = GetPokemonBST(enemy->core);
    f32 bst_ratio = (ref_bst > 0.0f) ? (f32)enemy_bst / ref_bst : 1.0f;

    s8 rand_offset = -2 + Utils::GetRandomValue(2);
    f32 target_level = ref_level + rand_offset;

    if (bst_ratio > 1.0f) {
      f32 nerf_amount = (bst_ratio - 1.0f) * nerf_strength * target_level;
      target_level -= nerf_amount;
    }

    s32 clamped = (s32)(target_level + 0.5f);
    if (clamped < 1) clamped = 1;
    if (clamped > max_level_cap) clamped = max_level_cap;

    if (config.battle_format == static_cast<u8>(BattleFormat::kHorde)) {
      clamped >>= 1;
    }

    enemy->core->SetLevel((u8)clamped);
  }
}

void PatchTrainerData(battle::Config& config, u16& trainer_id) {
  ui::LogApplication::Print(u"[%u] %ls %ls wants to battle!", trainer_id,
                            config.trainer_data[1]->name->GetBuffer(),
                            config.trainer_data[1]->title_name->GetBuffer());

  // remove items
  for (u32 i = 0; i < 4; i++) {
    config.trainer_data[1]->items[i] = ItemId::kNone;
  }

  auto& battle = feature::Battle::GetInstance();

  battle.sync_team_hp = false;
  battle.inverse_stats = false;
  battle.metronome_only = false;

  // Useful to have 6 pokemon
  for (u32 i = 1; i < 6; i++) {
    *config.pokemon_teams[1]->pokemons[i]->core = *config.pokemon_teams[1]->
        pokemons[0]->core;
    *config.pokemon_teams[1]->pokemons[i]->runtime = *config.pokemon_teams[1]
        ->pokemons[0]->runtime;
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[0]->pokemons[i]->accessor->Decrypt();
    config.pokemon_teams[1]->pokemons[i]->accessor->Decrypt();
    config.pokemon_teams[1]->pokemons[i]->core->experience = 0xFFFFFFFF;
  }

  for (u32 i = 0; i < SIZE(TRAINERS); i++) {
    if (TRAINERS[i].id == static_cast<BattleTrainer>(trainer_id)) {
      ApplyTrainerSpec(config, *TRAINERS[i].spec);
      break;
    }
  }

  PatchTrainer_Level(config);
  PatchTrainer_AI(config);

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->core->ResetNickname();
    config.pokemon_teams[1]->pokemons[i]->accessor->Encrypt();
    config.pokemon_teams[0]->pokemons[i]->accessor->Encrypt();
  }

  for (u32 i = 0; i < 6; i++) {
    config.pokemon_teams[1]->pokemons[i]->UpdateRuntimeData();
  }

  config.pokemon_teams[1]->HealAllPokemons();

  switch (static_cast<BattleTrainer>(trainer_id)) {
    case BattleTrainer::kRustboroCityYoungsterJosh:
      config.InverseTypes();
      break;
    case BattleTrainer::kRustboroCityYoungsterTommy:
      SaveTeamBeforeBattle();
      config.InverseTeams();
      break;
    case BattleTrainer::kRustboroCitySchoolkidGeorgia:
      SaveTeamBeforeBattle();
      battle.metronome_only = true;
      break;
    case BattleTrainer::kRustboroCityLeaderRoxanne:
      battle.sync_team_hp = true;
      break;
    default:
      break;
  }
}
}
