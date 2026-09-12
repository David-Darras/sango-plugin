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

#pragma once

#include <types.h>

enum class ScriptId : u16 {
  kNone = 0,
  kDoNothing = 2000,

  /* ---------------------------------------------------------
  POKEMON CENTER
  --------------------------------------------------------- */
  kNurse = 2100,
  kNurseOnYourBirthday = 2101,
  kNurseAfterBlackout = 2102,
  kPokemonLeagueNurse = 2103,
  kPokemonLeagueNurseAfterBlackout = 2104,
  kStorageBoxPc = 2001,
  kHoopaGiftCheck = 10094,
  kHoopaGift = 10095,
  kVolcanionGift = 10097,
  kUnknown10090 = 10090,

  /* ---------------------------------------------------------
  SAVING AND RECEIVING POKEMON
  --------------------------------------------------------- */
  kSaveGame = 2003,
  kSaveGameAutomatically = 2004,
  kSaveGameAutomatically2 = 2005,
  kReceivePokemon = 2002,
  kBlackOutAtHome = 2006,
  kReceiveRibbon = 2800,
  kEggHatchingMessage = 2250,

  /* ---------------------------------------------------------
  SHOP COUNTERS
  --------------------------------------------------------- */
  kShopCounter = 10117,
  kShopCounterWithoutPokeballs = 10118,
  kBattlePointShopCounter1 = 10110,
  kBattlePointShopCounter2 = 10111,
  kBattlePointMoveTutor1 = 10112,
  kBattlePointMoveTutor2 = 10113,
  kBattlePointMoveTutor3 = 10114,
  kBattlePointMoveTutor4 = 10115,
  kIncenseShop = 10119,
  kVitaminShop = 10120,
  kTechnicalMachineShop = 10121,
  kHerbalMedicineShop = 10122,
  kDepartmentStoreCounter1 = 10123,
  kDepartmentStoreCounter2 = 10124,
  kDepartmentStoreCounter3 = 10125,
  kDepartmentStoreCounter4 = 10126,
  kDepartmentStoreCounter5 = 10127,
  kFlowerShop = 10130,
  kSouvenirShop = 10131,
  kDollShop = 10132,
  kChairShop = 10133,
  kPrankItemShop = 10134,
  kSecretBaseDollShop = 10135,
  kSecretBasePosterShop = 10136,
  kSecretBaseMatShop = 10137,
  kSecretBaseCushionShop = 10138,
  kBargainShop = 10139,
  kUnknown10116 = 10116,
  kUnknown10128 = 10128,
  kUnknown10129 = 10129,

  /* ---------------------------------------------------------
  VENDING MACHINES
  --------------------------------------------------------- */
  kVendingMachine = 10480,
  kVendingMachineCheap = 10481,
  kVendingMachineExpensive = 10482,
  kWaterVendingMachine = 10483,
  kSodaVendingMachine = 10484,

  /* ---------------------------------------------------------
  FIELD MOVES
  Each move has a script for the obstacle and one for what happens once the
  move has played out.
  --------------------------------------------------------- */
  kRockSmash = 10050,
  kRockSmashDone = 10051,
  kCut = 10052,
  kCutDone = 10053,
  kStrength = 10054,
  kStrengthDone = 10055,
  kSmashWall = 10056,
  kSmashWallDone = 10057,
  kDig = 10058,
  kEscapeRope = 10059,
  kTeleport = 10060,
  kFlyLeave = 10061,
  kFlyArrive = 10062,
  kSurf = 10063,
  kSurfDone = 10064,
  kWaterfall = 10065,
  kWaterfallDone = 10066,
  kWaterfallDoneWithoutAnimation = 10067,
  kDive = 10068,
  kDiveFailed = 10069,
  kFlashDone = 10070,
  kAskToSurf = 10071,
  kAskToUseWaterfall = 10072,
  kAskToDive = 10073,
  kSecretPowerDone = 10074,
  kAskToUseSecretPower = 10075,
  kSecretPowerUnusable = 10076,
  kSecretPowerDeliveryOnly = 10077,

  /* ---------------------------------------------------------
  ITEMS ON THE GROUND
  --------------------------------------------------------- */
  kMegaStonePickup = 10250,
  kHiddenItemPickup = 7999,
  kBerryTree = 10540,
  kFossilRestoration = 10280,

  /* ---------------------------------------------------------
  ITEM USED FROM THE BAG
  --------------------------------------------------------- */
  kItemCannotBeUsedHere = 10845,
  kExpShareOn = 10846,
  kExpShareOff = 10847,
  kEonFlute = 10848,
  kRepelWoreOff = 10160,
  kUseAnotherRepel = 10161,
  kSweetScentFailedWeather = 10164,
  kSweetScentFoundNothing = 10165,

  /* ---------------------------------------------------------
  MOVE TUTORS AND MOVE MANAGEMENT
  --------------------------------------------------------- */
  kMoveDeleter = 2260,
  kMoveReminder = 2261,
  kDracoMeteorTutor = 2263,
  kStarterUltimateMoveTutor = 2264,
  kDragonAscentTutor = 2265,
  kMythicalPokemonMoveTutor = 2266,
  kUnknown2262 = 2262,

  /* ---------------------------------------------------------
  DAY CARE
  --------------------------------------------------------- */
  kDayCareGrandmother = 2200,
  kDayCareGrandfather = 2201,
  kDayCareGirl = 2202,
  kDayCareBoy = 2203,

  /* ---------------------------------------------------------
  POKEMON APPRAISAL
  --------------------------------------------------------- */
  kPokedexRating = 10380,
  kPokedexRatingFirstTime = 10381,
  kStatJudge = 10310,
  kHiddenPowerCheck = 10780,
  kPokemonMemories = 10640,
  kNameRater = 2225,

  /* ---------------------------------------------------------
  FORM CHANGES
  --------------------------------------------------------- */
  kRotomFormChange = 10315,
  kDeoxysFormChange = 10270,
  kCosplayPikachuTalk = 10316,
  kCosplayPikachuChange = 10317,
  kCosplayPikachuChangeFromChest = 10318,
  kCosplayPikachuReceive = 10319,

  /* ---------------------------------------------------------
  CLOTHING BOUTIQUE
  --------------------------------------------------------- */
  kClothingShopkeeper = 10709,
  kClothingShopkeeperFemale = 10700,
  kClothingShopkeeperMale = 10701,
  kFittingRoom = 10707,
  kFittingRoomInPokemonCenter = 10706,
  kFittingRoomPromotion = 10712,
  kClothingMannequin = 10708,
  kClothingShopExit = 10710,
  kClothingShopExitFemale = 10702,
  kClothingShopExitMale = 10703,
  kClothingShopTutorial = 10711,
  kClothingShopTutorialFemale = 10704,
  kClothingShopTutorialMale = 10705,

  /* ---------------------------------------------------------
  CONTESTS
  --------------------------------------------------------- */
  kContestSingle = 10520,
  kContestMultiplayer = 10521,
  kContestPhotoBackground = 10522,
  kContestPhotoViewer = 10539,
  kContestFan1 = 10523,
  kContestFan2 = 10524,
  kContestFan3 = 10525,
  kContestFan4 = 10526,
  kContestFan5 = 10527,
  kContestFan6 = 10528,
  kContestFan7 = 10529,
  kContestFan8 = 10530,
  kContestFan9 = 10531,
  kContestFan10 = 10532,
  kContestPortraitCool = 10534,
  kContestPortraitBeauty = 10535,
  kContestPortraitCute = 10536,
  kContestPortraitSmart = 10537,
  kContestPortraitTough = 10538,
  kContestFlagResetOnExit = 10533,

  /* ---------------------------------------------------------
  SPECIAL BATTLE RULES
  --------------------------------------------------------- */
  kInverseBattle = 10425,
  kSkyBattle1 = 10350,
  kSkyBattle2 = 10351,
  kSkyBattle3 = 10352,

  /* ---------------------------------------------------------
  SECRET BASES
  --------------------------------------------------------- */
  kSecretBaseExit = 10865,
  kSecretBaseInvalidExit = 10868,
  kSecretBasePc = 10866,
  kSecretBaseFlag = 10867,
  kSecretBaseTutorial = 10510,
  kSecretBaseCaretaker = 10880,
  kSecretBaseFriend1 = 10881,
  kSecretBaseFriend2 = 10882,
  kSecretBaseFriend3 = 10883,
  kSecretBaseFriend4 = 10884,
  kSecretBaseFriend5 = 10885,

  /* ---------------------------------------------------------
  SECRET BASE DECORATIONS
  --------------------------------------------------------- */
  kSandOrnament = 10900,
  kCandleStand = 10901,
  kStrawTrainingDummy = 10902,
  kBlackboard = 10903,
  kColourfulLight = 10904,
  kShabbyLight = 10905,
  kTelevisionDecoration = 10906,
  kPaperFlowerBall = 10907,
  kGymStatue = 10912,
  kGlassOrnament = 10913,
  kJukebox = 10914,
  kNoticeBoard = 10915,
  kPunchingDummy = 10917,
  kKecleonDoll = 10918,
  kGorgeousStand = 10919,
  kSymbolOrnament = 10920,
  kOrnament = 10921,
  kScenicOrnament = 10925,
  kFakePc = 10926,
  kPokemonDoll = 10927,
  kLevelBreaker = 10928,
  kPokeballPoster = 10929,
  kGreenPoster = 10930,
  kRedPoster = 10931,
  kLightBluePoster = 10932,
  kCutePoster = 10933,
  kSparklyPoster = 10934,
  kLongPoster = 10935,
  kSeaPoster = 10936,
  kSkyPoster = 10937,
  kKissPoster = 10938,
  kRedWallScroll = 10939,
  kBlueWallScroll = 10940,
  kGreenWallScroll = 10941,
  kFatherWallScroll = 10942,
  kHoleMat = 10908,
  kStartMat = 10909,
  kSpinPanel = 10910,
  kWarpPanel = 10911,
  kJumpMat = 10922,
  kRotateMat = 10923,
  kTrackMat = 10924,
  kUnknown10916 = 10916,

  /* ---------------------------------------------------------
  SCENERY YOU CAN EXAMINE
  --------------------------------------------------------- */
  kTrashCan = 2500,
  kShelf = 2501,
  kBookshelf = 2502,
  kCupboard = 2503,
  kMerchandiseShelf = 2504,
  kNarrowShelf = 2505,
  kNarrowBookshelf = 2506,
  kVase = 2507,
  kTelevision = 10650,

  /* ---------------------------------------------------------
  LADDERS
  --------------------------------------------------------- */
  kLadderUpShort = 10501,
  kLadderUpTall = 10502,
  kLadderDownShort = 10503,
  kLadderDownTall = 10504,

  /* ---------------------------------------------------------
  FISHING
  --------------------------------------------------------- */
  kFishingQuickBite = 10166,
  kFishingSlowBite = 10167,
  kFishingGotAway1 = 10168,
  kFishingGotAway2 = 10169,
  kFishingGotAway3 = 10170,

  /* ---------------------------------------------------------
  RESTAURANTS AND JUICE STANDS
  --------------------------------------------------------- */
  kRestaurant = 10740,
  kRestaurantSoignez = 10741,
  kJuiceStandFresh = 10435,
  kJuiceStandVintage = 10436,

  /* ---------------------------------------------------------
  MISCELLANEOUS
  --------------------------------------------------------- */
  kPostman = 10400,
  kGlobalLinkPostman = 10390,
  kOPower = 10840,
  kOPowerExpired = 10163,
  kHoloCasterMail = 10162,
  kAdventureGuide = 10171,
  kCannotRideBicycleHere = 10172,
  kWarpToNearbyPoint = 20000,
  kUnknown10105 = 10105,
  kUnknown10325 = 10325,
  kUnknown10830 = 10830,

  /* ---------------------------------------------------------
  RUN WHEN THE MAP ITSELF IS ENTERED OR LEFT
  --------------------------------------------------------- */
  kOnMapEnter = 60000,
  kOnMapExit = 60001,
  kUnknown60002 = 60002,
  kOnNewGame = 9600,
  kOnPostGameStart = 9602,
  kUnknown9601 = 9601,

  /* ---------------------------------------------------------
  RANGES
  One script per trainer and per pickable item, too many and too tied to their
  own map to be worth naming. Offset from the first of each block instead.
  --------------------------------------------------------- */
  kFirstTrainerBattle = 3000,
  kFirstTrainerRematch = 5000,
  kFirstGroundItem = 7000,

  /* ---------------------------------------------------------
  CUSTOM SCRIPTS
  Written in C++ and registered with feature::NativeScript. The game has no
  .amx for these: any id its script table doesn't cover, below the map
  enter/exit ids at 60000, works.
  --------------------------------------------------------- */
  kFirstCustom = 30500,
  kLittlerootGreeter = 30500,
  kLastCustom = 59999,
};
