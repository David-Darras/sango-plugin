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

#include "ui/main_application.h"
#include "ui/page/page_top.h"

#include "feature/feature_title_screen.h"

namespace ui {
const char* VIDEO_NAMES[] = {
    "", // 0
    "", // 1
    "Title Omega Ruby", // 2
    "Title Alpha Sapphire", // 3
    "Truck Look At Pokedex", // 4
    "Truck Show Pokemon", // 5
    "Truck Open Doors", // 6
    "Truck Jump Outside", // 7
    "", // 8
    "Boat", // 9
    "", // 10
    "Sky Up", // 11
    "Sky Down", // 12
    "Flying With Latios And Steven", // 13
    "Flying With Latias And Steven", // 14
    "Green Light", // 15
    "", // 16
    "", // 17
    "", // 18
    "", // 19
    "Groudon Walking", // 20
    "Kyogre Diving", // 21
    "", // 22
    "", // 23
    "", // 24
    "", // 25
    "", // 26
    "", // 27
    "Brown Pixels", // 28
    "Rayquaza Fly Omega Ruby", // 29
    "Rayquaza Fly Alpha Sapphire", // 30
    "Sky Pillar", // 31
    "Diving Go Down", // 32
    "Diving Go Up", // 33
    "Egg Hatching", // 34
    "Waterfall Down", // 35
    "Waterfall Up", // 36
    "Execute Field Move", // 37
    "Execute Field Move 2", // 38
    "Bulbasaur And Window", // 39
    "", // 40
    "", // 41
    "", // 42
    "Bulbasaur And Window 2", // 43
    "", // 44
    "", // 45
    "Window", // 46
    "Fly", // 47
    "Badge 1", // 48
    "Badge 2", // 49
    "Badge 3", // 50
    "Badge 4", // 51
    "Badge 5", // 52
    "Badge 6", // 53
    "Badge 7", // 54
    "Badge 8", // 55
    "Use Hm Bugged", // 56
    "Mega Latios Fly", // 57
    "Flash 1", // 58
    "Mega Latios Land", // 59
    "Flash 2", // 60
    "Evolution", // 61
    "Evolution 2", // 62
    "Evolution 3", // 63
    "Receive Pokemon", // 64
    "Receive Pokemon 2", // 65
    "Send Pokemon", // 66
    "Pokemon Exchange Red", // 67
    "Pokemon Exchange Blue", // 68
    "Black Lines", // 69
    "Black Lines 2", // 70
    "", // 71
    "", // 72
    "", // 73
    "", // 74
    "", // 75
    "", // 76
    "", // 77
    "", // 78
    "", // 79
    "", // 80
    "", // 81
    "", // 82
    "", // 83
    "Small Window", // 84
    "Primo Groudon", // 85
    "Primo Kyogre" // 86
};

void LoadTitleScreenPage(MainApplication& app, void* args) {
  auto& title_screen = feature::TitleScreen::GetInstance();

  app.Add("Is Enabled", title_screen.is_enabled)
     .Add("No delay", title_screen.no_delay)
     .Add("No shadow", title_screen.no_shadow)
     .Add("Top Video", title_screen.top_video)
     .WithArray(VIDEO_NAMES, SIZE(VIDEO_NAMES))
     .Add("Bottom Video", title_screen.bottom_video)
     .WithArray(VIDEO_NAMES, SIZE(VIDEO_NAMES))
     .AddSpecies("Pokemon Cry", title_screen.pokemon_cry_species)
     .Add("Pokemon Cry Volume", title_screen.pokemon_cry_volume);
}
} // namespace ui