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
 * @file name_list.h
 * @brief The names of the values of an entry, and the filter of the menu.
 */

#pragma once

#include "common.h"

namespace ui {
class PageItem;

/// Where the names of the values of an entry come from.
enum class NameSource : u8 {
  kNone,
  kArray, ///< The texts of PageItem::WithArray().
  kSpecies, ///< The names of the species (texts of the game).
  kItem, ///< The names of the items (texts of the game).
  kMove, ///< The names of the moves (texts of the game).
  kAbility, ///< The names of the abilities (texts of the game).
};

/**
 * @brief The names of the values of an entry, and the filter that finds a
 *        value from a part of its name.
 *
 * The names of the game come from the text archive of the language of the
 * game: the menu reads one file the first time, then keeps the names. On
 * XY, the menu asks the game for each name.
 *
 * The filter ignores the small and the capital letters and the accents:
 * "eclat" finds "Éclat Météorite". A number finds the values that start
 * with this number: "25" finds Pikachu.
 */
class NameList {
public:
  /// The largest number of values that the filter gives.
  static constexpr u32 kMaxResults = 1024;

  /// Returns where the names of the values of an entry come from.
  static NameSource GetSource(const PageItem& entry);

  /**
   * @brief Writes the name of a value of an entry (UTF-16).
   * @return false when the value has no name: `out` is then the number.
   */
  static bool GetName(const PageItem& entry, u32 value, c16* out,
                      u32 capacity);

  /**
   * @brief Finds the values of an entry whose name contains `query`.
   *
   * The values come in this order: the names that start with `query`, the
   * names with a word that starts with `query`, then the other names. In
   * each group, the values of `first` come first. An empty query gives
   * all the values.
   * @param entry The entry.
   * @param count The number of values of the entry (0 to count - 1).
   * @param query The text that the player typed (UTF-16).
   * @param first The values to show first (the suggestions), or null.
   * @param first_count The number of values of `first`.
   * @param results Receives the values.
   * @param capacity The size of `results` (kMaxResults at most).
   * @return The number of values.
   */
  static u32 Filter(const PageItem& entry, u32 count, const c16* query,
                    const u16* first, u32 first_count, u16* results,
                    u32 capacity);

  /**
   * @brief Writes a text in the form that the filter compares: small
   *        letters, without accents.
   * @return The number of characters of `out`.
   */
  static u32 Fold(const c16* text, c16* out, u32 capacity);
};
} // namespace ui
