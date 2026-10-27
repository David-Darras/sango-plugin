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
 * @file text_editor.h
 * @brief Changes the texts of the game: the dialogues, the names, the menus.
 */

#pragma once

#include "common.h"

namespace core {

/// The name of a text of the game.
struct TextKey {
  u32 file = 0; ///< The number of the text file, or TextEditor::kScriptFile.
  u32 id = 0;   ///< The number of the text in the file, or a hash of its content.

  bool operator==(const TextKey& other) const {
    return file == other.file && id == other.id;
  }
};

/// A text that the game read lately.
struct RecentText {
  TextKey key;
  bool is_changed = false;
};

/**
 * @brief Changes the texts of the game.
 *
 * The game reads each text with one function (the one that
 * sys::Message::GetString() calls). The editor looks at the result of this
 * function:
 * - it keeps the last texts that the game read, so that the menu can list
 *   them (a dialogue that was just on the screen is in the list);
 * - it replaces the text when the player changed it.
 *
 * A text is the codes of the game, and the tags between them: a tag is
 * `0x0010`, the number of the next codes, the code of the tag, then its
 * parameters. The tags change the color of the text, wait, change the page...
 * The player edits a readable form of the text, with markers between
 * brackets: `[red]`, `[/color]`, `[line]`, `[page]`, `[wait:30]`,
 * `[tag:0100:0]`. The editor converts the two forms.
 *
 * A text of a file is named by the number of the file and the number of the
 * text. A text that a script shows is read from memory, not from a file: the
 * editor names it by a hash of its content, so a change is for each text with
 * this content.
 */
class TextEditor {
  MAKE_SINGLETON(TextEditor)
public:
  /// The longest text, without the end character. A longer text is not kept.
  static constexpr u32 kMaxLength = 160;
  /// The size of the buffer of the readable form, with the end character.
  static constexpr u32 kMaxMarked = 255;
  static constexpr u32 kMaxRecent = 40;
  static constexpr u32 kMaxChanges = 48;
  /// The `file` of the texts that a script shows.
  static constexpr u32 kScriptFile = 0xFFFF;

  /// Reads the changes of the SD card.
  static void Initialize();

  /// Called by the function that gives a text of the game, after it wrote the
  /// text: keeps the text, and replaces it when the player changed it.
  static void OnString(const Message* message, u32 str_id, String* output);

  /// Writes the lately read texts, the newest first. Returns their number.
  static u32 GetRecent(RecentText* out, u32 max);

  /// The number of texts that the editor saw since the start. 0 means that
  /// the game never called the hook.
  static u32 GetSeenCount();

  /// Writes the readable form of the text (the changed text, or the text of
  /// the game) to `out` (kMaxMarked + 1 characters). Returns false when the
  /// text is not known or is too long.
  static bool ReadMarked(const TextKey& key, c16* out);

  /// Writes the readable form of the text that the game gave (without the
  /// changes), for a title. Returns false when the editor does not know it.
  static bool ReadOriginalPreview(const TextKey& key, c8* out, u32 size);

  /// Changes a text. The game uses the new text the next time that it reads
  /// it. Returns false, with a sentence in `error`, when the text is wrong
  /// (an unknown marker, a text too long, too many changes).
  static bool Apply(const TextKey& key, const c16* marked, const c8** error);

  /// Uses the text of the game again.
  static void Restore(const TextKey& key);
  static bool IsChanged(const TextKey& key);
  static u32 GetChangeCount();
  static void RemoveAll();

  /// Writes the changes to the SD card. Returns false when it fails.
  static bool Save();

  /// Converts the readable form into the codes of the game. `capacity` is the
  /// size of `raw` with the end character.
  static bool Decode(const c16* marked, c16* raw, u32 capacity, u32& length,
                     const c8** error);

  /// Converts the codes of the game into the readable form. Returns false
  /// when `marked` is too short (the text is then cut).
  static bool Encode(const c16* raw, u32 length, c16* marked, u32 size);

  /// Writes the letters of a text to `out` (ASCII only: a character that the
  /// menu cannot draw is a question mark), without the tags.
  static void MakePreview(const c16* raw, u32 length, c8* out, u32 size);

private:
  struct Recent {
    TextKey key;
    u32 stamp = 0; ///< 0: the slot is free.
    u16 capacity = 0; ///< The size of the buffer that the game gave.
    u16 length = 0;
    c16 text[kMaxLength + 1];
  };

  struct Change {
    bool is_used = false;
    TextKey key;
    u16 length = 0;
    c16 text[kMaxLength + 1];
  };

  Recent* FindRecent(const TextKey& key);
  Change* FindChange(const TextKey& key);

  Recent recent_[kMaxRecent];
  Change changes_[kMaxChanges];
  u32 stamp_ = 0;
  u32 seen_count_ = 0;
};

} // namespace core
