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
 * @file button_glyph.h
 * @brief The symbols of the buttons in the texts of the menu.
 */

#pragma once

#include "common.h"

namespace ui {
/**
 * @brief Replaces the marks of the buttons of a text by their symbols.
 *
 * A text of the menu writes a button as {A}, {B}, {X}, {Y}, {L} or {R}: "Press
 * {A} to play it". The menu draws the icons of the buttons of the 3DS (U+E000 to U+E005) when
 * the font has them, then the circled letters of Unicode, then the plain
 * letter.
 *
 * @param text The text (UTF-16).
 * @param buffer Receives the new text, when the text has a mark.
 * @param capacity The size of `buffer`, in characters.
 * @return `text` itself when it has no mark, or `buffer`.
 */
const c16* ExpandButtons(const c16* text, c16* buffer, u32 capacity);
} // namespace ui
