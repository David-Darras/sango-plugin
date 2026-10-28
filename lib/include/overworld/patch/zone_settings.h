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
 * @file zone_settings.h
 * @brief The music, the weather and the time of a map.
 */

#pragma once

#include "common.h"

namespace overworld {

/**
 * @brief Gives a map its own music, weather and time of day.
 *
 * When the game loads a map that has settings, the editor applies them with
 * the same tools as the pages Overworld > Music, Weather and Time of Day.
 * When the player goes to a map without settings, the editor gives back the
 * values that these tools had before the first setting was applied.
 *
 * The settings are in the file `sdmc:/sango/zones.txt`. Each line is
 * `map music weather hour minute`; -1 means that the map does not change
 * this value.
 */
class ZoneSettings {
  MAKE_SINGLETON(ZoneSettings)
public:
  static constexpr u32 kMaxZones = 24;

  struct Zone {
    bool is_used = false;
    u16 map = 0;
    bool has_music = false;
    u16 music = 0;
    bool has_weather = false;
    u8 weather = 0;
    bool has_time = false;
    u8 hour = 12;
    u8 minute = 0;
  };

  /// Reads the file.
  static void Initialize();

  /// Called at each frame: repeats the weather for a short time after a map
  /// loads, because the game sets it again while the map starts.
  static void Update();

  /// Called when the game loads the events of a map.
  static void OnMapLoaded(u32 map);

  /// Returns the settings of a map, or null.
  static Zone* Find(u32 map);

  /// Returns the settings of a map. The function makes them when they do not
  /// exist. Returns null when the table is full.
  static Zone* GetOrAdd(u32 map);

  /// Removes the settings of a map and gives back the values of the tools.
  static void Remove(u32 map);

  /// Applies the settings of the current map again (after a change).
  static void Reapply();

  /// Writes the file. Returns false when it cannot be written.
  static bool Save();

private:
  struct Saved {
    bool freeze_music = false;
    u16 music = 0;
    u8 rain_mode = 0;
    bool is_time_enabled = false;
    u8 hour = 12;
    u8 minute = 0;
  };

  static bool LoadLine(const c8* line);
  void Apply(const Zone& zone);
  void Restore();

  Zone zones_[kMaxZones];
  Saved saved_;
  bool is_applied_ = false;
  u16 current_map_ = 0xFFFF;
  u32 weather_frames_ = 0;
  u8 weather_ = 0;
  bool has_weather_ = false;
};

} // namespace overworld
