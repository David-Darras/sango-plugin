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
 * @file studio_items.h
 * @brief The new items and the changed items of Sango Studio.
 *
 * In the page "Item" of Sango Studio, the user changes the data of the items of
 * the game and makes new items. Sango Studio writes these files in
 * `sdmc:/sango-studio`:
 *
 * - `item/data/[id].bin`: the data of an item of the game (36 bytes, the layout
 *   of pokemon::ItemData). The plugin gives this data to the game in place of
 *   the file of the archive (see core::Archive::RegisterItemData()).
 * - `item/custom/[id].bin`: a new item, with an id from 900 to 931.
 * - `item/custom/[id].bclim`: the icon of a new item (30 x 30 pixels).
 *
 * The layout of the file of a new item (all the numbers are little-endian):
 *
 * | Offset | Size | Content |
 * |---|---|---|
 * | 0 | 4 | The text `SITM`. |
 * | 4 | 1 | The version of the layout: 1. |
 * | 5 | 2 | The id of the item. |
 * | 7 | 1 | The number of rules (the reactions of the item when a Pokémon holds it in a battle). |
 * | 8 | ... | For each of the eight languages: the name, the description, then the message of the Bag when the player uses the item (see battle/patch/studio_abilities.h for the texts). |
 * | ... | 36 | The data of the item (pokemon::ItemData). |
 * | ... | 1 | Bit 0: the player can use the item on a Pokémon in the Bag. |
 * | ... | 2 | The item of the game whose icon the item shows when it has no icon file. 0 for none. |
 * | ... | 1 | The number of actions of the Bag (6 at most). |
 * | ... | 5 each | An action of the Bag: its kind (1 byte), then two signed 16-bit numbers `a` and `b`. |
 * | ... | ... | The rules (see battle/patch/studio_rules.h). |
 *
 * The kinds of the actions of the Bag, with `a` and `b`:
 * 1. changes the HP (`a`: the change; 32767 gives all the HP back; a negative
 *    number removes HP),
 * 2. makes the Pokémon shiny (`a`: 0 switches, 1 makes it shiny, 2 makes it
 *    normal),
 * 3. goes to the next ability of the species,
 * 4. sets the level (`a`),
 * 5. adds EVs (`a`: the stat, 0 HP, 1 Attack, 2 Defense, 3 Speed, 4 Sp. Atk,
 *    5 Sp. Def; `b`: the number).
 *
 * The plugin registers each new item with battle::ItemExtension::AddItem().
 * core::StudioFiles calls this feature, so it works in the overlay and in every
 * product that calls core::StudioFiles::Load(). ORAS only.
 *
 * @see docs/tutorials/15-add-an-item.md
 */

#pragma once

#include "common.h"

namespace battle {

/// Reads the item files of Sango Studio and runs their rules.
class StudioItems {
public:
  /// The first id and the last id of an item of the studio.
  static constexpr u32 kFirstId = 900;
  static constexpr u32 kLastId = 931;
  /// The most new items that the studio can add at the same time.
  static constexpr u32 kMaxItems = 32;
  /// The most changed items of the game.
  static constexpr u32 kMaxChanges = 64;
  static constexpr u32 kMaxBagActions = 6;
  static constexpr u32 kNameSize = 28;
  static constexpr u32 kDescriptionSize = 96;
  static constexpr u32 kMessageSize = 64;

  /// Reads the files `item/custom/900.bin` to `item/custom/931.bin`. Call it
  /// one time after the SD card is ready.
  static void LoadAll();

  /// Reads one file again. `path` is the path in reload.bin, for example
  /// "item/custom/900.bin" or "item/data/30.bin".
  static void Apply(const c8* path);

  /// Takes the texts of the language of the game again when the player changes
  /// the language. Call it often: it is cheap.
  static void RefreshLanguage();
};

} // namespace battle
