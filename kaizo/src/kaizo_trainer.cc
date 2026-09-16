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

#include "core/utils.h"
#include "battle/patch/battle.h"
#include "battle/patch/trainer_team.h"
#include "battle/native/config.h"
#include "battle/constant/format.h"
#include "battle/constant/trainer.h"
#include "pokemon/constant/move.h"
#include "pokemon/constant/ability.h"
#include "battle/constant/ai.h"
#include "battle/constant/background.h"
#include "battle/constant/encounter_animation.h"
#include "battle/constant/ground.h"
#include "battle/constant/platform.h"
#include "battle/constant/weather.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/item.h"
#include "pokemon/constant/nature.h"
#include "pokemon/constant/species.h"
#include "pokemon/native/species_data.h"
#include "savedata/native/pokemon_team.h"
#include "ui/log_application.h"

namespace kaizo {
using TrainerOpponentSpec = battle::TrainerOpponent;
using TrainerSpec = battle::TrainerTeam;
using TrainerEntry = battle::TrainerTeamEntry;
using battle::Weather;

static u8 s_count = 0;
static pokemon::CoreData s_backup[6];

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

static const TrainerSpec kMaySpec(
    5, Format::kHorde, BackgroundId::kSkyPillarTop,
    GroundId::kSkyPillarTop, PlatformId::kSkyPillarTop,
    EncounterAnimationId::kRayquaza, Weather::kStrongWinds,
    {
        {SpeciesId::kWhismur, ItemId::kNone, AbilityId::kRattled, Nature::kModest, false,
         0, 0, 0, 0, 0, 0, MoveId::kNone, MoveId::kNone, MoveId::kNone,
         MoveId::kNone, FormId::kNormal, nullptr, 1},
        {SpeciesId::kWhismur, ItemId::kLifeOrb, AbilityId::kSoundproof, Nature::kModest,
         true, 244, 0, 12, 188, 0, 12, MoveId::kHyperVoice, MoveId::kFireBlast,
         MoveId::kShadowBall, MoveId::kExtrasensory, FormId::kNormal, u"Hash"},
        {SpeciesId::kWhismur, ItemId::kNone, AbilityId::kRattled, Nature::kModest, false,
         0, 0, 0, 0, 0, 0, MoveId::kNone, MoveId::kNone, MoveId::kNone,
         MoveId::kNone, FormId::kNormal, nullptr, 1},
        {SpeciesId::kWhismur, ItemId::kNone, AbilityId::kRattled, Nature::kModest, false,
         0, 0, 0, 0, 0, 0, MoveId::kNone, MoveId::kNone, MoveId::kNone,
         MoveId::kNone, FormId::kNormal, nullptr, 1},
        {SpeciesId::kWhismur, ItemId::kNone, AbilityId::kRattled, Nature::kModest, false,
         0, 0, 0, 0, 0, 0, MoveId::kNone, MoveId::kNone, MoveId::kNone,
         MoveId::kNone, FormId::kNormal, nullptr, 1},
    });

static const TrainerSpec kRoute102Kid1Spec(
    2, Format::kDouble, BackgroundId::kAquaBoss,
    GroundId::kAquaBoss, PlatformId::kWater,
    EncounterAnimationId::kKyogre, Weather::kHeavyRain,
    {
        {SpeciesId::kFurfrou, ItemId::kLeftovers, AbilityId::kFurCoat, Nature::kJolly,
         true, 0, 252, 0, 0, 4, 252, MoveId::kUTurn, MoveId::kThunderWave,
         MoveId::kReturn, MoveId::kSuckerPunch,
         FormId::kFurfrouHeart},
        {SpeciesId::kPoochyena, ItemId::kToxicOrb, AbilityId::kQuickFeet, Nature::kJolly,
         true, 0, 236, 0, 0, 36, 236, MoveId::kCrunch, MoveId::kPlayRough,
         MoveId::kFacade, MoveId::kFireFang},
    });

static const TrainerSpec kRoute102Kid2Spec(
    3, Format::kTriple, BackgroundId::kMagmaBoss,
    GroundId::kMagmaBoss, PlatformId::kVolcano,
    EncounterAnimationId::kGroudon, Weather::kExtremelyHarshSunlight,
    {
        {SpeciesId::kAzurill, ItemId::kEviolite, AbilityId::kHugePower, Nature::kAdamant,
         false, 116, 196, 116, 0, 36, 0, MoveId::kReturn, MoveId::kKnockOff,
         MoveId::kWaterfall, MoveId::kIronTail},
        {SpeciesId::kPichu, ItemId::kEviolite, AbilityId::kLightningRod, Nature::kTimid,
         true, 36, 0, 0, 236, 0, 196, MoveId::kNastyPlot, MoveId::kSubstitute,
         MoveId::kThunderbolt, MoveId::kHiddenPower},
        {SpeciesId::kRattata, ItemId::kFlameOrb, AbilityId::kGuts, Nature::kJolly, false,
         0, 228, 76, 0, 0, 180, MoveId::kFacade, MoveId::kSuckerPunch,
         MoveId::kFlameWheel, MoveId::kUTurn},
    });

static const TrainerSpec kRoute102Kid3Spec(
    2, Format::kSingle, BackgroundId::kDeoxys, GroundId::kDeoxys,
    PlatformId::kFlyingGymLeader, EncounterAnimationId::kDeoxys,
    Weather::kHail,
    {
        {SpeciesId::kPurrloin, ItemId::kEviolite, AbilityId::kPrankster, Nature::kCareful,
         false, 188, 0, 180, 0, 60, 68, MoveId::kEncore, MoveId::kUTurn,
         MoveId::kKnockOff, MoveId::kThunderWave},
        {SpeciesId::kGlameow, ItemId::kLifeOrb, AbilityId::kLimber, Nature::kJolly, false,
         0, 236, 20, 0, 0, 236, MoveId::kReturn, MoveId::kKnockOff, MoveId::kFakeOut,
         MoveId::kUTurn},
    });

static const TrainerSpec kRoute102GirlSpec(
    3, Format::kRotation, BackgroundId::kAbandonedShip,
    GroundId::kAbandonedShip, PlatformId::kShip,
    EncounterAnimationId::kHoopa, Weather::kNone,
    {
        {SpeciesId::kRaikou, ItemId::kChoiceSpecs, AbilityId::kPressure, Nature::kTimid,
         false, 0, 0, 0, 252, 4, 252, MoveId::kVoltSwitch, MoveId::kThunderbolt,
         MoveId::kShadowBall, MoveId::kHiddenPower},
        {SpeciesId::kEntei, ItemId::kChoiceBand, AbilityId::kPressure, Nature::kAdamant,
         true, 0, 252, 0, 0, 4, 252, MoveId::kSacredFire, MoveId::kFlareBlitz,
         MoveId::kExtremeSpeed, MoveId::kStoneEdge},
        {SpeciesId::kSuicune, ItemId::kLeftovers, AbilityId::kPressure, Nature::kTimid,
         false, 252, 0, 0, 0, 40, 216, MoveId::kSubstitute, MoveId::kProtect,
         MoveId::kCalmMind, MoveId::kScald},
    });

static const TrainerSpec kRoute104YoungsterBillySpec(
    2, Format::kSingle, BackgroundId::kSkyBattle,
    GroundId::kSkyBattle, PlatformId::kSky,
    EncounterAnimationId::kGymLeaderFlying, Weather::kInvalid,
    {
        {SpeciesId::kXatu, ItemId::kRockyHelmet, AbilityId::kMagicBounce, Nature::kTimid,
         true, 252, 0, 240, 0, 0, 16, MoveId::kPsychic, MoveId::kRoost,
         MoveId::kGrassKnot, MoveId::kUTurn},
        {SpeciesId::kTaillow, ItemId::kLifeOrb, AbilityId::kScrappy, Nature::kNaive,
         false, 0, 0, 36, 196, 36, 236, MoveId::kBoomburst, MoveId::kHeatWave,
         MoveId::kHiddenPower, MoveId::kBraveBird},
    });

static const TrainerSpec kRoute104RichBoyWinstonSpec(
    2, Format::kDouble, BackgroundId::kSkyBattle,
    GroundId::kSkyBattle, PlatformId::kSky,
    EncounterAnimationId::kGymLeaderFlying, Weather::kInvalid,
    {
        {SpeciesId::kPidove, ItemId::kLifeOrb, AbilityId::kSuperLuck, Nature::kNaive,
         false, 0, 236, 0, 0, 0, 252, MoveId::kTailwind, MoveId::kFacade,
         MoveId::kAerialAce, MoveId::kHeatWave},
        {SpeciesId::kDucklett, ItemId::kEviolite, AbilityId::kHydration, Nature::kCalm,
         true, 180, 0, 196, 4, 116, 0, MoveId::kAirSlash, MoveId::kDefog,
         MoveId::kRoost, MoveId::kScald},
    });

static const TrainerSpec kPetalburgWoodsBugCatcherLyleSpec(
    4, Format::kDouble, BackgroundId::kForest, GroundId::kForest,
    PlatformId::kForest, EncounterAnimationId::kRayquaza,
    Weather::kInvalid,
    {
        {SpeciesId::kMetapod, ItemId::kEviolite, AbilityId::kShedSkin, Nature::kNaughty,
         false, 248, 252, 0, 8, 0, 0, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kIronDefense, MoveId::kTackle},
        {SpeciesId::kKakuna, ItemId::kEviolite, AbilityId::kShedSkin, Nature::kNaughty,
         true, 248, 252, 0, 8, 0, 0, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kIronDefense, MoveId::kPoisonSting},
        {SpeciesId::kSilcoon, ItemId::kEviolite, AbilityId::kShedSkin, Nature::kNaughty,
         false, 248, 252, 0, 8, 0, 0, MoveId::kElectroweb, MoveId::kIronDefense,
         MoveId::kBugBite, MoveId::kPoisonSting},
        {SpeciesId::kCascoon, ItemId::kEviolite, AbilityId::kShedSkin, Nature::kNaughty,
         true, 248, 252, 0, 8, 0, 0, MoveId::kElectroweb, MoveId::kIronDefense,
         MoveId::kBugBite, MoveId::kPoisonSting},
    });

static const TrainerSpec kPetalburgWoodsTeamAquaGruntSpec(
    5, Format::kHorde, BackgroundId::kForest, GroundId::kForest,
    PlatformId::kForest, EncounterAnimationId::kRayquaza,
    Weather::kInvalid,
    {
        {SpeciesId::kScatterbug, ItemId::kEviolite, AbilityId::kCompoundEyes,
         Nature::kCareful, true, 132, 76, 116, 0, 156, 0, MoveId::kStunSpore,
         MoveId::kPoisonPowder, MoveId::kBugBite, MoveId::kTackle},
        {SpeciesId::kSewaddle, ItemId::kEviolite, AbilityId::kChlorophyll, Nature::kBold,
         true, 156, 0, 196, 40, 116, 0, MoveId::kStickyWeb, MoveId::kGigaDrain,
         MoveId::kHiddenPower, MoveId::kAirSlash},
        {SpeciesId::kWurmple, ItemId::kEviolite, AbilityId::kShieldDust, Nature::kRelaxed,
         true, 156, 76, 76, 0, 196, 0, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kPoisonSting, MoveId::kStringShot},
        {SpeciesId::kWeedle, ItemId::kEviolite, AbilityId::kShieldDust, Nature::kAdamant,
         true, 36, 236, 36, 36, 116, 36, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kPoisonSting, MoveId::kStringShot},
        {SpeciesId::kCaterpie, ItemId::kEviolite, AbilityId::kShieldDust, Nature::kRelaxed,
         true, 156, 0, 236, 0, 116, 0, MoveId::kBugBite, MoveId::kElectroweb,
         MoveId::kStringShot, MoveId::kTackle},
    });

static const TrainerSpec kPetalburgWoodsBugCatcherJamesSpec(
    2, Format::kSingle, BackgroundId::kForest, GroundId::kForest,
    PlatformId::kForest, EncounterAnimationId::kRayquaza,
    Weather::kInvalid,
    {
        {SpeciesId::kButterfree, ItemId::kLifeOrb, AbilityId::kTintedLens,
         Nature::kTimid, false, 0, 0, 4, 252, 0, 252, MoveId::kSleepPowder,
         MoveId::kQuiverDance, MoveId::kBugBuzz, MoveId::kEnergyBall},
        {SpeciesId::kBeedrill, ItemId::kBeedrillite, AbilityId::kSwarm, Nature::kJolly,
         false, 0, 252, 4, 0, 0, 252, MoveId::kUTurn, MoveId::kPoisonJab,
         MoveId::kDrillRun, MoveId::kKnockOff},
    });

static const TrainerSpec kRoute104LadyCindySpec(
    1, Format::kSingle, BackgroundId::kShore, GroundId::kShore,
    PlatformId::kShore, EncounterAnimationId::kKyogre, Weather::kRain,
    {
        {SpeciesId::kTotodile, ItemId::kEviolite, AbilityId::kSheerForce,
         Nature::kAdamant, false, 0, 236, 4, 0, 0, 252, MoveId::kDragonDance,
         MoveId::kWaterfall, MoveId::kIcePunch, MoveId::kSuperpower},
    });

static const TrainerSpec kRoute104LassHaleySpec(
    2, Format::kSingle, BackgroundId::kShore, GroundId::kShore,
    PlatformId::kShore, EncounterAnimationId::kKyogre, Weather::kRain,
    {
        {SpeciesId::kPiplup, ItemId::kEviolite, AbilityId::kTorrent, Nature::kBold, true,
         92, 0, 252, 0, 148, 0, MoveId::kStealthRock, MoveId::kDefog, MoveId::kScald,
         MoveId::kIceBeam},
        {SpeciesId::kOshawott, ItemId::kLifeOrb, AbilityId::kTorrent, Nature::kRash, true,
         0, 0, 0, 252, 0, 236, MoveId::kHydroPump, MoveId::kIceBeam, MoveId::kAirSlash,
         MoveId::kAquaJet},
    });

static const TrainerSpec kRoute104TwinsGinaAndMiaSpec(
    2, Format::kDouble, BackgroundId::kShore, GroundId::kShore,
    PlatformId::kShore, EncounterAnimationId::kKyogre,
    Weather::kHeavyRain,
    {
        {SpeciesId::kBlastoise, ItemId::kBlastoisinite, AbilityId::kRainDish,
         Nature::kModest, false, 248, 0, 0, 252, 8, 0, MoveId::kRapidSpin,
         MoveId::kWaterPulse, MoveId::kDarkPulse, MoveId::kAuraSphere},
        {SpeciesId::kSwampert, ItemId::kSwampertite, AbilityId::kDamp, Nature::kAdamant,
         false, 0, 252, 0, 0, 4, 252, MoveId::kWaterfall, MoveId::kEarthquake,
         MoveId::kIcePunch, MoveId::kPowerUpPunch},
    });

static const TrainerSpec kRoute104FishermanIvanSpec(
    5, Format::kHorde, BackgroundId::kShore, GroundId::kShore,
    PlatformId::kShore, EncounterAnimationId::kKyogre,
    Weather::kHeavyRain,
    {
        {SpeciesId::kMagikarp, ItemId::kNone, AbilityId::kRattled, Nature::kJolly, false,
         0, 196, 0, 0, 116, 196, MoveId::kSplash, MoveId::kTackle, MoveId::kFlail,
         MoveId::kBounce, FormId::kNormal, nullptr, 1},
        {SpeciesId::kMagikarp, ItemId::kNone, AbilityId::kRattled, Nature::kJolly, false,
         0, 196, 0, 0, 116, 196, MoveId::kSplash, MoveId::kTackle, MoveId::kFlail,
         MoveId::kBounce, FormId::kNormal, nullptr, 1},
        {SpeciesId::kGyarados, ItemId::kGyaradosite, AbilityId::kIntimidate,
         Nature::kJolly, true, 0, 252, 4, 0, 0, 252, MoveId::kDragonDance,
         MoveId::kCrunch, MoveId::kWaterfall, MoveId::kEarthquake},
        {SpeciesId::kMagikarp, ItemId::kNone, AbilityId::kRattled, Nature::kJolly, false,
         0, 196, 0, 0, 116, 196, MoveId::kSplash, MoveId::kTackle, MoveId::kFlail,
         MoveId::kBounce, FormId::kNormal, nullptr, 1},
        {SpeciesId::kMagikarp, ItemId::kNone, AbilityId::kRattled, Nature::kJolly, false,
         0, 196, 0, 0, 116, 196, MoveId::kSplash, MoveId::kTackle, MoveId::kFlail,
         MoveId::kBounce, FormId::kNormal, nullptr, 1},
    });

static const TrainerSpec kRustboroCityYoungsterJoshSpec(
    4, Format::kDouble, BackgroundId::kRockGymLeader2,
    GroundId::kRockGymLeader2, PlatformId::kRockGymLeader,
    EncounterAnimationId::kGymLeaderRock, Weather::kSandstorm,
    {
        {SpeciesId::kOmanyte, ItemId::kEviolite, AbilityId::kShellArmor, Nature::kModest,
         false, 76, 0, 0, 196, 0, 236, MoveId::kShellSmash, MoveId::kHydroPump,
         MoveId::kIceBeam, MoveId::kEarthPower},
        {SpeciesId::kKabuto, ItemId::kEviolite, AbilityId::kWeakArmor, Nature::kAdamant,
         false, 116, 196, 196, 0, 0, 0, MoveId::kRapidSpin, MoveId::kRockSlide,
         MoveId::kKnockOff, MoveId::kWaterfall},
        {SpeciesId::kLileep, ItemId::kEviolite, AbilityId::kStormDrain, Nature::kCalm,
         false, 228, 0, 140, 0, 140, 0, MoveId::kStealthRock, MoveId::kGigaDrain,
         MoveId::kRecover, MoveId::kAncientPower},
        {SpeciesId::kAnorith, ItemId::kBerryJuice, AbilityId::kBattleArmor,
         Nature::kJolly, false, 0, 236, 36, 0, 0, 236, MoveId::kStealthRock,
         MoveId::kRapidSpin, MoveId::kRockBlast, MoveId::kKnockOff},
    });

static const TrainerSpec kRustboroCityYoungsterTommySpec(
    4, Format::kDouble, BackgroundId::kRockGymLeader2,
    GroundId::kRockGymLeader2, PlatformId::kRockGymLeader,
    EncounterAnimationId::kGymLeaderRock, Weather::kSandstorm,
    {
        {SpeciesId::kCranidos, ItemId::kLifeOrb, AbilityId::kSheerForce, Nature::kJolly,
         true, 0, 236, 36, 0, 0, 212, MoveId::kRockSlide, MoveId::kSuperpower,
         MoveId::kZenHeadbutt, MoveId::kCrunch},
        {SpeciesId::kShieldon, ItemId::kBerryJuice, AbilityId::kSturdy, Nature::kImpish,
         true, 116, 20, 212, 0, 132, 0, MoveId::kStealthRock, MoveId::kHeavySlam,
         MoveId::kRockBlast, MoveId::kEarthquake},
        {SpeciesId::kTirtouga, ItemId::kBerryJuice, AbilityId::kSturdy, Nature::kJolly,
         false, 0, 212, 92, 0, 0, 180, MoveId::kShellSmash, MoveId::kWaterfall,
         MoveId::kRockSlide, MoveId::kZenHeadbutt},
        {SpeciesId::kArchen, ItemId::kBerryJuice, AbilityId::kDefeatist, Nature::kJolly,
         false, 76, 20, 76, 0, 76, 196, MoveId::kStealthRock, MoveId::kRockSlide,
         MoveId::kAcrobatics, MoveId::kEarthquake},
    });

static const TrainerSpec kRustboroCitySchoolkidGeorgiaSpec(
    1, Format::kSingle, BackgroundId::kRockGymLeader2,
    GroundId::kRockGymLeader2, PlatformId::kRockGymLeader,
    EncounterAnimationId::kRegirock, Weather::kSandstorm,
    {
        {SpeciesId::kRegirock, ItemId::kLeftovers, AbilityId::kClearBody,
         Nature::kCareful, false, 252, 4, 0, 0, 252, 0, MoveId::kRockSlide,
         MoveId::kToxic, MoveId::kStealthRock, MoveId::kProtect},
    },
    /* force_wild_battle_type = */ true);

static const TrainerSpec kRustboroCityLeaderRoxanneSpec(
    3, Format::kRotation, BackgroundId::kRockGymLeader2,
    GroundId::kRockGymLeader2, PlatformId::kRockGymLeader,
    EncounterAnimationId::kGymLeaderRock, Weather::kSandstorm,
    {
        {SpeciesId::kAerodactyl, ItemId::kAerodactylite, AbilityId::kRockHead,
         Nature::kJolly, true, 0, 252, 0, 0, 4, 252, MoveId::kStoneEdge,
         MoveId::kAerialAce, MoveId::kEarthquake, MoveId::kPursuit},
        {SpeciesId::kAerodactyl, ItemId::kAerodactylite, AbilityId::kRockHead,
         Nature::kJolly, false, 0, 252, 0, 0, 4, 252, MoveId::kStoneEdge,
         MoveId::kAerialAce, MoveId::kEarthquake, MoveId::kPursuit},
        {SpeciesId::kAerodactyl, ItemId::kAerodactylite, AbilityId::kRockHead,
         Nature::kJolly, false, 0, 252, 0, 0, 4, 252, MoveId::kStoneEdge,
         MoveId::kAerialAce, MoveId::kEarthquake, MoveId::kPursuit},
    });

static const TrainerEntry TRAINERS[] = {
    {TrainerId::kRoute103May1, &kMaySpec},
    {TrainerId::kRoute103May2, &kMaySpec},
    {TrainerId::kRoute103May3, &kMaySpec},
    {TrainerId::kRoute102Kid1, &kRoute102Kid1Spec},
    {TrainerId::kRoute102Kid2, &kRoute102Kid2Spec},
    {TrainerId::kRoute102Kid3, &kRoute102Kid3Spec},
    {TrainerId::kRoute102Girl, &kRoute102GirlSpec},
    {TrainerId::kRoute104YoungsterBilly,
     &kRoute104YoungsterBillySpec},
    {TrainerId::kRoute104RichBoyWinston,
     &kRoute104RichBoyWinstonSpec},
    {TrainerId::kPetalburgWoodsBugCatcherLyle,
     &kPetalburgWoodsBugCatcherLyleSpec},
    {TrainerId::kPetalburgWoodsTeamAquaGrunt,
     &kPetalburgWoodsTeamAquaGruntSpec},
    {TrainerId::kPetalburgWoodsBugCatcherJames,
     &kPetalburgWoodsBugCatcherJamesSpec},
    {TrainerId::kRoute104LadyCindy, &kRoute104LadyCindySpec},
    {TrainerId::kRoute104LassHaley, &kRoute104LassHaleySpec},
    {TrainerId::kRoute104TwinsGinaAndMia,
     &kRoute104TwinsGinaAndMiaSpec},
    {TrainerId::kRoute104FishermanIvan, &kRoute104FishermanIvanSpec},

    {TrainerId::kRustboroCityYoungsterJosh,
     &kRustboroCityYoungsterJoshSpec},
    {TrainerId::kRustboroCityYoungsterTommy,
     &kRustboroCityYoungsterTommySpec},
    {TrainerId::kRustboroCitySchoolkidGeorgia,
     &kRustboroCitySchoolkidGeorgiaSpec},
    {TrainerId::kRustboroCityLeaderRoxanne,
     &kRustboroCityLeaderRoxanneSpec},
};

void PatchTrainer_AI(battle::Config& config) {
  // Best AI
  AiFlags flags;
  switch (config.format) {
    case Format::kDouble:
    case Format::kTriple:
    case Format::kRotation:
      flags = AiFlags::kCasual | AiFlags::kCompetitive | AiFlags::kStrategist |
              AiFlags::kMulti;
      break;
    case Format::kHorde:
      flags = AiFlags::kCasual | AiFlags::kCompetitive | AiFlags::kStrategist |
              AiFlags::kHorde;
      break;
    case Format::kSingle:
    default:
      flags = AiFlags::kCasual | AiFlags::kCompetitive | AiFlags::kStrategist;
      break;
  }

  if (config.trainer_data[0] != nullptr)
    config.trainer_data[0]->ai_flags = flags;
  if (config.trainer_data[1] != nullptr)
    config.trainer_data[1]->ai_flags = flags;
  if (config.trainer_data[2] != nullptr)
    config.trainer_data[2]->ai_flags = flags;
  if (config.trainer_data[3] != nullptr)
    config.trainer_data[3]->ai_flags = flags;
}

u32 GetPokemonBST(pokemon::CoreData* core) {
  pokemon::SpeciesData& data =
      pokemon::SpeciesData::GetInstance(core->species, core->form);
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

    s8 rand_offset = -2 + core::Utils::GetRandomValue(2);
    f32 target_level = ref_level + rand_offset;

    if (bst_ratio > 1.0f) {
      f32 nerf_amount = (bst_ratio - 1.0f) * nerf_strength * target_level;
      target_level -= nerf_amount;
    }

    s32 clamped = (s32)(target_level + 0.5f);
    if (clamped < 1) clamped = 1;
    if (clamped > max_level_cap) clamped = max_level_cap;

    if (config.format == Format::kHorde) {
      clamped >>= 1;
    }

    enemy->core->SetLevel((u8)clamped);
  }
}

void PatchTrainerData(battle::Config& config, TrainerId& trainer_id) {
  ui::LogApplication::Print(u"[%u] %ls %ls wants to battle!", trainer_id,
                            config.trainer_data[1]->name->GetBuffer(),
                            config.trainer_data[1]->title_name->GetBuffer());

  // remove items
  for (u32 i = 0; i < 4; i++) {
    config.trainer_data[1]->items[i] = ItemId::kNone;
  }

  auto& battle = battle::Battle::GetInstance();

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

  const battle::TrainerTeam* team = battle::TrainerTeams::Find(trainer_id);
  if (team != nullptr) team->ApplyTo(config);

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

  switch (trainer_id) {
    case TrainerId::kRustboroCityYoungsterJosh:
      config.InverseTypes();
      break;
    case TrainerId::kRustboroCityYoungsterTommy:
      SaveTeamBeforeBattle();
      config.InverseTeams();
      break;
    case TrainerId::kRustboroCitySchoolkidGeorgia:
      SaveTeamBeforeBattle();
      battle.metronome_only = true;
      break;
    case TrainerId::kRustboroCityLeaderRoxanne:
      battle.sync_team_hp = true;
      break;
    default:
      break;
  }
}

void InitializeTrainerTeams() {
  battle::TrainerTeams::SetTable(TRAINERS, SIZE(TRAINERS));
}
}
