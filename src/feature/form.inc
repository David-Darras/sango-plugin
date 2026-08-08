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
  u16 species;
  u16 form_count;
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
    {SPECIES_VENUSAUR, FORM_VENUSAUR_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_CHARIZARD, FORM_CHARIZARD_COUNT, FORMS_NORMAL_MEGA_X_Y},
    {SPECIES_BLASTOISE, FORM_BLASTOISE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_BEEDRILL, FORM_BEEDRILL_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_PIDGEOT, FORM_PIDGEOT_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_PIKACHU, FORM_PIKACHU_COUNT, FORMS_PIKACHU},
    {SPECIES_ALAKAZAM, FORM_ALAKAZAM_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_SLOWBRO, FORM_SLOWBRO_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_GENGAR, FORM_GENGAR_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_KANGASKHAN, FORM_KANGASKHAN_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_PINSIR, FORM_PINSIR_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_GYARADOS, FORM_GYARADOS_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_AERODACTYL, FORM_AERODACTYL_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_MEWTWO, FORM_MEWTWO_COUNT, FORMS_NORMAL_MEGA_X_Y},
    {SPECIES_AMPHAROS, FORM_AMPHAROS_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_UNOWN, FORM_UNOWN_COUNT, FORMS_UNOWN},
    {SPECIES_STEELIX, FORM_STEELIX_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_SCIZOR, FORM_SCIZOR_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_HERACROSS, FORM_HERACROSS_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_HOUNDOOM, FORM_HOUNDOOM_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_TYRANITAR, FORM_TYRANITAR_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_SCEPTILE, FORM_SCEPTILE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_BLAZIKEN, FORM_BLAZIKEN_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_SWAMPERT, FORM_SWAMPERT_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_GARDEVOIR, FORM_GARDEVOIR_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_SABLEYE, FORM_SABLEYE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_MAWILE, FORM_MAWILE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_AGGRON, FORM_AGGRON_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_MEDICHAM, FORM_MEDICHAM_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_MANECTRIC, FORM_MANECTRIC_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_SHARPEDO, FORM_SHARPEDO_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_CAMERUPT, FORM_CAMERUPT_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_ALTARIA, FORM_ALTARIA_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_CASTFORM, FORM_CASTFORM_COUNT, FORMS_CASTFORM},
    {SPECIES_BANETTE, FORM_BANETTE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_ABSOL, FORM_ABSOL_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_GLALIE, FORM_GLALIE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_SALAMENCE, FORM_SALAMENCE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_METAGROSS, FORM_METAGROSS_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_LATIAS, FORM_LATIAS_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_LATIOS, FORM_LATIOS_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_KYOGRE, FORM_KYOGRE_COUNT, FORMS_KYOGRE},
    {SPECIES_GROUDON, FORM_GROUDON_COUNT, FORMS_GROUDON},
    {SPECIES_RAYQUAZA, FORM_RAYQUAZA_COUNT, FORMS_RAYQUAZA},
    {SPECIES_DEOXYS, FORM_DEOXYS_COUNT, FORMS_DEOXYS},
    {SPECIES_BURMY, FORM_BURMY_COUNT, FORMS_BURMY},
    {SPECIES_WORMADAM, FORM_WORMADAM_COUNT, FORMS_WORMADAM},
    {SPECIES_CHERRIM, FORM_CHERRIM_COUNT, FORMS_CHERRIM},
    {SPECIES_SHELLOS, FORM_SHELLOS_COUNT, FORMS_WEST_EAST},
    {SPECIES_GASTRODON, FORM_GASTRODON_COUNT, FORMS_WEST_EAST},
    {SPECIES_LOPUNNY, FORM_LOPUNNY_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_GARCHOMP, FORM_GARCHOMP_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_LUCARIO, FORM_LUCARIO_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_ABOMASNOW, FORM_ABOMASNOW_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_GALLADE, FORM_GALLADE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_ROTOM, FORM_ROTOM_COUNT, FORMS_ROTOM},
    {SPECIES_GIRATINA, FORM_GIRATINA_COUNT, FORMS_GIRATINA},
    {SPECIES_SHAYMIN, FORM_SHAYMIN_COUNT, FORMS_SHAYMIN},
    {SPECIES_ARCEUS, FORM_ARCEUS_COUNT, FORMS_ARCEUS},
    {SPECIES_AUDINO, FORM_AUDINO_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_BASCULIN, FORM_BASCULIN_COUNT, FORMS_BASCULIN},
    {SPECIES_DARMANITAN, FORM_DARMANITAN_COUNT, FORMS_DARMANITAN},
    {SPECIES_DEERLING, FORM_DEERLING_COUNT, FORMS_SEASONS},
    {SPECIES_SAWSBUCK, FORM_SAWSBUCK_COUNT, FORMS_SEASONS},
    {SPECIES_TORNADUS, FORM_TORNADUS_COUNT, FORMS_INCARNATE_THERIAN},
    {SPECIES_THUNDURUS, FORM_THUNDURUS_COUNT, FORMS_INCARNATE_THERIAN},
    {SPECIES_LANDORUS, FORM_LANDORUS_COUNT, FORMS_INCARNATE_THERIAN},
    {SPECIES_KYUREM, FORM_KYUREM_COUNT, FORMS_KYUREM},
    {SPECIES_KELDEO, FORM_KELDEO_COUNT, FORMS_KELDEO},
    {SPECIES_MELOETTA, FORM_MELOETTA_COUNT, FORMS_MELOETTA},
    {SPECIES_GENESECT, FORM_GENESECT_COUNT, FORMS_GENESECT},
    {SPECIES_VIVILLON, FORM_VIVILLON_COUNT, FORMS_VIVILLON},
    {SPECIES_FLABEBE, FORM_FLABEBE_COUNT, FORMS_FLOWER_COLORS},
    {SPECIES_FLOETTE, FORM_FLOETTE_COUNT, FORMS_FLOETTE},
    {SPECIES_FLORGES, FORM_FLORGES_COUNT, FORMS_FLOWER_COLORS},
    {SPECIES_FURFROU, FORM_FURFROU_COUNT, FORMS_FURFROU},
    {SPECIES_MEOWSTIC, FORM_MEOWSTIC_COUNT, FORMS_MEOWSTIC},
    {SPECIES_AEGISLASH, FORM_AEGISLASH_COUNT, FORMS_AEGISLASH},
    {SPECIES_PUMPKABOO, FORM_PUMPKABOO_COUNT, FORMS_SIZES},
    {SPECIES_GOURGEIST, FORM_GOURGEIST_COUNT, FORMS_SIZES},
    {SPECIES_XERNEAS, FORM_XERNEAS_COUNT, FORMS_XERNEAS},
    {SPECIES_DIANCIE, FORM_DIANCIE_COUNT, FORMS_NORMAL_MEGA},
    {SPECIES_HOOPA, FORM_HOOPA_COUNT, FORMS_HOOPA},
};

static u16 GetFormCount(u16 species) {
  for (u32 i = 0; i < species; i++) {
    if (POKEMON_FORMS_LIST[i].species == species) {
      return POKEMON_FORMS_LIST[i].form_count;
    }
  }
  return 1;
}

static const char* GetFormName(u16 species, u16 form) {
  for (u32 i = 0; i < species; i++) {
    if (POKEMON_FORMS_LIST[i].species == species
        && form < POKEMON_FORMS_LIST[i].form_count) {
      return POKEMON_FORMS_LIST[i].form_names[form];
    }
  }
  return "Normal";
}