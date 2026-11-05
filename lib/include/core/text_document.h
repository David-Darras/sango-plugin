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
 * @file text_document.h
 * @brief A text file of Sango Maker in memory, for the menus that change it.
 */

#pragma once

#include "core/types.h"

namespace core {

/**
 * @brief A data file of Sango Maker (trainers.txt...) that the menus change.
 *
 * The document keeps the text of the file as it is, with its comments, its
 * empty lines and its order. A file is a list of sections: a line [header],
 * then the lines of the section. The functions read and change the lines. The
 * menus build their entries from the lines, and write the lines again when the
 * player changes an entry. Save() writes the file, and keeps a copy of the old
 * file with the extension .bak.
 *
 * A line has 255 characters at most. A document has 24 KB of text at most. The
 * document reads the file when Load() runs and gives the memory back when
 * Unload() runs: a menu loads its document when it opens and unloads it when
 * it closes.
 *
 * @code
 * core::TextDocument document;
 * if (document.Load("trainers.txt")) {
 *   const u32 section = document.AddSection("Route 102 Lass Tiana");
 *   document.SetKey(section, "money", "8");
 *   document.Save();
 * }
 * @endcode
 */
class TextDocument {
public:
  /// The length of a line at most, with the end of the text.
  static constexpr u32 kMaxLine = 256;
  /// The size of the text at most, in bytes.
  static constexpr u32 kMaxText = 24 * 1024;
  /// The value that the search functions give when they find nothing.
  static constexpr u32 kNone = 0xFFFFFFFF;
  /// The number of the lines before the first section (or of the whole file
  /// when it has no section, like rules.txt). The key and the row functions
  /// accept it in place of a section.
  static constexpr u32 kPreamble = 0xFFFFFFFE;

  TextDocument() = default;
  ~TextDocument() { Unload(); }

  TextDocument(const TextDocument&) = delete;
  TextDocument& operator=(const TextDocument&) = delete;

  /// Reads a file of the data folder. A file that does not exist gives an
  /// empty document. Returns false when the heap is full or the file is too
  /// large.
  bool Load(const c8* file_name);

  /// Writes the file. Returns false when it fails.
  bool Save();

  /// Gives the memory back.
  void Unload();

  /// Returns true when the text changed since Load() or Save().
  bool IsChanged() const { return is_changed_; }

  /// Returns the number of lines of the document.
  u32 GetLineCount() const;
  /// Copies a line (without the end of the line). Returns false when the
  /// number is too large.
  bool GetLine(u32 line, c8* out, u32 capacity) const;
  /// Changes a line.
  bool SetLine(u32 line, const c8* text);
  /// Adds a line before the line `line` (the number of lines: at the end).
  bool InsertLine(u32 line, const c8* text);
  /// Removes a line.
  bool RemoveLine(u32 line);

  /// Returns the number of sections.
  u32 GetSectionCount() const;
  /// Returns the number of the line of the header of a section, or kNone.
  u32 GetSectionLine(u32 section) const;
  /// Returns the number of the line after the last line of a section.
  u32 GetSectionEnd(u32 section) const;
  /// Copies the text of the header, without the brackets. Returns false when
  /// the section does not exist.
  bool GetHeader(u32 section, c8* out, u32 capacity) const;
  /// Changes the text of the header.
  bool SetHeader(u32 section, const c8* header);
  /// Adds a section at the end. Returns its number, or kNone when the document
  /// is full.
  u32 AddSection(const c8* header);
  /// Removes a section and its lines.
  bool RemoveSection(u32 section);

  /// Returns the number of the line of the `index`th key `key` of a section
  /// (a line "key = value"), or kNone. The case of the key does not count.
  u32 FindKey(u32 section, const c8* key, u32 index = 0) const;
  /// Copies the value of a key. Returns false when the section has no such
  /// key.
  bool GetValue(u32 section, const c8* key, c8* out, u32 capacity,
                u32 index = 0) const;
  /// Gives a key a value: it changes the line, or adds the line at the end of
  /// the section. An empty value removes the key.
  bool SetKey(u32 section, const c8* key, const c8* value, u32 index = 0);
  /// Removes the `index`th key of a section.
  bool RemoveKey(u32 section, const c8* key, u32 index = 0);

  /// Returns the number of the lines of a section that are not a comment, not
  /// empty, and not a key (the rows: "Pikachu, 5, 8").
  u32 GetRowCount(u32 section) const;
  /// Returns the number of the line of a row of a section, or kNone.
  u32 GetRowLine(u32 section, u32 row) const;
  /// Adds a row at the end of the section.
  bool AddRow(u32 section, const c8* text);

  /// The same for the lines of a section that are not empty and not a
  /// comment, keys included (the lines of a script).
  u32 GetContentLineCount(u32 section) const;
  u32 GetContentLine(u32 section, u32 index) const;

  /// Copies a part of a line: the `index`th text between commas, without the
  /// spaces at the ends. Returns false when the line has no such part.
  static bool GetPart(const c8* line, u32 index, c8* out, u32 capacity);
  /// Returns true when the line is a key: "word = text".
  static bool IsKeyLine(const c8* line);
  /// Returns true when the line is a comment or is empty.
  static bool IsBlankLine(const c8* line);

private:
  // The start of a line in the text, and its length without the end of line.
  bool GetLineRange(u32 line, u32* start, u32* length) const;
  // Replaces `old_length` bytes at `start` with a text.
  bool Replace(u32 start, u32 old_length, const c8* text, u32 length);
  // Returns true when the line is a header.
  bool IsHeaderLine(u32 line) const;
  // The lines of the body of a section: the first, and the end.
  bool GetBody(u32 section, u32* begin, u32* end) const;
  // Adds a line at the end of the body of a section, before its empty lines.
  bool InsertAtEnd(u32 section, const c8* text);

  c8* text_ = nullptr;
  u32 length_ = 0;
  bool is_changed_ = false;
  c8 file_name_[48] = {};
};

} // namespace core
