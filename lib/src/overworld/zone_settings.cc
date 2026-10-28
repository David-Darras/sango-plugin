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
 * @file zone_settings.cc
 * @brief The music, the weather and the time of a map.
 *
 * The declarations are in overworld/patch/zone_settings.h.
 */

#include "overworld/patch/zone_settings.h"

#include <cstdio>

#include "core/native/process_manager.h"
#include "core/patch/music_lock.h"
#include "core/patch/time_override.h"
#include "overworld/native/weather_manager.h"
#include "overworld/patch/overworld.h"
#include "overworld/patch/weather_override.h"
#include "system/native/file.h"
#include "system/native/sound.h"

namespace overworld {

namespace {
const c16 kFolder[] = u"sdmc:/sango";
const c16 kPath[] = u"sdmc:/sango/zones.txt";
constexpr u32 kLineSize = 96;
// The game sets the weather while the map starts: the settings repeat it for
// this number of frames.
constexpr u32 kWeatherFrames = 150;

bool ReadInt(const c8*& p, s32& value) {
  while (*p == ' ') p++;
  bool is_negative = false;
  if (*p == '-') {
    is_negative = true;
    p++;
  }
  if (*p < '0' || *p > '9') return false;
  s32 result = 0;
  while (*p >= '0' && *p <= '9') result = result * 10 + (*p++ - '0');
  value = is_negative ? -result : result;
  return true;
}
} // namespace

ZoneSettings::Zone* ZoneSettings::Find(u32 map) {
  for (Zone& zone : GetInstance().zones_) {
    if (zone.is_used && zone.map == map) return &zone;
  }
  return nullptr;
}

ZoneSettings::Zone* ZoneSettings::GetOrAdd(u32 map) {
  if (Zone* zone = Find(map)) return zone;
  for (Zone& zone : GetInstance().zones_) {
    if (!zone.is_used) {
      zone = Zone();
      zone.is_used = true;
      zone.map = static_cast<u16>(map);
      return &zone;
    }
  }
  return nullptr;
}

void ZoneSettings::Remove(u32 map) {
  auto& self = GetInstance();
  if (Zone* zone = Find(map)) zone->is_used = false;
  if (self.is_applied_ && self.current_map_ == map) {
    self.Restore();
    self.is_applied_ = false;
  }
}

bool ZoneSettings::LoadLine(const c8* line) {
  const c8* p = line;
  s32 map;
  s32 music;
  s32 weather;
  s32 hour;
  s32 minute;
  if (!ReadInt(p, map) || !ReadInt(p, music) || !ReadInt(p, weather) ||
      !ReadInt(p, hour) || !ReadInt(p, minute) || map < 0) {
    return false;
  }
  Zone* zone = GetOrAdd(static_cast<u32>(map));
  if (zone == nullptr) return false;
  zone->has_music = music >= 0;
  zone->music = static_cast<u16>(music < 0 ? 0 : music);
  zone->has_weather = weather >= 0;
  zone->weather = static_cast<u8>(weather < 0 ? 0 : weather);
  zone->has_time = hour >= 0;
  zone->hour = static_cast<u8>(hour < 0 ? 12 : hour);
  zone->minute = static_cast<u8>(minute < 0 ? 0 : minute);
  return true;
}

void ZoneSettings::Initialize() {
  sys::File file;
  file.Open(kPath, sys::File::kRead);
  if (!file.IsOpen()) return;
  c8 chunk[96];
  c8 line[kLineSize];
  u32 length = 0;
  for (;;) {
    const s32 read = file.Read(chunk, sizeof(chunk));
    if (read <= 0) break;
    for (s32 i = 0; i < read; i++) {
      const c8 c = chunk[i];
      if (c == '\n') {
        line[length] = '\0';
        if (length != 0 && line[0] != '#') LoadLine(line);
        length = 0;
      } else if (c != '\r' && length < kLineSize - 1) {
        line[length++] = c;
      }
    }
    if (static_cast<u32>(read) < sizeof(chunk)) break;
  }
  line[length] = '\0';
  if (length != 0 && line[0] != '#') LoadLine(line);
}

bool ZoneSettings::Save() {
  u32 count = 0;
  for (const Zone& zone : GetInstance().zones_) {
    if (zone.is_used) count++;
  }
  if (count == 0) {
    sys::File::Delete(kPath);
    return true;
  }
  sys::File::CreateDirectory(kFolder);
  sys::File file(kPath, true);
  if (!file.IsOpen()) return false;
  static const c8 kHeader[] =
      "# Sango zone settings. map music weather hour minute (-1: no change)\n";
  file.Write(kHeader, sizeof(kHeader) - 1);
  for (const Zone& zone : GetInstance().zones_) {
    if (!zone.is_used) continue;
    c8 line[kLineSize];
    const int length = snprintf(
        line, sizeof(line), "%u %d %d %d %d\n", (unsigned)zone.map,
        zone.has_music ? (int)zone.music : -1,
        zone.has_weather ? (int)zone.weather : -1,
        zone.has_time ? (int)zone.hour : -1, zone.has_time ? (int)zone.minute : -1);
    if (length > 0) file.Write(line, static_cast<u32>(length));
  }
  return true;
}

void ZoneSettings::Restore() {
  auto& music = Overworld::GetInstance();
  music.freeze_background_music = saved_.freeze_music;
  music.background_music = static_cast<BackgroundMusicId>(saved_.music);
  WeatherOverride::GetInstance().mode = static_cast<WeatherMode>(saved_.rain_mode);
  auto& time = core::TimeOverride::GetInstance();
  time.is_enabled = saved_.is_time_enabled;
  time.hour = saved_.hour;
  time.minute = saved_.minute;
  has_weather_ = false;
}

void ZoneSettings::Apply(const Zone& zone) {
  if (!is_applied_) {
    // The first settings: keep the values of the tools.
    auto& music = Overworld::GetInstance();
    saved_.freeze_music = music.freeze_background_music;
    saved_.music = static_cast<u16>(music.background_music);
    saved_.rain_mode = static_cast<u8>(WeatherOverride::GetInstance().mode);
    auto& time = core::TimeOverride::GetInstance();
    saved_.is_time_enabled = time.is_enabled;
    saved_.hour = time.hour;
    saved_.minute = time.minute;
    is_applied_ = true;
  } else {
    Restore();
  }
  if (zone.has_music) {
    auto& music = Overworld::GetInstance();
    music.freeze_background_music = true;
    music.background_music = static_cast<BackgroundMusicId>(zone.music);
    core::MusicLock::Play(music.background_music);
  }
  if (zone.has_time) {
    auto& time = core::TimeOverride::GetInstance();
    time.is_enabled = true;
    time.hour = zone.hour;
    time.minute = zone.minute;
  }
  if (zone.has_weather) {
    weather_ = zone.weather;
    has_weather_ = true;
    weather_frames_ = kWeatherFrames;
  }
}

void ZoneSettings::OnMapLoaded(u32 map) {
  auto& self = GetInstance();
  self.current_map_ = static_cast<u16>(map);
  const Zone* zone = Find(map);
  if (zone != nullptr) {
    self.Apply(*zone);
  } else if (self.is_applied_) {
    self.Restore();
    self.is_applied_ = false;
  }
}

void ZoneSettings::Reapply() {
  auto& self = GetInstance();
  const Zone* zone = Find(self.current_map_);
  if (zone != nullptr) {
    self.Apply(*zone);
  } else if (self.is_applied_) {
    self.Restore();
    self.is_applied_ = false;
  }
}

void ZoneSettings::Update() {
#ifdef GAME_ORAS
  auto& self = GetInstance();
  if (!self.has_weather_ || self.weather_frames_ == 0) return;
  if (!core::ProcessManager::IsOverworldActive()) return;
  self.weather_frames_--;
  if (self.weather_frames_ % 15 != 0) return;
  WeatherManager::GetInstance().SetWeather(static_cast<Weather>(self.weather_));
#endif
}

} // namespace overworld
