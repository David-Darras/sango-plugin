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
  Species species;
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
    {Species::kVenusaur, Form::kVenusaurCount, FORMS_NORMAL_MEGA},
    {Species::kCharizard, Form::kCharizardCount, FORMS_NORMAL_MEGA_X_Y},
    {Species::kBlastoise, Form::kBlastoiseCount, FORMS_NORMAL_MEGA},
    {Species::kBeedrill, Form::kBeedrillCount, FORMS_NORMAL_MEGA},
    {Species::kPidgeot, Form::kPidgeotCount, FORMS_NORMAL_MEGA},
    {Species::kPikachu, Form::kPikachuCount, FORMS_PIKACHU},
    {Species::kAlakazam, Form::kAlakazamCount, FORMS_NORMAL_MEGA},
    {Species::kSlowbro, Form::kSlowbroCount, FORMS_NORMAL_MEGA},
    {Species::kGengar, Form::kGengarCount, FORMS_NORMAL_MEGA},
    {Species::kKangaskhan, Form::kKangaskhanCount, FORMS_NORMAL_MEGA},
    {Species::kPinsir, Form::kPinsirCount, FORMS_NORMAL_MEGA},
    {Species::kGyarados, Form::kGyaradosCount, FORMS_NORMAL_MEGA},
    {Species::kAerodactyl, Form::kAerodactylCount, FORMS_NORMAL_MEGA},
    {Species::kMewtwo, Form::kMewtwoCount, FORMS_NORMAL_MEGA_X_Y},
    {Species::kAmpharos, Form::kAmpharosCount, FORMS_NORMAL_MEGA},
    {Species::kUnown, Form::kUnownCount, FORMS_UNOWN},
    {Species::kSteelix, Form::kSteelixCount, FORMS_NORMAL_MEGA},
    {Species::kScizor, Form::kScizorCount, FORMS_NORMAL_MEGA},
    {Species::kHeracross, Form::kHeracrossCount, FORMS_NORMAL_MEGA},
    {Species::kHoundoom, Form::kHoundoomCount, FORMS_NORMAL_MEGA},
    {Species::kTyranitar, Form::kTyranitarCount, FORMS_NORMAL_MEGA},
    {Species::kSceptile, Form::kSceptileCount, FORMS_NORMAL_MEGA},
    {Species::kBlaziken, Form::kBlazikenCount, FORMS_NORMAL_MEGA},
    {Species::kSwampert, Form::kSwampertCount, FORMS_NORMAL_MEGA},
    {Species::kGardevoir, Form::kGardevoirCount, FORMS_NORMAL_MEGA},
    {Species::kSableye, Form::kSableyeCount, FORMS_NORMAL_MEGA},
    {Species::kMawile, Form::kMawileCount, FORMS_NORMAL_MEGA},
    {Species::kAggron, Form::kAggronCount, FORMS_NORMAL_MEGA},
    {Species::kMedicham, Form::kMedichamCount, FORMS_NORMAL_MEGA},
    {Species::kManectric, Form::kManectricCount, FORMS_NORMAL_MEGA},
    {Species::kSharpedo, Form::kSharpedoCount, FORMS_NORMAL_MEGA},
    {Species::kCamerupt, Form::kCameruptCount, FORMS_NORMAL_MEGA},
    {Species::kAltaria, Form::kAltariaCount, FORMS_NORMAL_MEGA},
    {Species::kCastform, Form::kCastformCount, FORMS_CASTFORM},
    {Species::kBanette, Form::kBanetteCount, FORMS_NORMAL_MEGA},
    {Species::kAbsol, Form::kAbsolCount, FORMS_NORMAL_MEGA},
    {Species::kGlalie, Form::kGlalieCount, FORMS_NORMAL_MEGA},
    {Species::kSalamence, Form::kSalamenceCount, FORMS_NORMAL_MEGA},
    {Species::kMetagross, Form::kMetagrossCount, FORMS_NORMAL_MEGA},
    {Species::kLatias, Form::kLatiasCount, FORMS_NORMAL_MEGA},
    {Species::kLatios, Form::kLatiosCount, FORMS_NORMAL_MEGA},
    {Species::kKyogre, Form::kKyogreCount, FORMS_KYOGRE},
    {Species::kGroudon, Form::kGroudonCount, FORMS_GROUDON},
    {Species::kRayquaza, Form::kRayquazaCount, FORMS_RAYQUAZA},
    {Species::kDeoxys, Form::kDeoxysCount, FORMS_DEOXYS},
    {Species::kBurmy, Form::kBurmyCount, FORMS_BURMY},
    {Species::kWormadam, Form::kWormadamCount, FORMS_WORMADAM},
    {Species::kCherrim, Form::kCherrimCount, FORMS_CHERRIM},
    {Species::kShellos, Form::kShellosCount, FORMS_WEST_EAST},
    {Species::kGastrodon, Form::kGastrodonCount, FORMS_WEST_EAST},
    {Species::kLopunny, Form::kLopunnyCount, FORMS_NORMAL_MEGA},
    {Species::kGarchomp, Form::kGarchompCount, FORMS_NORMAL_MEGA},
    {Species::kLucario, Form::kLucarioCount, FORMS_NORMAL_MEGA},
    {Species::kAbomasnow, Form::kAbomasnowCount, FORMS_NORMAL_MEGA},
    {Species::kGallade, Form::kGalladeCount, FORMS_NORMAL_MEGA},
    {Species::kRotom, Form::kRotomCount, FORMS_ROTOM},
    {Species::kGiratina, Form::kGiratinaCount, FORMS_GIRATINA},
    {Species::kShaymin, Form::kShayminCount, FORMS_SHAYMIN},
    {Species::kArceus, Form::kArceusCount, FORMS_ARCEUS},
    {Species::kAudino, Form::kAudinoCount, FORMS_NORMAL_MEGA},
    {Species::kBasculin, Form::kBasculinCount, FORMS_BASCULIN},
    {Species::kDarmanitan, Form::kDarmanitanCount, FORMS_DARMANITAN},
    {Species::kDeerling, Form::kDeerlingCount, FORMS_SEASONS},
    {Species::kSawsbuck, Form::kSawsbuckCount, FORMS_SEASONS},
    {Species::kTornadus, Form::kTornadusCount, FORMS_INCARNATE_THERIAN},
    {Species::kThundurus, Form::kThundurusCount, FORMS_INCARNATE_THERIAN},
    {Species::kLandorus, Form::kLandorusCount, FORMS_INCARNATE_THERIAN},
    {Species::kKyurem, Form::kKyuremCount, FORMS_KYUREM},
    {Species::kKeldeo, Form::kKeldeoCount, FORMS_KELDEO},
    {Species::kMeloetta, Form::kMeloettaCount, FORMS_MELOETTA},
    {Species::kGenesect, Form::kGenesectCount, FORMS_GENESECT},
    {Species::kVivillon, Form::kVivillonCount, FORMS_VIVILLON},
    {Species::kFlabebe, Form::kFlabebeCount, FORMS_FLOWER_COLORS},
    {Species::kFloette, Form::kFloetteCount, FORMS_FLOETTE},
    {Species::kFlorges, Form::kFlorgesCount, FORMS_FLOWER_COLORS},
    {Species::kFurfrou, Form::kFurfrouCount, FORMS_FURFROU},
    {Species::kMeowstic, Form::kMeowsticCount, FORMS_MEOWSTIC},
    {Species::kAegislash, Form::kAegislashCount, FORMS_AEGISLASH},
    {Species::kPumpkaboo, Form::kPumpkabooCount, FORMS_SIZES},
    {Species::kGourgeist, Form::kGourgeistCount, FORMS_SIZES},
    {Species::kXerneas, Form::kXerneasCount, FORMS_XERNEAS},
    {Species::kDiancie, Form::kDiancieCount, FORMS_NORMAL_MEGA},
    {Species::kHoopa, Form::kHoopaCount, FORMS_HOOPA},
};

static u16 GetFormCount(Species species) {
  const u16 species_id = static_cast<u16>(species);
  for (u32 i = 0; i < species_id; i++) {
    if (POKEMON_FORMS_LIST[i].species == species) {
      return static_cast<u16>(POKEMON_FORMS_LIST[i].form_count);
    }
  }
  return 1;
}

static const char* GetFormName(Species species, Form form) {
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