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
  Form form_count;
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
    {SpeciesId::kVenusaur, Form::kVenusaurCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kCharizard, Form::kCharizardCount, FORMS_NORMAL_MEGA_X_Y},
    {SpeciesId::kBlastoise, Form::kBlastoiseCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kBeedrill, Form::kBeedrillCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kPidgeot, Form::kPidgeotCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kPikachu, Form::kPikachuCount, FORMS_PIKACHU},
    {SpeciesId::kAlakazam, Form::kAlakazamCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSlowbro, Form::kSlowbroCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGengar, Form::kGengarCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kKangaskhan, Form::kKangaskhanCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kPinsir, Form::kPinsirCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGyarados, Form::kGyaradosCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAerodactyl, Form::kAerodactylCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kMewtwo, Form::kMewtwoCount, FORMS_NORMAL_MEGA_X_Y},
    {SpeciesId::kAmpharos, Form::kAmpharosCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kUnown, Form::kUnownCount, FORMS_UNOWN},
    {SpeciesId::kSteelix, Form::kSteelixCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kScizor, Form::kScizorCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kHeracross, Form::kHeracrossCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kHoundoom, Form::kHoundoomCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kTyranitar, Form::kTyranitarCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSceptile, Form::kSceptileCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kBlaziken, Form::kBlazikenCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSwampert, Form::kSwampertCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGardevoir, Form::kGardevoirCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSableye, Form::kSableyeCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kMawile, Form::kMawileCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAggron, Form::kAggronCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kMedicham, Form::kMedichamCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kManectric, Form::kManectricCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSharpedo, Form::kSharpedoCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kCamerupt, Form::kCameruptCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAltaria, Form::kAltariaCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kCastform, Form::kCastformCount, FORMS_CASTFORM},
    {SpeciesId::kBanette, Form::kBanetteCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAbsol, Form::kAbsolCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGlalie, Form::kGlalieCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kSalamence, Form::kSalamenceCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kMetagross, Form::kMetagrossCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kLatias, Form::kLatiasCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kLatios, Form::kLatiosCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kKyogre, Form::kKyogreCount, FORMS_KYOGRE},
    {SpeciesId::kGroudon, Form::kGroudonCount, FORMS_GROUDON},
    {SpeciesId::kRayquaza, Form::kRayquazaCount, FORMS_RAYQUAZA},
    {SpeciesId::kDeoxys, Form::kDeoxysCount, FORMS_DEOXYS},
    {SpeciesId::kBurmy, Form::kBurmyCount, FORMS_BURMY},
    {SpeciesId::kWormadam, Form::kWormadamCount, FORMS_WORMADAM},
    {SpeciesId::kCherrim, Form::kCherrimCount, FORMS_CHERRIM},
    {SpeciesId::kShellos, Form::kShellosCount, FORMS_WEST_EAST},
    {SpeciesId::kGastrodon, Form::kGastrodonCount, FORMS_WEST_EAST},
    {SpeciesId::kLopunny, Form::kLopunnyCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGarchomp, Form::kGarchompCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kLucario, Form::kLucarioCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kAbomasnow, Form::kAbomasnowCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kGallade, Form::kGalladeCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kRotom, Form::kRotomCount, FORMS_ROTOM},
    {SpeciesId::kGiratina, Form::kGiratinaCount, FORMS_GIRATINA},
    {SpeciesId::kShaymin, Form::kShayminCount, FORMS_SHAYMIN},
    {SpeciesId::kArceus, Form::kArceusCount, FORMS_ARCEUS},
    {SpeciesId::kAudino, Form::kAudinoCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kBasculin, Form::kBasculinCount, FORMS_BASCULIN},
    {SpeciesId::kDarmanitan, Form::kDarmanitanCount, FORMS_DARMANITAN},
    {SpeciesId::kDeerling, Form::kDeerlingCount, FORMS_SEASONS},
    {SpeciesId::kSawsbuck, Form::kSawsbuckCount, FORMS_SEASONS},
    {SpeciesId::kTornadus, Form::kTornadusCount, FORMS_INCARNATE_THERIAN},
    {SpeciesId::kThundurus, Form::kThundurusCount, FORMS_INCARNATE_THERIAN},
    {SpeciesId::kLandorus, Form::kLandorusCount, FORMS_INCARNATE_THERIAN},
    {SpeciesId::kKyurem, Form::kKyuremCount, FORMS_KYUREM},
    {SpeciesId::kKeldeo, Form::kKeldeoCount, FORMS_KELDEO},
    {SpeciesId::kMeloetta, Form::kMeloettaCount, FORMS_MELOETTA},
    {SpeciesId::kGenesect, Form::kGenesectCount, FORMS_GENESECT},
    {SpeciesId::kVivillon, Form::kVivillonCount, FORMS_VIVILLON},
    {SpeciesId::kFlabebe, Form::kFlabebeCount, FORMS_FLOWER_COLORS},
    {SpeciesId::kFloette, Form::kFloetteCount, FORMS_FLOETTE},
    {SpeciesId::kFlorges, Form::kFlorgesCount, FORMS_FLOWER_COLORS},
    {SpeciesId::kFurfrou, Form::kFurfrouCount, FORMS_FURFROU},
    {SpeciesId::kMeowstic, Form::kMeowsticCount, FORMS_MEOWSTIC},
    {SpeciesId::kAegislash, Form::kAegislashCount, FORMS_AEGISLASH},
    {SpeciesId::kPumpkaboo, Form::kPumpkabooCount, FORMS_SIZES},
    {SpeciesId::kGourgeist, Form::kGourgeistCount, FORMS_SIZES},
    {SpeciesId::kXerneas, Form::kXerneasCount, FORMS_XERNEAS},
    {SpeciesId::kDiancie, Form::kDiancieCount, FORMS_NORMAL_MEGA},
    {SpeciesId::kHoopa, Form::kHoopaCount, FORMS_HOOPA},
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

static const char* GetFormName(SpeciesId species, Form form) {
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