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
 * @file page_event.cc
 * @brief The menu pages of the event flags and of the script variables.
 *
 * Each page shows 16 values from a first number. The names come from
 * sdmc:/sango/event_names.ini: a creator of a ROM hack writes the names of
 * the flags and of the variables of the hack there.
 *
 * @code
 * [flags]
 * 1740 = First trainer defeated
 * [variables]
 * 12 = Story progress
 * @endcode
 */

#include <cstdio>

#include "core/ini.h"
#include "savedata/native/event_table.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
const c16* const kNamesPath = u"sdmc:/sango/event_names.ini";

// The values that a page shows.
constexpr u32 kLines = 16;
// The characters of the name of an entry.
constexpr u32 kNameLength = 48;

constexpr u32 kFlagCount = sizeof(savedata::EventTable::flag) * 8;
constexpr u32 kVariableCount =
    sizeof(savedata::EventTable::data) / sizeof(savedata::EventTable::data[0]);

const c8* kOffOn[] = {"Off", "On"};

u16 first_flag = 0;
u16 first_variable = 0;
c8 flag_names[kLines][kNameLength];
c8 variable_names[kLines][kNameLength];

// The file of the names. The page reads it the first time, and again with
// "Read The Names Again".
core::Ini& GetNames() {
  static core::Ini ini;
  static bool is_loaded = false;
  if (!is_loaded) {
    is_loaded = true;
    ini.Load(kNamesPath);
  }
  return ini;
}

void ReloadNames(void*) {
  GetNames().Load(kNamesPath);
  MainApplication::GetInstance().Refresh();
  MainApplication::GetInstance().ShowToast("The names of event_names.ini "
                                           "are read again.");
}

// Writes the name of a flag or of a variable: its number, then the name of
// the file when there is one.
void MakeName(c8* out, const c8* section, const c8* fallback, u32 number) {
  c8 key[12];
  snprintf(key, sizeof(key), "%lu", (unsigned long)number);
  const c8* name = GetNames().Get(section, key);
  if (name != nullptr && name[0] != '\0') {
    snprintf(out, kNameLength, "%lu %s", (unsigned long)number, name);
  } else {
    snprintf(out, kNameLength, "%s %lu", fallback, (unsigned long)number);
  }
}
} // namespace

void LoadEventFlagsPage(MainApplication& app, void* args) {
  auto& table = savedata::EventTable::GetInstance();
  // The flags are bits: flag n is the bit n % 32 of the word n / 32.
  u32* words = reinterpret_cast<u32*>(table.flag);
  if (first_flag > kFlagCount - kLines) first_flag = kFlagCount - kLines;

  app.Add("First Flag", first_flag)
     .WithBounds(0, kFlagCount - kLines)
     .WithRefresh()
     .WithDescription("The page shows 16 flags from this number. Type a "
                      "number, or hold Left / Right.")
     .Add("Read The Names Again", ReloadNames)
     .WithDescription("Reads sdmc:/sango/event_names.ini again: section "
                      "[flags], one line \"number = name\" for each flag.")
     .AddSection("Flags");
  for (u32 i = 0; i < kLines; i++) {
    const u32 flag = first_flag + i;
    MakeName(flag_names[i], "flags", "Flag", flag);
    app.Add(flag_names[i], &words[flag / 32], flag % 32, 1)
       .WithArray(kOffOn, SIZE(kOffOn));
  }
}

void LoadEventVariablesPage(MainApplication& app, void* args) {
  auto& table = savedata::EventTable::GetInstance();
  if (first_variable > kVariableCount - kLines) {
    first_variable = kVariableCount - kLines;
  }

  app.Add("First Variable", first_variable)
     .WithBounds(0, kVariableCount - kLines)
     .WithRefresh()
     .WithDescription("The page shows 16 variables from this number. Type a "
                      "number, or hold Left / Right.")
     .Add("Read The Names Again", ReloadNames)
     .WithDescription("Reads sdmc:/sango/event_names.ini again: section "
                      "[variables], one line \"number = name\".")
     .AddSection("Variables");
  for (u32 i = 0; i < kLines; i++) {
    const u32 variable = first_variable + i;
    MakeName(variable_names[i], "variables", "Variable", variable);
    app.Add(variable_names[i], table.data[variable]);
  }
}
} // namespace ui
