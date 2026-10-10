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
 * @file page_common.h
 * @brief The functions and the texts that several menu pages share.
 */

#pragma once

#include "common.h"
#include "pokemon/constant/form.h"
#include "pokemon/constant/species.h"

namespace ui {
class MainApplication;
void LoadColorPage(MainApplication& app, void* args);
void LoadColor8Page(MainApplication& app, void* args);

/// Closes the menu and loads the current map again.
void RefreshMap(void* args);

/**
 * @brief Returns numbered texts for WithArray(), for example "Box 1" to
 *        "Box 31". The first text has the number 1.
 * @param format The text with one `%u` (for example "Box %u" or "TM%02u").
 *        It must stay in memory.
 * @param count The number of texts (100 at most).
 * @return The texts. They stay in memory.
 */
const c8** GetNumberedNames(const c8* format, u32 count);

/**
 * @brief Writes the abilities of a species and form: the first ability, the
 *        second ability and the hidden ability. For WithSuggestions().
 * @return The number of abilities (without the same ability two times).
 */
u32 GetSpeciesAbilities(SpeciesId species, FormId form, u16* ids,
                        u32 capacity);

/**
 * @brief Writes the moves that a species and form can learn: the moves of
 *        its levels, then its TMs. For WithSuggestions().
 *
 * The function loads the level moves of the species: call it only when the
 * player edits an entry, not when the page loads.
 * @return The number of moves.
 */
u32 GetSpeciesMoves(SpeciesId species, FormId form, u16* ids, u32 capacity);

// The texts of the values of the game, for WithArray(). The index is the
// value. Use SIZE() for the number of texts.

/// Male, Female, Genderless (pokemon::Gender).
extern const c8* kGenderNames[3];
/// The 25 natures (Nature).
extern const c8* kNatureNames[25];
/// The Poké Balls (pokemon::Ball). 0 is None.
extern const c8* kBallNames[26];
/// The languages of a Pokémon (Language).
extern const c8* kLanguageNames[9];
/// The stat stages of a battle: 0 is -6, 6 is the normal stage, 12 is +6.
extern const c8* kStatStageNames[13];
/// The games of generation 6 (the game version of a save or a Pokémon).
extern const c8* kGameVersionNames[28];
/// The four directions (Up, Down, Left, Right).
extern const c8* kSideNames[4];
} // namespace ui
