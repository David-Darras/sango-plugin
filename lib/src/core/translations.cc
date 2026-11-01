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
 * @file translations.cc
 * @brief The translations of the texts of the ROM hack.
 *
 * The declarations are in core/translations.h.
 */

#include "core/translations.h"

#include "core/data_folder.h"
#include "core/heap_buffer.h"
#include "core/patch/text_editor.h"
#include "core/problems.h"
#include "system/native/core.h"
#include "system/native/file.h"

namespace core {
namespace {
// The text between the English text and the translation.
const c8 kSeparator[] = " => ";
constexpr u32 kSeparatorLength = 4;

u32 Hash(const c16* text) {
  u32 hash = 2166136261u;
  for (u32 i = 0; text[i] != 0; i++) {
    hash ^= text[i];
    hash *= 16777619u;
  }
  return hash;
}

bool IsSame(const c16* a, const c16* b) {
  u32 i = 0;
  for (; a[i] != 0 && b[i] != 0; i++) {
    if (a[i] != b[i]) return false;
  }
  return a[i] == b[i];
}

// The name of the file of a language, or null.
const c16* GetFilePath(Language language) {
  switch (language) {
    case Language::kJapanese: return SANGO_DATA_PATH(u"translations/ja.txt");
    case Language::kEnglish: return SANGO_DATA_PATH(u"translations/en.txt");
    case Language::kFrench: return SANGO_DATA_PATH(u"translations/fr.txt");
    case Language::kItalian: return SANGO_DATA_PATH(u"translations/it.txt");
    case Language::kGerman: return SANGO_DATA_PATH(u"translations/de.txt");
    case Language::kSpanish: return SANGO_DATA_PATH(u"translations/es.txt");
    case Language::kKorean: return SANGO_DATA_PATH(u"translations/ko.txt");
    default: return nullptr;
  }
}

} // namespace

bool Translations::Add(Language language, const c16* english,
                       const c16* translation) {
  if (english == nullptr || translation == nullptr) return false;
  return GetInstance().AddEntry(language, english, translation);
}

bool Translations::AddEntry(Language language, const c16* english,
                            const c16* translation) {
  if (entry_count_ >= kMaxTranslations) {
    Problems::Add(u"translations: more than %u texts", kMaxTranslations);
    return false;
  }
  entries_[entry_count_++] = {language, Hash(english), english, translation};
  return true;
}

const c16* Translations::Translate(const c16* english) {
  if (english == nullptr) return english;
  auto& self = GetInstance();
  const Language language = sys::Core::GetInstance().GetLanguage();
  if (language != self.loaded_language_) {
    self.loaded_language_ = language;
    self.LoadFile(language);
  }
  if (self.entry_count_ == 0) return english;

  const u32 hash = Hash(english);
  for (u32 i = 0; i < self.entry_count_; i++) {
    const Entry& entry = self.entries_[i];
    if (entry.language == language && entry.hash == hash &&
        IsSame(entry.english, english)) {
      return entry.translation;
    }
  }
  return english;
}

void Translations::LoadFile(Language language) {
  const c16* path = GetFilePath(language);
  if (path == nullptr) return;
  // The texts of the files of the other languages stay: each one keeps its
  // language.
  // The text of the file: only while the function reads it.
  core::HeapBuffer buffer(kMaxFileSize);
  if (!buffer.IsValid()) return;
  const c8* file_buffer = buffer.As<c8>();
  const u32 size = sys::File::ReadAll(path, buffer.Get(), buffer.Size());
  if (size == 0) return;
  if (size >= buffer.Size()) {
    Problems::Add(u"translations: the file is too big (%u bytes)",
                  kMaxFileSize);
  }

  u32 start = 0;
  for (u32 i = 0; i <= size; i++) {
    if (i < size && file_buffer[i] != '\n') continue;
    u32 length = i - start;
    if (length > 0 && file_buffer[start + length - 1] == '\r') length--;
    if (!LoadLine(language, file_buffer + start, length)) return;
    start = i + 1;
  }
}

bool Translations::LoadLine(Language language, const c8* line, u32 length) {
  // A UTF-8 file can start with a mark of 3 bytes.
  if (length >= 3 && static_cast<u8>(line[0]) == 0xEF &&
      static_cast<u8>(line[1]) == 0xBB && static_cast<u8>(line[2]) == 0xBF) {
    line += 3;
    length -= 3;
  }
  if (length == 0 || line[0] == '#') return true;

  u32 separator = length;
  for (u32 i = 0; i + kSeparatorLength <= length; i++) {
    bool found = true;
    for (u32 k = 0; k < kSeparatorLength; k++) {
      if (line[i + k] != kSeparator[k]) found = false;
    }
    if (found) {
      separator = i;
      break;
    }
  }
  if (separator == length) return true;

  const c16* english = Store(line, separator);
  const c16* translation = Store(line + separator + kSeparatorLength,
                                 length - separator - kSeparatorLength);
  if (english == nullptr || translation == nullptr) {
    Problems::Add(u"translations: the texts are too long");
    return false;
  }
  return AddEntry(language, english, translation);
}

const c16* Translations::Store(const c8* text, u32 length) {
  const u32 capacity = kMaxFileSize - text_size_;
  if (capacity < 2) return nullptr;
  c16* out = texts_ + text_size_;
  const u32 count = TextEditor::FromUtf8(text, length, out, capacity);
  if (count + 1 >= capacity) return nullptr;
  out[count] = 0;

  // "\n" in the file is a new line.
  u32 write = 0;
  for (u32 read = 0; read < count; read++) {
    if (out[read] == u'\\' && read + 1 < count && out[read + 1] == u'n') {
      out[write++] = u'\n';
      read++;
    } else {
      out[write++] = out[read];
    }
  }
  out[write] = 0;
  text_size_ += write + 1;
  return out;
}

} // namespace core
