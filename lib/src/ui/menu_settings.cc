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
 * @file menu_settings.cc
 * @brief The settings file of the menu: sdmc:/sango/menu.ini.
 *
 * The file keeps the theme, the effects, the pinned entries and the recent
 * entries. It is an INI file: the player can change it in a text editor.
 * The menu reads it before its first frame, and writes it when the menu
 * closes (only when a setting changed).
 *
 * The declarations are in ui/main_application.h.
 */

#include <cstdio>
#include <cstring>

#include "core/ini.h"
#include "system/native/file.h"
#include "ui/main_application.h"

namespace ui {
namespace {
const c16* const kSettingsFolder = u"sdmc:/sango";
const c16* const kSettingsPath = u"sdmc:/sango/menu.ini";

// The names of the buttons of Theme::keys. The index is the value.
const c8* const kKeyNames[] = {"None", "Left", "Right", "Up", "Down",
                               "A",    "B",    "X",     "Y",  "L",
                               "R",    "ZL",   "ZR",    "Start"};

// The file is 8 KB: it stays out of the stack.
core::Ini& GetIni() {
  static core::Ini ini;
  return ini;
}

// A checksum of the text of the file: the menu writes the file only when
// the text changes.
u32 last_checksum = 0;

u32 GetChecksum(const c8* text, u32 size) {
  u32 hash = 2166136261u;
  for (u32 i = 0; i < size; i++) {
    hash = (hash ^ (u8)text[i]) * 16777619u;
  }
  return hash;
}

u8 ToByte(f32 value) {
  if (value <= 0) return 0;
  if (value >= 1) return 255;
  return (u8)(value * 255.0f + 0.5f);
}

// A color as RRGGBBAA.
u32 ToHex(const Color& color) {
  return (ToByte(color.r) << 24) | (ToByte(color.g) << 16) |
         (ToByte(color.b) << 8) | ToByte(color.a);
}

Color ToColor(u32 hex) {
  return Color(((hex >> 24) & 0xFF) / 255.0f, ((hex >> 16) & 0xFF) / 255.0f,
               ((hex >> 8) & 0xFF) / 255.0f, (hex & 0xFF) / 255.0f);
}

u8 ReadKey(const core::Ini& ini, const c8* key, u8 fallback) {
  const c8* value = ini.Get("theme", key);
  if (value == nullptr) return fallback;
  for (u32 i = 0; i < SIZE(kKeyNames); i++) {
    const c8* name = kKeyNames[i];
    u32 j = 0;
    while (name[j] != '\0' && (value[j] | 0x20) == (name[j] | 0x20)) j++;
    if (name[j] == '\0' && value[j] == '\0') return i;
  }
  return fallback;
}
} // namespace

void MainApplication::LoadSettings() {
  are_settings_loaded_ = 1;
  core::Ini& ini = GetIni();
  if (ini.Load(kSettingsPath)) {
    Theme& theme = theme_;
    theme.text_shadow = ini.GetBool("menu", "text_shadow", theme.text_shadow);
    theme.animations = ini.GetBool("menu", "animations", theme.animations);

    theme.background_color = ToColor(
        ini.GetHex("theme", "background", ToHex(theme.background_color)));
    theme.unselected_text_color = ToColor(
        ini.GetHex("theme", "text", ToHex(theme.unselected_text_color)));
    theme.selected_text_color = ToColor(ini.GetHex(
        "theme", "selected_text", ToHex(theme.selected_text_color)));
    theme.open_sound = ini.GetInt("theme", "open_sound", theme.open_sound);
    theme.close_sound = ini.GetInt("theme", "close_sound", theme.close_sound);
    theme.confirm_sound =
        ini.GetInt("theme", "confirm_sound", theme.confirm_sound);
    theme.next_sound = ini.GetInt("theme", "next_sound", theme.next_sound);
    theme.error_sound = ini.GetInt("theme", "error_sound", theme.error_sound);
    theme.keys[0] = ReadKey(ini, "menu_key_1", theme.keys[0]);
    theme.keys[1] = ReadKey(ini, "menu_key_2", theme.keys[1]);
    theme.keys[2] = ReadKey(ini, "menu_key_3", theme.keys[2]);

    // The shortcuts keep only their way: the menu finds the entries again
    // when the first page loads.
    c8 key[16];
    c8 section[kSectionLength];
    pin_count_ = 0;
    for (u32 i = 0; i < kMaxPins; i++) {
      snprintf(key, sizeof(key), "pin%lu", (unsigned long)(i + 1));
      const c8* path = ini.Get("pins", key);
      if (path == nullptr || path[0] == '\0') continue;
      Shortcut& shortcut = pins_[pin_count_++];
      strncpy(shortcut.path, path, sizeof(shortcut.path) - 1);
      shortcut.path[sizeof(shortcut.path) - 1] = '\0';
      SplitPath(shortcut.path, section, shortcut.name);
      shortcut.is_available = false;
    }
    recent_count_ = 0;
    for (u32 i = 0; i < kMaxRecents; i++) {
      snprintf(key, sizeof(key), "recent%lu", (unsigned long)(i + 1));
      const c8* path = ini.Get("recent", key);
      if (path == nullptr || path[0] == '\0') continue;
      Shortcut& shortcut = recents_[recent_count_++];
      strncpy(shortcut.path, path, sizeof(shortcut.path) - 1);
      shortcut.path[sizeof(shortcut.path) - 1] = '\0';
      SplitPath(shortcut.path, section, shortcut.name);
      shortcut.is_available = false;
    }
  }

  // Measure the current settings: the menu writes the file only after a
  // change.
  SaveSettings();
}

void MainApplication::SaveSettings() {
  // The first call (from LoadSettings) only measures the settings.
  static bool is_first_call = true;
  core::Ini& ini = GetIni();
  ini.Clear();
  ini.AddComment("The settings of the menu of the plugin.");
  ini.AddComment("You can change this file. The menu reads it when the game "
                 "starts.");
  ini.AddComment("Colors: RRGGBBAA in hexadecimal. 1 = on, 0 = off.");

  ini.AddSection("menu");
  ini.SetInt("text_shadow", theme_.text_shadow ? 1 : 0);
  ini.SetInt("animations", theme_.animations ? 1 : 0);

  ini.AddSection("theme");
  ini.SetHex("background", ToHex(theme_.background_color));
  ini.SetHex("text", ToHex(theme_.unselected_text_color));
  ini.SetHex("selected_text", ToHex(theme_.selected_text_color));
  ini.SetInt("open_sound", theme_.open_sound);
  ini.SetInt("close_sound", theme_.close_sound);
  ini.SetInt("confirm_sound", theme_.confirm_sound);
  ini.SetInt("next_sound", theme_.next_sound);
  ini.SetInt("error_sound", theme_.error_sound);
  ini.AddComment("The buttons that open the menu: None, Left, Right, Up, "
                 "Down, A, B, X, Y, L, R, Start.");
  ini.AddComment("None for the three buttons: Start.");
  for (u32 i = 0; i < SIZE(theme_.keys); i++) {
    c8 key[16];
    snprintf(key, sizeof(key), "menu_key_%lu", (unsigned long)(i + 1));
    const u8 value = theme_.keys[i] < SIZE(kKeyNames) ? theme_.keys[i] : 0;
    ini.Set(key, kKeyNames[value]);
  }

  ini.AddSection("pins");
  ini.AddComment("One entry on each line: the pages, [the section], then "
                 "the name of the entry.");
  for (u32 i = 0; i < pin_count_; i++) {
    c8 key[16];
    snprintf(key, sizeof(key), "pin%lu", (unsigned long)(i + 1));
    ini.Set(key, pins_[i].path);
  }
  ini.AddSection("recent");
  for (u32 i = 0; i < recent_count_; i++) {
    c8 key[16];
    snprintf(key, sizeof(key), "recent%lu", (unsigned long)(i + 1));
    ini.Set(key, recents_[i].path);
  }

  const u32 checksum = GetChecksum(ini.GetText(), ini.GetSize());
  if (is_first_call) {
    is_first_call = false;
    last_checksum = checksum;
    return;
  }
  if (checksum == last_checksum) return;
  sys::File::CreateDirectory(kSettingsFolder);
  if (ini.Save(kSettingsPath)) last_checksum = checksum;
}
} // namespace ui
