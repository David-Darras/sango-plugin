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
 * @file translations.h
 * @brief The translations of the texts of the ROM hack.
 *
 * Write the texts of your ROM hack in English (u"..."). A translation
 * replaces a text when the game is in an other language: the names and the
 * descriptions of the new moves, abilities, types and items, and the texts of
 * the C++ scripts.
 *
 * Two ways to give a translation:
 * - a file in the data folder: `translations/fr.txt` for French. A
 *   translator changes this file, without code (see below);
 * - in the code: Translations::Add().
 *
 * The file has one translation on each line, in UTF-8: the English text,
 * ` => `, then the translation. `\n` is a new line. A line that starts with `#`
 * is a comment.
 *
 * @code{.unparsed}
 * # The moves
 * Thunderclap => Coup de tonnerre
 * A fast electric attack.\nIt calls the rain. => Une attaque électrique rapide.\nElle appelle la pluie.
 * @endcode
 *
 * The files: ja.txt, en.txt, fr.txt, it.txt, de.txt, es.txt, ko.txt.
 */

#pragma once

#include "common.h"

namespace core {

/// The translations of the texts of the ROM hack.
class Translations {
  MAKE_SINGLETON(Translations)

public:
  /// The number of translations, for all the languages.
  static constexpr u32 kMaxTranslations = 1024;
  /// The size of the translation file, in bytes.
  static constexpr u32 kMaxFileSize = 32 * 1024;

  /**
   * @brief Adds a translation in the code.
   *
   * @code
   * core::Translations::Add(Language::kFrench, u"Thunderclap",
   *                         u"Coup de tonnerre");
   * @endcode
   *
   * The texts must stay in memory: use u"..." texts.
   */
  static bool Add(Language language, const c16* english,
                  const c16* translation);

  /// Returns the translation of a text in the language of the game, or the
  /// text when there is no translation.
  static const c16* Translate(const c16* english);

private:
  struct Entry {
    Language language;
    u32 hash;
    const c16* english;
    const c16* translation;
  };

  /// Reads the file of the language of the game, one time for each language.
  void LoadFile(Language language);
  /// Reads one line of the file. Returns false when the memory is full.
  bool LoadLine(Language language, const c8* line, u32 length);
  /// Copies a UTF-8 text into the memory of the texts, and returns it.
  const c16* Store(const c8* text, u32 length);
  bool AddEntry(Language language, const c16* english,
                const c16* translation);

  Entry entries_[kMaxTranslations] = {};
  u32 entry_count_ = 0;
  c16 texts_[kMaxFileSize] = {};
  u32 text_size_ = 0;
  Language loaded_language_ = Language::kNone;
};

/// Translates a text of the ROM hack. See core::Translations.
INLINE const c16* Translate(const c16* english) {
  return Translations::Translate(english);
}

} // namespace core
