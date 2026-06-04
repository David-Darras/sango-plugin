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

static const char* FORMS[][30] =
{
    {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
        "O",
        "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "!", "?"
    }, // Unown
    {"Normal", "Attack", "Defense", "Speed"}, // Deoxys
    {"Land", "Sky"}, // Shaymin
    {"Altered", "Origin"}, // Giratina
    {"Normal", "Heat", "Wash", "Frost", "Fan", "Mow"}, // Rotom
    {"West Sea", "East Sea"}, // Shellos
    {"West Sea", "East Sea"}, // Gastrodon
    {"Plant", "Sandy", "Trash"}, // Burmy
    {"Plant", "Sandy", "Trash"}, // Wormadam
    {"Normal", "Sunny", "Rainy", "Snowy"}, // Castform
    {"Overcast", "Sunshine"}, // Cherrim
    {"Spring", "Summer", "Autumn", "Winter"}, // Deerling
    {"Spring", "Summer", "Autumn", "Winter"}, // Sawsbuck
    {"Aria", "Step"}, // Meloetta
    {"Standard", "Zen"}, // Darmanitan
    {"Red-Striped", "Blue-Striped"}, // Basculin

    {"Normal", "White", "Black"}, // Kyurem
    {"Ordinary", "Resolute"}, // Keldeo
    {"Incarnate", "Therian"}, // Thundurus
    {"Incarnate", "Therian"}, // Tornadus
    {"Incarnate", "Therian"}, // Landorus

    {
        "Icy Snow", "Polar", "Tundra", "Continental", "Garden", "Elegant",
        "Meadow", "Modern", "Marine", "Archipelago", "High Plains", "Sandstorm",
        "River", "Monsoon", "Savanna", "Sun", "Ocean", "Jungle", "Fancy",
        "Poké Ball"
    }, // Vivillon
    {"Red", "Yellow", "Orange", "Blue", "White"}, // Flabébé
    {"Red", "Yellow", "Orange", "Blue", "White", "Eternal"}, // Floette
    {"Red", "Yellow", "Orange", "Blue", "White"}, // Florges
    {"Average", "Small", "Large", "Super"}, // Pumpkaboo
    {"Average", "Small", "Large", "Super"}, // Gourgeist
    {"Shield", "Blade"}, // Aegislash
    {"Neutral", "Active"}, // Xerneas

    // --- Mega Evolutions ---
    {"Normal", "Mega"}, // Venusaur
    {"Normal", "Mega X", "Mega Y"}, // Charizard
    {"Normal", "Mega"}, // Blastoise
    {"Normal", "Mega"}, // Alakazam
    {"Normal", "Mega"}, // Gengar
    {"Normal", "Mega"}, // Kangaskhan
    {"Normal", "Mega"}, // Pinsir
    {"Normal", "Mega"}, // Gyarados
    {"Normal", "Mega"}, // Aerodactyl
    {"Normal", "Mega X", "Mega Y"}, // Mewtwo
    {"Normal", "Mega"}, // Ampharos
    {"Normal", "Mega"}, // Scizor
    {"Normal", "Mega"}, // Heracross
    {"Normal", "Mega"}, // Houndoom
    {"Normal", "Mega"}, // Tyranitar
    {"Normal", "Mega"}, // Blaziken
    {"Normal", "Mega"}, // Gardevoir
    {"Normal", "Mega"}, // Mawile
    {"Normal", "Mega"}, // Aggron
    {"Normal", "Mega"}, // Medicham
    {"Normal", "Mega"}, // Manectric
    {"Normal", "Mega"}, // Banette
    {"Normal", "Mega"}, // Absol
    {"Normal", "Mega"}, // Latias
    {"Normal", "Mega"}, // Latios
    {"Normal", "Mega"}, // Garchomp
    {"Normal", "Mega"}, // Lucario
    {"Normal", "Mega"}, // Abomasnow

    {"Normal", "Rock Star", "Belle", "Pop Star", "Ph. D.", "Libre", "Cosplay"},
    // Pikachu
    {"Confined", "Unbound"}, // Hoopa
    {"Normal", "Mega"}, // Beedrill
    {"Normal", "Mega"}, // Pidgeot
    {"Normal", "Mega"}, // Slowbro
    {"Normal", "Mega"}, // Steelix
    {"Normal", "Mega"}, // Sceptile
    {"Normal", "Mega"}, // Swampert
    {"Normal", "Mega"}, // Sableye
    {"Normal", "Mega"}, // Sharpedo
    {"Normal", "Mega"}, // Camerupt
    {"Normal", "Mega"}, // Altaria
    {"Normal", "Mega"}, // Glalie
    {"Normal", "Mega"}, // Salamence
    {"Normal", "Mega"}, // Metagross
    {"Normal", "Mega"}, // Rayquaza
    {"Normal", "Mega"}, // Lopunny
    {"Normal", "Mega"}, // Gallade
    {"Normal", "Mega"}, // Audino
    {"Normal", "Mega"}, // Diancie
    {"Normal", "Primal"}, // Kyogre
    {"Normal", "Primal"}, // Groudon

    {
        "Normal", "Fighting", "Flying", "Poison", "Ground", "Rock", "Bug",
        "Ghost",
        "Steel", "Fire", "Water", "Grass", "Electric", "Psychic", "Ice",
        "Dragon",
        "Dark", "Fairy"
    }, // Arceus
    {"Normal", "Douse", "Shock", "Burn", "Chill"}, // Genesect
    {
        "Natural", "Heart", "Star", "Diamond", "Debutante", "Matron", "Dandy",
        "La Reine", "Kabuki", "Pharaoh"
    }, // Furfrou
};