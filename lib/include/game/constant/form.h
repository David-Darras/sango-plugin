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

enum class Form : u8 {
  kNormal = 0,
  kMega = 1,

  kVenusaurNormal = 0,
  kVenusaurMega = 1,
  kVenusaurCount = 2,

  kCharizardNormal = 0,
  kCharizardMegax = 1,
  kCharizardMegay = 2,
  kCharizardCount = 3,

  kBlastoiseNormal = 0,
  kBlastoiseMega = 1,
  kBlastoiseCount = 2,

  kBeedrillNormal = 0,
  kBeedrillMega = 1,
  kBeedrillCount = 2,

  kPidgeotNormal = 0,
  kPidgeotMega = 1,
  kPidgeotCount = 2,

  kPikachuNormal = 0,
  kPikachuStyle = 1,
  kPikachuBeautiful = 2,
  kPikachuCute = 3,
  kPikachuClever = 4,
  kPikachuStrong = 5,
  kPikachuContest = 6,
  kPikachuCount = 7,

  kAlakazamNormal = 0,
  kAlakazamMega = 1,
  kAlakazamCount = 2,

  kSlowbroNormal = 0,
  kSlowbroMega = 1,
  kSlowbroCount = 2,

  kGengarNormal = 0,
  kGengarMega = 1,
  kGengarCount = 2,

  kKangaskhanNormal = 0,
  kKangaskhanMega = 1,
  kKangaskhanCount = 2,

  kPinsirNormal = 0,
  kPinsirMega = 1,
  kPinsirCount = 2,

  kGyaradosNormal = 0,
  kGyaradosMega = 1,
  kGyaradosCount = 2,

  kAerodactylNormal = 0,
  kAerodactylMega = 1,
  kAerodactylCount = 2,

  kMewtwoNormal = 0,
  kMewtwoMegax = 1,
  kMewtwoMegay = 2,
  kMewtwoCount = 3,

  kAmpharosNormal = 0,
  kAmpharosMega = 1,
  kAmpharosCount = 2,

  kUnownA = 0,
  kUnownB = 1,
  kUnownC = 2,
  kUnownD = 3,
  kUnownE = 4,
  kUnownF = 5,
  kUnownG = 6,
  kUnownH = 7,
  kUnownI = 8,
  kUnownJ = 9,
  kUnownK = 10,
  kUnownL = 11,
  kUnownM = 12,
  kUnownN = 13,
  kUnownO = 14,
  kUnownP = 15,
  kUnownQ = 16,
  kUnownR = 17,
  kUnownS = 18,
  kUnownT = 19,
  kUnownU = 20,
  kUnownV = 21,
  kUnownW = 22,
  kUnownX = 23,
  kUnownY = 24,
  kUnownZ = 25,
  kUnownExclamation = 26,
  kUnownQuestion = 27,
  kUnownCount = 28,

  kSteelixNormal = 0,
  kSteelixMega = 1,
  kSteelixCount = 2,

  kScizorNormal = 0,
  kScizorMega = 1,
  kScizorCount = 2,

  kHeracrossNormal = 0,
  kHeracrossMega = 1,
  kHeracrossCount = 2,

  kHoundoomNormal = 0,
  kHoundoomMega = 1,
  kHoundoomCount = 2,

  kTyranitarNormal = 0,
  kTyranitarMega = 1,
  kTyranitarCount = 2,

  kSceptileNormal = 0,
  kSceptileMega = 1,
  kSceptileCount = 2,

  kBlazikenNormal = 0,
  kBlazikenMega = 1,
  kBlazikenCount = 2,

  kSwampertNormal = 0,
  kSwampertMega = 1,
  kSwampertCount = 2,

  kGardevoirNormal = 0,
  kGardevoirMega = 1,
  kGardevoirCount = 2,

  kSableyeNormal = 0,
  kSableyeMega = 1,
  kSableyeCount = 2,

  kMawileNormal = 0,
  kMawileMega = 1,
  kMawileCount = 2,

  kAggronNormal = 0,
  kAggronMega = 1,
  kAggronCount = 2,

  kMedichamNormal = 0,
  kMedichamMega = 1,
  kMedichamCount = 2,

  kManectricNormal = 0,
  kManectricMega = 1,
  kManectricCount = 2,

  kSharpedoNormal = 0,
  kSharpedoMega = 1,
  kSharpedoCount = 2,

  kCameruptNormal = 0,
  kCameruptMega = 1,
  kCameruptCount = 2,

  kAltariaNormal = 0,
  kAltariaMega = 1,
  kAltariaCount = 2,

  kCastformNormal = 0,
  kCastformSun = 1,
  kCastformRain = 2,
  kCastformSnow = 3,
  kCastformCount = 4,

  kBanetteNormal = 0,
  kBanetteMega = 1,
  kBanetteCount = 2,

  kAbsolNormal = 0,
  kAbsolMega = 1,
  kAbsolCount = 2,

  kGlalieNormal = 0,
  kGlalieMega = 1,
  kGlalieCount = 2,

  kSalamenceNormal = 0,
  kSalamenceMega = 1,
  kSalamenceCount = 2,

  kMetagrossNormal = 0,
  kMetagrossMega = 1,
  kMetagrossCount = 2,

  kLatiasNormal = 0,
  kLatiasMega = 1,
  kLatiasCount = 2,

  kLatiosNormal = 0,
  kLatiosMega = 1,
  kLatiosCount = 2,

  kKyogreNormal = 0,
  kKyogreAlpha = 1,
  kKyogreCount = 2,

  kGroudonNormal = 0,
  kGroudonOmega = 1,
  kGroudonCount = 2,

  kRayquazaNormal = 0,
  kRayquazaDelta = 1,
  kRayquazaCount = 2,

  kDeoxysNormal = 0,
  kDeoxysAttack = 1,
  kDeoxysDefence = 2,
  kDeoxysSpeed = 3,
  kDeoxysCount = 4,

  kBurmyPlant = 0,
  kBurmySand = 1,
  kBurmyTrash = 2,
  kBurmyCount = 3,

  kWormadamPlant = 0,
  kWormadamGround = 1,
  kWormadamSteel = 2,
  kWormadamCount = 3,

  kCherrimOvercast = 0,
  kCherrimSunshine = 1,
  kCherrimCount = 2,

  kShellosWest = 0,
  kShellosEast = 1,
  kShellosCount = 2,

  kGastrodonWest = 0,
  kGastrodonEast = 1,
  kGastrodonCount = 2,

  kLopunnyNormal = 0,
  kLopunnyMega = 1,
  kLopunnyCount = 2,

  kGarchompNormal = 0,
  kGarchompMega = 1,
  kGarchompCount = 2,

  kLucarioNormal = 0,
  kLucarioMega = 1,
  kLucarioCount = 2,

  kAbomasnowNormal = 0,
  kAbomasnowMega = 1,
  kAbomasnowCount = 2,

  kGalladeNormal = 0,
  kGalladeMega = 1,
  kGalladeCount = 2,

  kRotomNormal = 0,
  kRotomHeat = 1,
  kRotomWash = 2,
  kRotomFrost = 3,
  kRotomFan = 4,
  kRotomMow = 5,
  kRotomCount = 6,

  kGiratinaAltered = 0,
  kGiratinaOrigin = 1,
  kGiratinaCount = 2,

  kShayminLand = 0,
  kShayminSky = 1,
  kShayminCount = 2,

  kArceusNormal = 0,
  kArceusFighting = 1,
  kArceusFlying = 2,
  kArceusPoison = 3,
  kArceusGround = 4,
  kArceusRock = 5,
  kArceusBug = 6,
  kArceusGhost = 7,
  kArceusSteel = 8,
  kArceusFire = 9,
  kArceusWater = 10,
  kArceusGrass = 11,
  kArceusElectric = 12,
  kArceusPsychic = 13,
  kArceusIce = 14,
  kArceusDragon = 15,
  kArceusDark = 16,
  kArceusFairy = 17,
  kArceusCount = 18,

  kAudinoNormal = 0,
  kAudinoMega = 1,
  kAudinoCount = 2,

  kBasculinBlack = 0,
  kBasculinWhite = 1,
  kBasculinCount = 2,

  kDarmanitanStandard = 0,
  kDarmanitanZen = 1,
  kDarmanitanCount = 2,

  kDeerlingSpring = 0,
  kDeerlingSummer = 1,
  kDeerlingAutumn = 2,
  kDeerlingWinter = 3,
  kDeerlingCount = 4,

  kSawsbuckSpring = 0,
  kSawsbuckSummer = 1,
  kSawsbuckAutumn = 2,
  kSawsbuckWinter = 3,
  kSawsbuckCount = 4,

  kTornadusIncarnate = 0,
  kTornadusTherian = 1,
  kTornadusCount = 2,

  kThundurusIncarnate = 0,
  kThundurusTherian = 1,
  kThundurusCount = 2,

  kLandorusIncarnate = 0,
  kLandorusTherian = 1,
  kLandorusCount = 2,

  kKyuremNormal = 0,
  kKyuremWhite = 1,
  kKyuremBlack = 2,
  kKyuremCount = 3,

  kKeldeoOrdinary = 0,
  kKeldeoResolute = 1,
  kKeldeoCount = 2,

  kMeloettaAria = 0,
  kMeloettaPirouette = 1,
  kMeloettaCount = 2,

  kGenesectNormal = 0,
  kGenesectAqua = 1,
  kGenesectShock = 2,
  kGenesectBlaze = 3,
  kGenesectFreeze = 4,
  kGenesectCount = 5,

  kVivillonArea1 = 0,
  kVivillonArea2 = 1,
  kVivillonArea3 = 2,
  kVivillonArea4 = 3,
  kVivillonArea5 = 4,
  kVivillonArea6 = 5,
  kVivillonArea7 = 6,
  kVivillonArea8 = 7,
  kVivillonArea9 = 8,
  kVivillonArea10 = 9,
  kVivillonArea11 = 10,
  kVivillonArea12 = 11,
  kVivillonArea13 = 12,
  kVivillonArea14 = 13,
  kVivillonArea15 = 14,
  kVivillonArea16 = 15,
  kVivillonArea17 = 16,
  kVivillonArea18 = 17,
  kVivillonArea19 = 18,
  kVivillonArea20 = 19,
  kVivillonCount = 20,

  kFlabebeRed = 0,
  kFlabebeYellow = 1,
  kFlabebeOrange = 2,
  kFlabebeBlue = 3,
  kFlabebeWhite = 4,
  kFlabebeCount = 5,

  kFloetteRed = 0,
  kFloetteYellow = 1,
  kFloetteOrange = 2,
  kFloetteBlue = 3,
  kFloetteWhite = 4,
  kFloetteEternal = 5,
  kFloetteCount = 6,

  kFlorgesRed = 0,
  kFlorgesYellow = 1,
  kFlorgesOrange = 2,
  kFlorgesBlue = 3,
  kFlorgesWhite = 4,
  kFlorgesCount = 5,

  kFurfrouNormal = 0,
  kFurfrouHeart = 1,
  kFurfrouStar = 2,
  kFurfrouDiamond = 3,
  kFurfrouDebutante = 4,
  kFurfrouMatron = 5,
  kFurfrouDandy = 6,
  kFurfrouLaReine = 7,
  kFurfrouKabuki = 8,
  kFurfrouPharaoh = 9,
  kFurfrouCount = 10,

  kMeowsticMale = 0,
  kMeowsticFemale = 1,
  kMeowsticCount = 2,

  kAegislashShield = 0,
  kAegislashBlade = 1,
  kAegislashCount = 2,

  kPumpkabooNormal = 0,
  kPumpkabooSmall = 1,
  kPumpkabooLarge = 2,
  kPumpkabooSuper = 3,
  kPumpkabooCount = 4,

  kGourgeistNormal = 0,
  kGourgeistSmall = 1,
  kGourgeistLarge = 2,
  kGourgeistSuper = 3,
  kGourgeistCount = 4,

  kXerneasNormal = 0,
  kXerneasActive = 1,
  kXerneasCount = 2,

  kDiancieNormal = 0,
  kDiancieMega = 1,
  kDiancieCount = 2,

  kHoopaConfined = 0,
  kHoopaUnbound = 1,
  kHoopaCount = 2,
};