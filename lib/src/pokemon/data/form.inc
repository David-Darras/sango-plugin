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

typedef struct {
  SpeciesId species;
  FormId form_count;
  const char* const* form_names;
} PokemonForm;

static const char* const FORMS_NORMAL_MEGA[] = {"Normal", "Mega"};
static const char* const FORMS_NORMAL_MEGA_X_Y[] = {
    "Normal", "Mega X", "Mega Y"};
static const char* const FORMS_WEST_EAST[] = {"West", "East"};
static const char* const FORMS_SEASONS[] = {"Spring", "Summer", "Autumn",
                                            "Winter"};
static const char* const FORMS_INCARNATE_THERIAN[] = {"Incarnate", "Therian"};
static const char* const FORMS_FLOWER_COLORS[] = {
    "Red", "Yellow", "Orange", "Blue",
    "White"};
static const char* const FORMS_SIZES[] = {"Normal", "Small", "Large", "Super"};

static const char* const FORMS_PIKACHU[] = {"Normal", "Style", "Beautiful",
                                            "Cute",
                                            "Clever", "Strong", "Contest"};
static const char* const FORMS_UNOWN[] = {"A", "B", "C", "D", "E", "F", "G",
                                          "H", "I",
                                          "J", "K", "L", "M", "N", "O", "P",
                                          "Q", "R",
                                          "S", "T", "U", "V", "W", "X", "Y",
                                          "Z",
                                          "Exclamation", "Question"};
static const char* const FORMS_CASTFORM[] = {"Normal", "Sun", "Rain", "Snow"};
static const char* const FORMS_KYOGRE[] = {"Normal", "Alpha"};
static const char* const FORMS_GROUDON[] = {"Normal", "Omega"};
static const char* const FORMS_RAYQUAZA[] = {"Normal", "Delta"};
static const char* const FORMS_DEOXYS[] = {"Normal", "Attack", "Defence",
                                           "Speed"};
static const char* const FORMS_BURMY[] = {"Plant", "Sand", "Trash"};
static const char* const FORMS_WORMADAM[] = {"Plant", "Ground", "Steel"};
static const char* const FORMS_CHERRIM[] = {"Overcast", "Sunshine"};
static const char* const FORMS_ROTOM[] = {"Normal", "Heat", "Wash", "Frost",
                                          "Fan",
                                          "Mow"};
static const char* const FORMS_GIRATINA[] = {"Altered", "Origin"};
static const char* const FORMS_SHAYMIN[] = {"Land", "Sky"};
static const char* const FORMS_ARCEUS[] = {"Normal", "Fighting", "Flying",
                                           "Poison",
                                           "Ground", "Rock", "Bug", "Ghost",
                                           "Steel",
                                           "Fire", "Water", "Grass", "Electric",
                                           "Psychic", "Ice", "Dragon", "Dark",
                                           "Fairy"};
static const char* const FORMS_BASCULIN[] = {"Black", "White"};
static const char* const FORMS_DARMANITAN[] = {"Standard", "Zen"};
static const char* const FORMS_KYUREM[] = {"Normal", "White", "Black"};
static const char* const FORMS_KELDEO[] = {"Ordinary", "Resolute"};
static const char* const FORMS_MELOETTA[] = {"Aria", "Pirouette"};
static const char* const FORMS_GENESECT[] = {"Normal", "Aqua", "Shock", "Blaze",
                                             "Freeze"};
static const char* const FORMS_VIVILLON[] = {"Area 1", "Area 2", "Area 3",
                                             "Area 4",
                                             "Area 5", "Area 6", "Area 7",
                                             "Area 8",
                                             "Area 9", "Area 10", "Area 11",
                                             "Area 12",
                                             "Area 13", "Area 14", "Area 15",
                                             "Area 16", "Area 17", "Area 18",
                                             "Area 19", "Area 20"};
static const char* const FORMS_FLOETTE[] = {"Red", "Yellow", "Orange", "Blue",
                                            "White",
                                            "Eternal"};
static const char* const FORMS_FURFROU[] = {"Normal", "Heart", "Star",
                                            "Diamond",
                                            "Debutante", "Matron", "Dandy",
                                            "La Reine",
                                            "Kabuki", "Pharaoh"};
static const char* const FORMS_MEOWSTIC[] = {"Male", "Female"};
static const char* const FORMS_AEGISLASH[] = {"Shield", "Blade"};
static const char* const FORMS_XERNEAS[] = {"Normal", "Active"};
static const char* const FORMS_HOOPA[] = {"Confined", "Unbound"};

static const PokemonForm POKEMON_FORMS_LIST[] = {
    {SpeciesId::kVenusaur, FormId::kVenusaurCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kCharizard, FormId::kCharizardCount, FORMS_NORMAL_MEGA_X_Y},
    {SpeciesId::kBlastoise, FormId::kBlastoiseCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kBeedrill, FormId::kBeedrillCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kPidgeot, FormId::kPidgeotCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kPikachu, FormId::kPikachuCount, FORMS_PIKACHU},
    {SpeciesId::kAlakazam, FormId::kAlakazamCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSlowbro, FormId::kSlowbroCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGengar, FormId::kGengarCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kKangaskhan, FormId::kKangaskhanCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kPinsir, FormId::kPinsirCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGyarados, FormId::kGyaradosCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAerodactyl, FormId::kAerodactylCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kMewtwo, FormId::kMewtwoCount, FORMS_NORMAL_MEGA_X_Y},
    {SpeciesId::kAmpharos, FormId::kAmpharosCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kUnown, FormId::kUnownCount, FORMS_UNOWN},
    {SpeciesId::kSteelix, FormId::kSteelixCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kScizor, FormId::kScizorCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kHeracross, FormId::kHeracrossCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kHoundoom, FormId::kHoundoomCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kTyranitar, FormId::kTyranitarCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSceptile, FormId::kSceptileCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kBlaziken, FormId::kBlazikenCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSwampert, FormId::kSwampertCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGardevoir, FormId::kGardevoirCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSableye, FormId::kSableyeCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kMawile, FormId::kMawileCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAggron, FormId::kAggronCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kMedicham, FormId::kMedichamCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kManectric, FormId::kManectricCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSharpedo, FormId::kSharpedoCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kCamerupt, FormId::kCameruptCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAltaria, FormId::kAltariaCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kCastform, FormId::kCastformCount, FORMS_CASTFORM},
    {SpeciesId::kBanette, FormId::kBanetteCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAbsol, FormId::kAbsolCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGlalie, FormId::kGlalieCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSalamence, FormId::kSalamenceCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kMetagross, FormId::kMetagrossCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kLatias, FormId::kLatiasCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kLatios, FormId::kLatiosCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kKyogre, FormId::kKyogreCount, FORMS_KYOGRE},
    {SpeciesId::kGroudon, FormId::kGroudonCount, FORMS_GROUDON},
    {SpeciesId::kRayquaza, FormId::kRayquazaCount, FORMS_RAYQUAZA},
    {SpeciesId::kDeoxys, FormId::kDeoxysCount, FORMS_DEOXYS},
    {SpeciesId::kBurmy, FormId::kBurmyCount, FORMS_BURMY},
    {SpeciesId::kWormadam, FormId::kWormadamCount, FORMS_WORMADAM},
    {SpeciesId::kCherrim, FormId::kCherrimCount, FORMS_CHERRIM},
    {SpeciesId::kShellos, FormId::kShellosCount, FORMS_WEST_EAST},
    {SpeciesId::kGastrodon, FormId::kGastrodonCount, FORMS_WEST_EAST},
    {SpeciesId::kLopunny, FormId::kLopunnyCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGarchomp, FormId::kGarchompCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kLucario, FormId::kLucarioCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAbomasnow, FormId::kAbomasnowCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGallade, FormId::kGalladeCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kRotom, FormId::kRotomCount, FORMS_ROTOM},
    {SpeciesId::kGiratina, FormId::kGiratinaCount, FORMS_GIRATINA},
    {SpeciesId::kShaymin, FormId::kShayminCount, FORMS_SHAYMIN},
    {SpeciesId::kArceus, FormId::kArceusCount, FORMS_ARCEUS},
    {SpeciesId::kAudino, FormId::kAudinoCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kBasculin, FormId::kBasculinCount, FORMS_BASCULIN},
    {SpeciesId::kDarmanitan, FormId::kDarmanitanCount, FORMS_DARMANITAN},
    {SpeciesId::kDeerling, FormId::kDeerlingCount, FORMS_SEASONS},
    {SpeciesId::kSawsbuck, FormId::kSawsbuckCount, FORMS_SEASONS},
    {SpeciesId::kTornadus, FormId::kTornadusCount, FORMS_INCARNATE_THERIAN},
    {SpeciesId::kThundurus, FormId::kThundurusCount, FORMS_INCARNATE_THERIAN},
    {SpeciesId::kLandorus, FormId::kLandorusCount, FORMS_INCARNATE_THERIAN},
    {SpeciesId::kKyurem, FormId::kKyuremCount, FORMS_KYUREM},
    {SpeciesId::kKeldeo, FormId::kKeldeoCount, FORMS_KELDEO},
    {SpeciesId::kMeloetta, FormId::kMeloettaCount, FORMS_MELOETTA},
    {SpeciesId::kGenesect, FormId::kGenesectCount, FORMS_GENESECT},
    {SpeciesId::kVivillon, FormId::kVivillonCount, FORMS_VIVILLON},
    {SpeciesId::kFlabebe, FormId::kFlabebeCount, FORMS_FLOWER_COLORS},
    {SpeciesId::kFloette, FormId::kFloetteCount, FORMS_FLOETTE},
    {SpeciesId::kFlorges, FormId::kFlorgesCount, FORMS_FLOWER_COLORS},
    {SpeciesId::kFurfrou, FormId::kFurfrouCount, FORMS_FURFROU},
    {SpeciesId::kMeowstic, FormId::kMeowsticCount, FORMS_MEOWSTIC},
    {SpeciesId::kAegislash, FormId::kAegislashCount, FORMS_AEGISLASH},
    {SpeciesId::kPumpkaboo, FormId::kPumpkabooCount, FORMS_SIZES},
    {SpeciesId::kGourgeist, FormId::kGourgeistCount, FORMS_SIZES},
    {SpeciesId::kXerneas, FormId::kXerneasCount, FORMS_XERNEAS},
    {SpeciesId::kDiancie, FormId::kDiancieCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kHoopa, FormId::kHoopaCount, FORMS_HOOPA},
};

static u16 GetFormCount(SpeciesId species) {
  const u16 species_id = static_cast<u16>(species);
  for (u32 i = 0; i < species_id; i++) {
    if (POKEMON_FORMS_LIST[i].species == species) {
      return static_cast<u16>(POKEMON_FORMS_LIST[i].form_count);
    }
  }
  return 1;
}

static const char* GetFormName(SpeciesId species, FormId form) {
  const u16 species_id = static_cast<u16>(species);
  const u16 form_id = static_cast<u8>(form);
  for (u32 i = 0; i < species_id; i++) {
    if (POKEMON_FORMS_LIST[i].species == species
        && form_id < static_cast<u16>(POKEMON_FORMS_LIST[i].form_count)) {
      return POKEMON_FORMS_LIST[i].form_names[form_id];
    }
  }
  return "Normal";
}