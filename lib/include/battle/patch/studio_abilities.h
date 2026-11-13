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
 * @file studio_abilities.h
 * @brief The new abilities that Sango Studio makes.
 *
 * In the "New abilities" tab of Sango Studio, the user makes an ability with
 * menus: a name and a description in the eight languages, and reactions. A
 * reaction says "when this happens, if all these conditions are true, do these
 * actions". Sango Studio writes one file for each ability:
 * `sdmc:/sango-studio/ability/<id>.bin`, with an id from 192 to 255.
 *
 * This feature reads the files and registers each ability with
 * battle::GameExtension::AddAbility(). One reaction function, written once in
 * this feature, runs the rules of the file. The user writes no code.
 *
 * core::StudioFiles calls this feature, so it works in the overlay and in
 * every product that calls core::StudioFiles::Load(). A product can still add
 * its own abilities with battle::GameExtension::AddAbility(): use ids that the
 * studio does not use (the overlay uses 248 to 255).
 *
 * The layout of the file (all the numbers are little-endian):
 *
 * | Offset | Size | Content |
 * |---|---|---|
 * | 0 | 4 | The text `SABR`. |
 * | 4 | 1 | The version of the layout: 1. |
 * | 5 | 1 | The id of the ability. |
 * | 6 | 1 | The number of rules. |
 * | 7 | 1 | Always 0. |
 * | 8 | ... | For each of the eight languages (Japanese, Japanese with kanji, English, French, Italian, German, Spanish, Korean): the name, then the description. A text is a 16-bit length (in 16-bit units), then the units. |
 * | ... | ... | The rules. |
 *
 * A rule is a 16-bit moment (a value of battle::MomentKind), the number of
 * conditions (1 byte), the number of actions (1 byte), the conditions (3 bytes
 * each) and the actions (7 bytes each).
 *
 * A condition is its kind (1 byte) and a signed 16-bit argument. An action is
 * its kind (1 byte), its target (1 byte), two signed 16-bit arguments `a` and
 * `b`, and its flags (1 byte; bit 0 shows the banner of the ability).
 *
 * @see docs/tutorials/05-add-an-ability.md
 */

#pragma once

#include "common.h"

namespace battle {

/// Reads the ability files of Sango Studio and runs their rules in battle.
class StudioAbilities {
public:
  /// The first id and the last id of an ability of the studio.
  static constexpr u32 kFirstId = 192;
  static constexpr u32 kLastId = 255;
  /// The most abilities that the studio can add at the same time.
  static constexpr u32 kMaxAbilities = 32;
  /// The limits of one ability. The studio uses the same limits.
  static constexpr u32 kMaxRules = 8;
  static constexpr u32 kMaxConditions = 6;
  static constexpr u32 kMaxActions = 6;
  /// The size of the buffers of the texts, in 16-bit units, with the zero at
  /// the end. A longer text is cut.
  static constexpr u32 kNameSize = 28;
  static constexpr u32 kDescriptionSize = 96;

  /// Reads the files `ability/192.bin` to `ability/255.bin`. Call it one time
  /// after the SD card is ready.
  static void LoadAll();

  /// Reads one file again. `path` is the path in reload.bin, for example
  /// "ability/230.bin". A file that is gone removes the reactions of the
  /// ability: its name stays until the game restarts.
  static void Apply(const c8* path);

  /// Takes the texts of the language of the game again when the player changes
  /// the language. Call it often: it is cheap.
  static void RefreshLanguage();
};

} // namespace battle
