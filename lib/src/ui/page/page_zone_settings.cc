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
 * @file page_zone_settings.cc
 * @brief The menu page of the music, the weather and the time of the current
 *        map.
 */

#include <cstdio>

#include "overworld/native/map_manager.h"
#include "overworld/patch/zone_settings.h"
#include "ui/main_application.h"
#include "ui/page/enum_names.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using overworld::ZoneSettings;

constexpr u32 kLabelLength = 48;
c8 map_note[kLabelLength];
u32 current_map = 0;

void ApplyZone(void*) { ZoneSettings::Reapply(); }

void SaveZones(void*) {
  MainApplication::GetInstance().ShowToast(
      ZoneSettings::Save() ? "Saved to the SD card." : "The SD card refused.");
}

void RemoveZone(void*) {
  ZoneSettings::Remove(current_map);
  MainApplication::GetInstance().Refresh();
}
} // namespace

void LoadZoneSettingsPage(MainApplication& app, void* args) {
  current_map = static_cast<u32>(overworld::MapManager::GetInstance().GetMap());
  ZoneSettings::Zone* zone = ZoneSettings::GetOrAdd(current_map);
  if (zone == nullptr) {
    app.Add("The table of zones is full.")
       .WithDescription("24 maps can have settings. Remove the settings of "
                        "another map first.");
    return;
  }
  snprintf(map_note, kLabelLength, "Map %lu", (unsigned long)current_map);

  app.OnChange(ApplyZone);
  app.AddSection(map_note);
  app.Add("Change The Music", zone->has_music)
     .WithDescription("On: this map always plays the music below.")
     .Add("Music", zone->music)
     .WithArray(kBackgroundMusicNames, SIZE(kBackgroundMusicNames))
     .WithBounds(0, SIZE(kBackgroundMusicNames) - 1);
#ifdef GAME_ORAS
  static const c8* kWeatherNames[] = {"Sunny",    "Rainy",   "Thunderstorm",
                                      "Misty",    "Ash",     "Sandstorm",
                                      "Cloudy",   "Stormy",  "Dry",
                                      "None"};
  app.Add("Change The Weather", zone->has_weather)
     .WithDescription("On: the weather of this map is the one below. The game "
                      "can need a few seconds to show it.")
     .Add("Weather", zone->weather)
     .WithArray(kWeatherNames, SIZE(kWeatherNames))
     .WithBounds(0, SIZE(kWeatherNames) - 1);
#endif
  app.Add("Change The Time", zone->has_time)
     .WithDescription("On: this map uses the time below, not the time of the "
                      "console.")
     .Add("Hour", zone->hour)
     .WithBounds(0, 23)
     .Add("Minute", zone->minute)
     .WithBounds(0, 59);

  app.AddSection("File");
  app.Add("Save To The SD Card", SaveZones)
     .WithDescription("Writes sdmc:/sango/zones.txt. The game applies the "
                      "settings each time that it loads the map.");
  app.Add("Remove The Settings Of This Map", RemoveZone)
     .WithConfirm()
     .WithDescription("The map uses the music, the weather and the time of "
                      "the game again.");
}

} // namespace ui
