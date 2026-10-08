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
 * @file archive_id.h
 * @brief The ids of the archives (GARC files) of the game.
 *
 * An archive is a file of the game that contains many files: models,
 * texts, data. The id is the number of the archive in the game.
 */

#pragma once

#include <types.h>


namespace core {

/// The id of an archive of the game. See core::Archive.
enum class ArchiveId : u32 {
  kSummary = 0, ///< The layouts of the summary screen.
  kSummaryModel = 1, ///< The cameras of the summary screen.
  kPartyMenu = 2, ///< The layouts of the party screen.
  kBattle3d = 3, ///< The camera animations of the battles.
  kBattleLayout = 4,
  kBattleApp = 5, ///< The bottom screen of the battles.
  kMoveMotionTable = 6,
  kBattleAi = 7, ///< The AI scripts of the trainers (Pawn).
  kPokemonModel = 8,
  kFade = 9, ///< The fade between two maps.
  kSkybox = 10, ///< The sky and the weather.
  kMenuAppWindow = 11,
  kMenuPokemonAmieWindow = 12,
  kZoneData = 13,
  kOverworldArea = 14,
  kConfig = 15, ///< The layout of the Options menu.
  kPokemonAmieBerryPicker = 16,
  kPokemonAmieHeadIt = 17,
  kPokemonAmieTilePuzzle = 18,
  kNameInput = 19, ///< The layouts of the name input screen.
  kNameInputData = 20, ///< The labels of the keyboard buttons.
  kOverworldModel = 21,
  kOverworldModelParam = 22, ///< The descriptions of the character models.
  kPropModel = 23,
  /// The title panels of the bottom screen in the overworld.
  kOverworldMenu = 24,
  kOverworldMenuPss = 25,
  kOverworldMenuPokemonAmie = 26,
  /// The Super Training panel of the bottom screen.
  kOverworldMenuSuperTraining = 27,
  kTrainingCommon = 28, ///< The training selection of Super Training.
  kOverworldScript = 29, ///< The shared scripts (AMX files).
  kMoveLearn = 30,
  kMoveEffectParticle = 31,
  kMoveEffectModel = 32,
  kMoveEffectLayout = 33, ///< The layouts of the battle intros.
  kBattleMoveAnimation = 34,
  kPokemonAmieEffect = 35, ///< The emotions of the Pokémon in Pokémon-Amie.
  kTrainerData = 36,
  kTrainerTypeData = 37, ///< The trainer classes.
  kTrainerPokemonData = 38,
  kMapLandData = 39, ///< The graphics of the overworld blocks.
  kMapMatrix = 40,
  kPokemonAmieMenu = 41,
  kPokemonAmieIcon = 42,
  kPssBattleReceive = 43, ///< The connection layouts of the PSS.
  kPssProfile = 44,
  kPssInviteWindow = 45,
  kPssInterfereWindow = 46, ///< The PSS layout over the overworld.
  kPssUserNotify = 47,
  kPssMessageWindow = 48,
  kPssIconSelect = 49,
  kPssPlaySelect = 50, ///< The selection of the battle or trade connection.
  kPssFavorite = 51,
  kPssPlayerSelect = 52,
  kPssYesNoDialog = 53,
  kPssMainMenu = 54,
  kPssConfig = 55,
  /// The layout to view a player and to interact with the player.
  kPssCharacterMenu = 56,
  kPssMatching = 57, ///< The layout of the battle settings.
  kPssInterfereInfo = 58,
  /// The sky and the textures of the overworld areas.
  kOverworldAreaEnvironment = 59,
  kWeather = 60, ///< The weather effects of the overworld.
  kBag = 61,
  kOverworldCameraAnimation = 62,
  kBattleShaders = 63, ///< The culling shaders.
  kOverworldEffect = 64, ///< The effect pack of the overworld.
  kBattleSpot = 65, ///< The Battle Spot layouts of the PSS.
  kTownMapInfo = 66, ///< The places of the Town Map.
  kTownMapLayout = 67,
  kTownMapNavLayout = 68,
  /// The menus of the title screen (Mystery Gift, Live Competition...).
  kTitleMenuLayout = 69,
  kSaveDataLayout = 70,
  kGameTextJapanese = 71,
  kGameTextKanji = 72,
  kGameTextEnglish = 73,
  kGameTextFrench = 74,
  kGameTextItalian = 75,
  kGameTextGerman = 76,
  kGameTextSpanish = 77,
  kGameTextKorean = 78,
  /// The texts of the story and of the characters.
  kScriptMessageJapanese = 79,
  kScriptMessageJapaneseKanji = 80,
  kScriptMessageEnglish = 81,
  kScriptMessageFrench = 82,
  kScriptMessageItalian = 83,
  kScriptMessageGerman = 84,
  kScriptMessageSpanish = 85,
  kScriptMessageKorean = 86,
  /// The overworld outfit models. Alpha Sapphire does not use them.
  kOverworldCustomizationData = 87,
  kBattleCustomizationData = 88, ///< The battle outfit models.
  kPokemonAmieScript = 89,
  kBoutique = 90, ///< The layouts of the Boutique (the clothing shop).
  kPokemonIcon = 91,
  kItemIcon = 92,
  /// The icons of the move categories (physical, special, status).
  kMoveCategoryIcon = 93,
  kMessageWindow = 94, ///< The layouts of the message windows.
  kPartyMenuCommon = 95, ///< The Exp. bar, the HP bar and the held item icon.
  kSummaryCommon = 96, ///< The small icons of the summary screen (markings...).
  kTimeIcon = 97, ///< The layout of the loading wheel.
  kMenuCursor = 98, ///< The layout of the red cursor frame.
  kPoffinIcon = 99,
  kPokemonVoice = 100, ///< The cries and the sound of the promotion videos.
  kPokemonAmieModel = 101, ///< The Poké Puff models.
  kBoutiqueData = 102, ///< The lights and the environment of the Boutique.
  kBox = 103,
  kPokeballModel = 104, ///< The Poké Ball models of the overworld (low detail).
  /// The Poké Ball models of the battles (high detail).
  kPokeballModelBattle = 105,
  kStarterSelect = 106,
  kShop = 107, ///< The layouts of the Poké Mart.
  kPlayerIcon = 108,
  kTrainerCard = 109, ///< The Trainer Card and its 3D camera.
  kTouchBarCommon = 110, ///< The touch bar of the Pokédex and of the PC boxes.
  kPcMenu = 111, ///< The layouts of the first PC screen.
  kTradeList = 112, ///< The layouts of the trade screen.
  kLinkBattleMenu = 113,
  kSuperTraining = 114,
  kSuperTrainingActorData = 115,
  kSuperTrainingAnimationData = 116,
  kSuperTrainingStageData = 117,
  kSuperTrainingBossData = 118,
  kSuperTrainingMookData = 119,
  kSuperTrainingBossAction = 120,
  kSuperTrainingMookAction = 121,
  kSuperTrainingBossAi = 122,
  kSuperTrainingMookAi = 123,
  kSuperTrainingMissileData = 124,
  kSuperTrainingFireData = 125,
  kSuperTrainingBulletData = 126,
  kSuperTrainingGoodsData = 127,
  kSuperTrainingItemData = 128,
  kSuperTrainingParticle = 129,
  kSuperTrainingParam = 130,
  kEvolutionDemo = 131,
  kPokedexData = 132,
  kTrainerModel = 133, ///< The trainer models of the battles (high detail).
  /// The menu with two choices (for example, to learn a move).
  kCommonDisplay = 134,
  kTrainerMessageData = 135,
  kBattleBackground = 136,
  kAreaObjectList = 137, ///< The lists of the characters of the areas.
  kGts = 138,
  kHoloCasterMailData = 139,
  kHoloCasterMail = 140, ///< The layouts and the models of the Holo Caster.
  kOPower = 141, ///< The layouts of the O-Powers.
  kBoxSearch = 142,
  kGymGimmickSequence = 143,
  kPokemonAmieApp = 144, ///< The scripts of the Pokémon-Amie room.
  kPokemonAmieBackground = 145,
  kPokemonAmiePokePuff = 146,
  kItemShortcutButton = 147,
  kMoveRelearner = 148, ///< The layouts of the Move Reminder.
  kPoffinCase = 149,
  kCaptureDemo = 150, ///< The cursor graphics of the capture tutorial.
  kPokemonRide = 151, ///< The animations of the Pokémon that the player rides.
  kCutsceneData = 152,
  kDialogueListMenu = 153, ///< The layout of the list of answers.
  kFormChangeEffect = 154,
  kMysteryGiftLayout = 155,
  kMysteryGiftEffect = 156, ///< The particle effect when a gift opens.
  kGameSyncLayout = 157,
  kHoloCasterShader = 158, ///< The static noise of the Holo Caster.
  kDialogCommon = 159, ///< The layout of the message box.
  kTrainerTexture = 160, ///< The pictures of the trainers.
  kBattleMatch = 161, ///< The layout of the online battle search.
  kCutsceneSequence = 162,
  kHmCutEffect = 163,
  kHmRockSmashEffect = 164,
  /// The Rock Smash effect on the walls of Victory Road.
  kHmRockSmashWallEffect = 165,
  kHmDigEffect = 166,
  kFont = 167,
  kFriendEntry = 168, ///< One friend of the Friend Safari list.
  kOverworldNonStayAnimation = 169,
  kRegulation = 170,
  kBattleVideoSave = 171, ///< The graphics of the Battle Video recorder.
  kGrammar = 172, ///< The tables of the articles of each language.
  kTrialDownloadLayout = 173,
  kSortString = 174, ///< The sort tables of the clothes, the items...
  kH3dCommonShader = 175,
  kOverworldResident = 176,
  kCommunicationWait = 177,
  kScrollBar = 178,
  kFriendSafariSelect = 179,
  kGameResidentGraphics = 180,
  kPokedexDistribution = 181,
  /// The Pokémon of the Battle Maison (normal level).
  kBattleMaisonMovesets = 182,
  /// The trainers of the Battle Maison (normal level).
  kBattleMaisonTrainerData = 183,
  /// The Pokémon of the Battle Maison (super level).
  kSuperMaisonMovesets = 184,
  /// The trainers of the Battle Maison (super level).
  kSuperMaisonTrainerData = 185,
  kInverseBattleMovesets = 186,
  kHallOfFamePc = 187, ///< The Hall of Fame on the PC.
  kHomeButtonLock = 188,
  kMoveTable = 189, ///< The data of the moves.
  kEggMove = 190,
  kLevelUpMove = 191,
  kEvolutionTable = 192,
  kMegaEvolutionTable = 193,
  kGrowthTable = 194, ///< The experience tables of the levels.
  kSpeciesData = 195,
  kEggSpeciesTable = 196, ///< The species of the Eggs.
  kItemData = 197,
  kItemBattlePocket = 198,
  kBattleVideoPlayer = 199,
  kCommendation = 200, ///< The certificate of the full Pokédex.
  kLanguageSelect = 201,
  kNumberInput = 202,
  kPhotoViewer = 203,
  kEshop = 204, ///< The layouts of the gift downloads.
  kTrialHouseResult = 205,
  kWonderTrade = 206, ///< The layouts of Wonder Trade.
  kVsDemo = 207, ///< The layout before and after a link battle.
  /// The selection of a partner at the Battle Maison.
  kBattleMaisonSelect = 208,
  kPokemonCenter = 209, ///< The model of the inside of the Pokémon Center.
  kOverworldSubMotion = 210,
  kHmWaterfallSplash = 211,
  /// The icons of the types and of the status conditions.
  kGraphicFontCommon = 212,
  kGraphicFontBattle = 213, ///< The buttons and the Exp. bar of the battles.
  /// The "VS <name>" texts of the battle intros.
  kGraphicFontBattleEffect = 214,
  kGraphicFontMisc = 215,
  kGraphicFontPokemonAmie = 216,
  kGraphicFontSuperTraining = 217,
  kOverworldCommonSequence = 218,
  kLiveCup = 219,
  kBoutiqueEffect = 220, ///< The effect when the player tries an outfit.
  kFishingEffect = 221, ///< The water splash of the fishing rods.
  kPokemonAmieGoodsData = 222,
  kPokemonAmieCakeData = 223,
  kLowerScreenTutorial = 224, ///< Not used.
  /// The tutorials of Pokémon-Amie and of Super Training.
  kGraphicFontTutorial = 225,
  kBattlePartySelect = 226,
  kSystemIcon = 227, ///< The StreetPass and extra save data icons.
  kBattleVideoPlay = 228, ///< The bottom screen of the Battle Video player.
  kStaffRoll = 229, ///< The layout of the credits.
  kStaffRollData = 230,
  kStereoFrame = 231, ///< The black frame of the 3D effect.
  kGameOver = 232,
  kEncounterEffect = 233,
  kGraphicFontGameSync = 234,
  kGraphicFontPokedex = 235,
  kGraphicFontTheEnd = 236, ///< The "The End" text of the credits.
  /// The layout and the models of the Hall of Fame scene.
  kHallOfFameDemo = 237,
  kFatalError = 238,
  kGraphicFontNameInput = 239,
  kGraphicFontHallOfFameDemo = 240,
  kGameClearFade = 241, ///< The white fade at the end of the game.
  kContestData = 242,
  kPokeNavPlus = 243, ///< The PokéNav Plus app on the bottom screen.
  kGraphicFontPokeNavPlus = 244,
  kPokeNavPlusCommon = 245,
  kBuzzNav = 246,
  kAreaNav = 247,
  kDexNav = 248,
  kPlayNav = 249,
  /// The appeal screen of the second round of a contest.
  kContestAppealUi = 250,
  kSoaringOverworld = 251, ///< Soaring with Latias or Latios.
  kSoaringOther = 252,
  kSoaringEffect = 253,
  kSoaringCharacter = 254, ///< The models of the player on Latias or Latios.
  kSoaringLayout = 255,
  kPokedexUi = 256,
  kSecretBase = 257, ///< The models of the inside of the Secret Bases.
  kSecretBaseLayout = 258, ///< The bottom screen of the Secret Bases.
  kSecretBasePhoto = 259,
  kGymElectric = 260,
  kGymPsychic = 261, ///< The model of the Mossdeep Gym.
  kDexNavData = 262,
  kPokemonPicture = 263, ///< The pictures of the contests.
  kContestEffect = 264,
  kContestPhoto = 265,
  kGymFlying = 266,
  /// The overworld models of the Pokémon that the player touches.
  kContactEncounter = 267,
  kContactEncounterData = 268,
  kIntroMovie = 269,
  kAreaRoute102 = 270,
  kGymFighting = 271,
  kGymFire = 272,
  kGymWater = 273,
  kEliteFour = 274, ///< The models of the Elite Four rooms.
  kHallOfFameRoom = 275,
  kGraphicFontTouchBar = 276,
  /// The Pokédex sprites of Ruby and Sapphire (Game Boy Advance).
  kPokemonSpriteRs = 277,
  kGraphicFontContestDebut = 278,
  kGraphicFontContestAppeal = 279,
  kGraphicFontContestResult = 280,
  kAreaRoute120 = 281, ///< The water reflections of Route 120.
  kOverworldScreenEffect = 282, ///< The Flash effect.
  kContestCamera = 283,
  /// The resources of the Pokémon that the player touches on Routes 104 and
  /// 105.
  kAreaWorld03 = 284,
  /// The resources of the Pokémon that the player touches on Routes 104 and
  /// 105.
  kAreaWorld05 = 285,
  kAreaRoute114 = 286,
  kAreaRoute124 = 287,
  kAreaUnderwater01 = 288,
  kAreaUnderwater02 = 289,
  kAreaUnderwater03 = 290,
  kHmFlyModel = 291, ///< The bird model of Fly and its animations.
  kAreaFortreeCity = 292,
  kChampionPassage = 293,
  kSecretBaseQr = 294,
  kAreaPacifidlogTown = 295,
  kAreaSootopolisCity = 296, ///< The water reflections of Sootopolis City.
  kAreaCaveOfOrigin = 297,
  kMotionBlur = 298,
};

} // namespace core

using core::ArchiveId;
