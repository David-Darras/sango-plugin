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
 * @file type_names.cc
 * @brief The names of the types in the lists of the menu.
 */

#include <cstdio>

#include "battle/patch/game_extension.h"
#include "ui/main_application.h"

namespace ui {
namespace {
constexpr u32 kGameTypeCount = 18;
constexpr u32 kNameSize = 24;

const c8* const kGameTypeNames[kGameTypeCount] = {
    "Normal", "Fighting", "Flying", "Poison", "Ground", "Rock",
    "Bug",    "Ghost",    "Steel",  "Fire",   "Water",  "Grass",
    "Electric", "Psychic", "Ice",   "Dragon", "Dark",   "Fairy"};
} // namespace

const c8** GetTypeNames(u32* count) {
  static const c8* names[battle::GameExtension::kMaxTypes];
  static c8 texts[battle::GameExtension::kMaxTypes][kNameSize];

  const u32 size = battle::GameExtension::GetTypeListSize();
  for (u32 type = 0; type < size; type++) {
    names[type] = type < kGameTypeCount ? kGameTypeNames[type] : "None";
    const battle::TypeSpec* spec = battle::GameExtension::FindType(type);
    if (type > kGameTypeCount) {
      snprintf(texts[type], kNameSize, "Type %lu", (unsigned long)type);
      names[type] = texts[type];
    }
    if (spec != nullptr && spec->name != nullptr) {
      // The name of the game is UTF-16: the menu shows the plain letters.
      u32 length = 0;
      for (; length < kNameSize - 1 && spec->name[length] != 0; length++) {
        texts[type][length] = spec->name[length] < 128
                                  ? static_cast<c8>(spec->name[length])
                                  : '?';
      }
      texts[type][length] = '\0';
      names[type] = texts[type];
    }
  }
  *count = size;
  return names;
}

} // namespace ui
