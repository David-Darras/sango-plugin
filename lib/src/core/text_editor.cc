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
 * @file text_editor.cc
 * @brief Changes the texts of the game: the dialogues, the names, the menus.
 *
 * The declarations are in core/patch/text_editor.h.
 */

#include "core/patch/text_editor.h"

#include <cstdio>

#include "system/native/file.h"

namespace core {

namespace {
const c16 kFolder[] = u"sdmc:/sango";
const c16 kPath[] = u"sdmc:/sango/texts.txt";

// The codes of the tags that the editor knows by name. A tag is the code
// 0x0010, the number of the next codes (the code of the tag and its
// parameters), the code of the tag, then the parameters.
constexpr c16 kTagStart = 0x0010;
constexpr u32 kMaxTagParams = 15;
constexpr u32 kMaxTagText = 40;
constexpr u32 kTagColor = 0xBD00;
constexpr u32 kTagColorEnd = 0xBD01;
constexpr u32 kTagLine = 0xBE00;
constexpr u32 kTagPage = 0xBE01;
constexpr u32 kTagWait = 0xBE02;
constexpr c16 kNewLine = 0x000A;

// The colors of the tag 0xBD00, in the order of the game.
constexpr u32 kColorCount = 9;
const c8* const kColorNames[kColorCount] = {
    "red", "green", "blue", "orange", "pink", "purple", "cyan", "yellow", "sky"};

// The lines of the file are short; the biggest has the numbers and 255
// characters of UTF-8.
constexpr u32 kLineSize = 800;

u32 Hash(const c16* text, u32 length) {
  u32 hash = 2166136261u;
  for (u32 i = 0; i < length; i++) {
    hash ^= text[i];
    hash *= 16777619u;
  }
  return hash;
}

bool IsSame(const c16* a, u32 a_length, const c16* b, u32 b_length) {
  if (a_length != b_length) return false;
  for (u32 i = 0; i < a_length; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

// Writes ASCII text and numbers to a buffer of characters.
struct Writer {
  c16* out;
  u32 size;
  u32 position = 0;
  bool is_ok = true;

  Writer(c16* buffer, u32 capacity) : out(buffer), size(capacity) {}

  void Put(c16 c) {
    if (position + 1 < size) {
      out[position++] = c;
    } else {
      is_ok = false;
    }
  }
  void Text(const c8* text) {
    while (*text != '\0') Put(static_cast<c16>(static_cast<u8>(*text++)));
  }
  void Number(u32 value) {
    c8 digits[12];
    u32 count = 0;
    if (value == 0) digits[count++] = '0';
    while (value != 0) {
      digits[count++] = static_cast<c8>('0' + value % 10);
      value /= 10;
    }
    while (count != 0) Put(static_cast<c16>(digits[--count]));
  }
  void Hex4(u32 value) {
    for (s32 shift = 12; shift >= 0; shift -= 4) {
      Put(static_cast<c16>("0123456789ABCDEF"[(value >> shift) & 15]));
    }
  }
  void End() {
    if (size != 0) out[position < size ? position : size - 1] = 0;
  }
};

bool Equals(const c16* text, u32 length, const c8* literal) {
  u32 i = 0;
  for (; i < length; i++) {
    if (literal[i] == '\0' || text[i] != static_cast<u8>(literal[i])) return false;
  }
  return literal[i] == '\0';
}

bool StartsWith(const c16* text, u32 length, const c8* literal) {
  u32 i = 0;
  for (; literal[i] != '\0'; i++) {
    if (i >= length || text[i] != static_cast<u8>(literal[i])) return false;
  }
  return true;
}

// A number of 1 to 5 digits, 65535 at most.
bool ParseDecimal(const c16* text, u32 length, u32& value) {
  if (length == 0 || length > 5) return false;
  u32 result = 0;
  for (u32 i = 0; i < length; i++) {
    if (text[i] < u'0' || text[i] > u'9') return false;
    result = result * 10 + (text[i] - u'0');
  }
  if (result > 0xFFFF) return false;
  value = result;
  return true;
}

// Exactly 4 hexadecimal digits.
bool ParseHex4(const c16* text, u32 length, u32& value) {
  if (length != 4) return false;
  u32 result = 0;
  for (u32 i = 0; i < 4; i++) {
    const c16 c = text[i];
    u32 digit;
    if (c >= u'0' && c <= u'9') {
      digit = c - u'0';
    } else if (c >= u'A' && c <= u'F') {
      digit = c - u'A' + 10;
    } else if (c >= u'a' && c <= u'f') {
      digit = c - u'a' + 10;
    } else {
      return false;
    }
    result = result * 16 + digit;
  }
  value = result;
  return true;
}

// Adds a tag to the codes. `capacity` has the end character.
bool AddTag(c16* raw, u32& position, u32 capacity, u32 code, const u32* params,
            u32 param_count) {
  if (param_count > kMaxTagParams) return false;
  if (position + 3 + param_count + 1 > capacity) return false;
  raw[position++] = kTagStart;
  raw[position++] = static_cast<c16>(1 + param_count);
  raw[position++] = static_cast<c16>(code);
  for (u32 i = 0; i < param_count; i++) raw[position++] = static_cast<c16>(params[i]);
  return true;
}

// Reads the body of a marker (the text between the brackets) and adds the
// codes. Returns false, with a sentence in `error`, when the marker is wrong.
bool DecodeMarker(const c16* body, u32 length, c16* raw, u32& position,
                  u32 capacity, const c8** error) {
  const c8* too_long = "The text is too long.";
  auto put = [&](c16 c) {
    if (position + 1 >= capacity) return false;
    raw[position++] = c;
    return true;
  };

  if (Equals(body, length, "nl")) {
    if (!put(kNewLine)) { *error = too_long; return false; }
    return true;
  }
  if (Equals(body, length, "lb")) {
    if (!put(u'[')) { *error = too_long; return false; }
    return true;
  }
  if (Equals(body, length, "rb")) {
    if (!put(u']')) { *error = too_long; return false; }
    return true;
  }

  u32 params[kMaxTagParams];
  u32 param_count = 0;
  u32 code = 0;
  bool is_tag = false;

  for (u32 color = 0; color < kColorCount && !is_tag; color++) {
    if (Equals(body, length, kColorNames[color])) {
      code = kTagColor;
      params[param_count++] = color;
      is_tag = true;
    }
  }
  if (!is_tag) {
    if (Equals(body, length, "/color")) {
      code = kTagColorEnd;
    } else if (Equals(body, length, "line")) {
      code = kTagLine;
    } else if (Equals(body, length, "page")) {
      code = kTagPage;
    } else if (StartsWith(body, length, "wait:")) {
      u32 frames;
      if (!ParseDecimal(body + 5, length - 5, frames)) {
        *error = "[wait:N] needs a number of frames.";
        return false;
      }
      code = kTagWait;
      params[param_count++] = frames;
    } else if (StartsWith(body, length, "x:")) {
      u32 value;
      if (!ParseHex4(body + 2, length - 2, value)) {
        *error = "[x:HHHH] needs 4 hexadecimal digits.";
        return false;
      }
      if (!put(static_cast<c16>(value))) { *error = too_long; return false; }
      return true;
    } else if (StartsWith(body, length, "tag:")) {
      if (length < 8 || !ParseHex4(body + 4, 4, code)) {
        *error = "[tag:HHHH] needs 4 hexadecimal digits.";
        return false;
      }
      u32 i = 8;
      while (i < length) {
        if (body[i] != u':') {
          *error = "Write the parameters of a tag as :N.";
          return false;
        }
        u32 end = i + 1;
        while (end < length && body[end] != u':') end++;
        u32 value;
        if (param_count >= kMaxTagParams ||
            !ParseDecimal(body + i + 1, end - i - 1, value)) {
          *error = "A parameter of a tag must be a number.";
          return false;
        }
        params[param_count++] = value;
        i = end;
      }
    } else {
      *error = "Unknown marker. Use the Insert entries.";
      return false;
    }
  }
  if (!AddTag(raw, position, capacity, code, params, param_count)) {
    *error = too_long;
    return false;
  }
  return true;
}

// UTF-8 for the file.
u32 ToUtf8(c16 c, c8* out) {
  if (c < 0x80) {
    out[0] = static_cast<c8>(c);
    return 1;
  }
  if (c < 0x800) {
    out[0] = static_cast<c8>(0xC0 | (c >> 6));
    out[1] = static_cast<c8>(0x80 | (c & 0x3F));
    return 2;
  }
  out[0] = static_cast<c8>(0xE0 | (c >> 12));
  out[1] = static_cast<c8>(0x80 | ((c >> 6) & 0x3F));
  out[2] = static_cast<c8>(0x80 | (c & 0x3F));
  return 3;
}

// Reads a UTF-8 text (the characters over 0xFFFF are skipped) into `out`.
u32 FromUtf8(const c8* in, u32 length, c16* out, u32 capacity) {
  u32 count = 0;
  u32 i = 0;
  while (i < length && count + 1 < capacity) {
    const u8 first = static_cast<u8>(in[i]);
    u32 c;
    u32 extra;
    if (first < 0x80) {
      c = first;
      extra = 0;
    } else if ((first & 0xE0) == 0xC0) {
      c = first & 0x1F;
      extra = 1;
    } else if ((first & 0xF0) == 0xE0) {
      c = first & 0x0F;
      extra = 2;
    } else {
      i++;
      continue;
    }
    if (i + extra >= length && extra != 0) break;
    for (u32 k = 1; k <= extra; k++) {
      c = (c << 6) | (static_cast<u8>(in[i + k]) & 0x3F);
    }
    out[count++] = static_cast<c16>(c);
    i += 1 + extra;
  }
  out[count] = 0;
  return count;
}
} // namespace

TextEditor::Recent* TextEditor::FindRecent(const TextKey& key) {
  for (Recent& recent : recent_) {
    if (recent.stamp != 0 && recent.key == key) return &recent;
  }
  return nullptr;
}

TextEditor::Change* TextEditor::FindChange(const TextKey& key) {
  for (Change& change : changes_) {
    if (change.is_used && change.key == key) return &change;
  }
  return nullptr;
}

void TextEditor::OnString(const Message* message, u32 str_id, String* output) {
  if (message == nullptr || output == nullptr || output->buffer == nullptr ||
      !output->is_initialized) {
    return;
  }
  auto& self = GetInstance();
  self.seen_count_++;
  const u32 capacity = output->capacity;
  const u32 length = output->size;
  if (capacity < 2 || length == 0 || length >= capacity || length > kMaxLength) {
    return;
  }
  const c16* text = output->buffer;

  TextKey key;
  if (message->archive == nullptr) {
    // A text that a script built in memory: the file number is 0 for all of
    // them, so the content names the text.
    key.file = kScriptFile;
    key.id = Hash(text, length);
  } else {
    key.file = message->file_id;
    key.id = str_id;
  }

  Recent* recent = self.FindRecent(key);
  if (recent == nullptr) {
    recent = &self.recent_[0];
    for (Recent& slot : self.recent_) {
      if (slot.stamp == 0) {
        recent = &slot;
        break;
      }
      if (slot.stamp < recent->stamp) recent = &slot;
    }
    recent->key = key;
    recent->capacity = static_cast<u16>(capacity);
    recent->length = static_cast<u16>(length);
    for (u32 i = 0; i < length; i++) recent->text[i] = text[i];
    recent->text[length] = 0;
  }
  recent->stamp = ++self.stamp_;

  const Change* change = self.FindChange(key);
  if (change != nullptr && change->length < capacity) {
    for (u32 i = 0; i < change->length; i++) output->buffer[i] = change->text[i];
    output->buffer[change->length] = 0;
    output->size = change->length;
  }
}

u32 TextEditor::GetRecent(RecentText* out, u32 max) {
  auto& self = GetInstance();
  u32 count = 0;
  u32 limit = 0xFFFFFFFFu;
  while (count < max) {
    const Recent* best = nullptr;
    for (const Recent& recent : self.recent_) {
      if (recent.stamp != 0 && recent.stamp < limit &&
          (best == nullptr || recent.stamp > best->stamp)) {
        best = &recent;
      }
    }
    if (best == nullptr) break;
    out[count].key = best->key;
    out[count].is_changed = self.FindChange(best->key) != nullptr;
    count++;
    limit = best->stamp;
  }
  return count;
}

u32 TextEditor::GetSeenCount() { return GetInstance().seen_count_; }

bool TextEditor::ReadMarked(const TextKey& key, c16* out) {
  auto& self = GetInstance();
  out[0] = 0;
  if (const Change* change = self.FindChange(key)) {
    return Encode(change->text, change->length, out, kMaxMarked + 1);
  }
  if (const Recent* recent = self.FindRecent(key)) {
    return Encode(recent->text, recent->length, out, kMaxMarked + 1);
  }
  return false;
}

bool TextEditor::ReadOriginalPreview(const TextKey& key, c8* out, u32 size) {
  const Recent* recent = GetInstance().FindRecent(key);
  if (recent == nullptr) {
    if (size != 0) out[0] = '\0';
    return false;
  }
  MakePreview(recent->text, recent->length, out, size);
  return true;
}

bool TextEditor::Apply(const TextKey& key, const c16* marked, const c8** error) {
  auto& self = GetInstance();
  c16 raw[kMaxLength + 1];
  u32 length = 0;
  if (!Decode(marked, raw, kMaxLength + 1, length, error)) return false;

  const Recent* recent = self.FindRecent(key);
  if (recent != nullptr) {
    if (length >= recent->capacity) {
      *error = "The text is longer than the room that the game gives.";
      return false;
    }
    if (IsSame(recent->text, recent->length, raw, length)) {
      Restore(key);
      return true;
    }
  }
  Change* change = self.FindChange(key);
  if (change == nullptr) {
    for (Change& slot : self.changes_) {
      if (!slot.is_used) {
        change = &slot;
        break;
      }
    }
    if (change == nullptr) {
      *error = "48 changes at most: remove one first.";
      return false;
    }
    change->is_used = true;
    change->key = key;
  }
  for (u32 i = 0; i < length; i++) change->text[i] = raw[i];
  change->text[length] = 0;
  change->length = static_cast<u16>(length);
  return true;
}

void TextEditor::Restore(const TextKey& key) {
  if (Change* change = GetInstance().FindChange(key)) change->is_used = false;
}

bool TextEditor::IsChanged(const TextKey& key) {
  return GetInstance().FindChange(key) != nullptr;
}

u32 TextEditor::GetChangeCount() {
  u32 count = 0;
  for (const Change& change : GetInstance().changes_) {
    if (change.is_used) count++;
  }
  return count;
}

void TextEditor::RemoveAll() {
  for (Change& change : GetInstance().changes_) change.is_used = false;
}

bool TextEditor::Save() {
  auto& self = GetInstance();
  if (GetChangeCount() == 0) {
    sys::File::Delete(kPath);
    return true;
  }
  sys::File::CreateDirectory(kFolder);
  sys::File file(kPath, true);
  if (!file.IsOpen()) return false;
  static const c8 kHeader[] =
      "# Sango game text changes. Each line is file:id=text.\n";
  file.Write(kHeader, sizeof(kHeader) - 1);
  for (const Change& change : self.changes_) {
    if (!change.is_used) continue;
    c16 marked[kMaxMarked + 1];
    if (!Encode(change.text, change.length, marked, kMaxMarked + 1)) continue;
    c8 line[kLineSize];
    int used = snprintf(line, 32, "%lu:%08lX=", (unsigned long)change.key.file,
                        (unsigned long)change.key.id);
    if (used < 0) continue;
    u32 length = static_cast<u32>(used);
    for (u32 i = 0; marked[i] != 0 && length + 4 < kLineSize; i++) {
      length += ::core::ToUtf8(marked[i], line + length);
    }
    line[length++] = '\n';
    file.Write(line, length);
  }
  return true;
}

namespace {
// One line of the file: file:id=text.
void LoadLine(const c8* line, u32 length) {
  if (length == 0 || line[0] == '#') return;
  u32 i = 0;
  u32 file = 0;
  while (i < length && line[i] >= '0' && line[i] <= '9') {
    file = file * 10 + static_cast<u32>(line[i++] - '0');
  }
  if (i >= length || line[i] != ':') return;
  i++;
  u32 id = 0;
  u32 digits = 0;
  while (i < length) {
    const c8 c = line[i];
    u32 digit;
    if (c >= '0' && c <= '9') {
      digit = static_cast<u32>(c - '0');
    } else if (c >= 'A' && c <= 'F') {
      digit = static_cast<u32>(c - 'A' + 10);
    } else if (c >= 'a' && c <= 'f') {
      digit = static_cast<u32>(c - 'a' + 10);
    } else {
      break;
    }
    id = id * 16 + digit;
    digits++;
    i++;
  }
  if (digits == 0 || i >= length || line[i] != '=') return;
  i++;
  c16 marked[TextEditor::kMaxMarked + 1];
  FromUtf8(line + i, length - i, marked, TextEditor::kMaxMarked + 1);
  TextKey key;
  key.file = file;
  key.id = id;
  const c8* error = nullptr;
  TextEditor::Apply(key, marked, &error);
}
} // namespace

void TextEditor::Initialize() {
  sys::File file;
  file.Open(kPath, sys::File::kRead);
  if (!file.IsOpen()) return;
  c8 chunk[256];
  c8 line[kLineSize];
  u32 line_length = 0;
  for (;;) {
    const s32 read = file.Read(chunk, sizeof(chunk));
    if (read <= 0) break;
    for (s32 i = 0; i < read; i++) {
      const c8 c = chunk[i];
      if (c == '\n') {
        LoadLine(line, line_length);
        line_length = 0;
      } else if (c != '\r' && line_length < kLineSize - 1) {
        line[line_length++] = c;
      }
    }
    if (static_cast<u32>(read) < sizeof(chunk)) break;
  }
  LoadLine(line, line_length);
}

bool TextEditor::Decode(const c16* marked, c16* raw, u32 capacity, u32& length,
                        const c8** error) {
  u32 position = 0;
  u32 i = 0;
  while (marked[i] != 0) {
    const c16 c = marked[i];
    if (c != u'[') {
      if (position + 1 >= capacity) {
        *error = "The text is too long.";
        return false;
      }
      raw[position++] = c;
      i++;
      continue;
    }
    u32 end = i + 1;
    while (marked[end] != 0 && marked[end] != u']' && end - i <= kMaxTagText) {
      end++;
    }
    if (marked[end] != u']') {
      *error = "A marker is not closed: add ].";
      return false;
    }
    if (!DecodeMarker(marked + i + 1, end - i - 1, raw, position, capacity,
                      error)) {
      return false;
    }
    i = end + 1;
  }
  raw[position] = 0;
  length = position;
  return true;
}

bool TextEditor::Encode(const c16* raw, u32 length, c16* marked, u32 size) {
  Writer writer(marked, size);
  u32 i = 0;
  while (i < length) {
    const c16 c = raw[i];
    if (c == kTagStart && i + 1 < length && raw[i + 1] >= 1 &&
        raw[i + 1] <= kMaxTagParams + 1 && i + 2 + raw[i + 1] <= length) {
      const u32 count = raw[i + 1];
      const u32 code = raw[i + 2];
      const c16* params = raw + i + 3;
      const u32 param_count = count - 1;
      if (code == kTagColor && param_count == 1 && params[0] < kColorCount) {
        writer.Put(u'[');
        writer.Text(kColorNames[params[0]]);
        writer.Put(u']');
      } else if (code == kTagColorEnd && param_count == 0) {
        writer.Text("[/color]");
      } else if (code == kTagLine && param_count == 0) {
        writer.Text("[line]");
      } else if (code == kTagPage && param_count == 0) {
        writer.Text("[page]");
      } else if (code == kTagWait && param_count == 1) {
        writer.Text("[wait:");
        writer.Number(params[0]);
        writer.Put(u']');
      } else {
        writer.Text("[tag:");
        writer.Hex4(code);
        for (u32 k = 0; k < param_count; k++) {
          writer.Put(u':');
          writer.Number(params[k]);
        }
        writer.Put(u']');
      }
      i += 2 + count;
      continue;
    }
    if (c == kNewLine) {
      writer.Text("[nl]");
    } else if (c == u'[') {
      writer.Text("[lb]");
    } else if (c == u']') {
      writer.Text("[rb]");
    } else if (c < 0x20) {
      writer.Text("[x:");
      writer.Hex4(c);
      writer.Put(u']');
    } else {
      writer.Put(c);
    }
    i++;
  }
  writer.End();
  return writer.is_ok;
}

u32 TextEditor::ToUtf8(const c16* text, c8* out, u32 capacity) {
  u32 length = 0;
  for (u32 i = 0; text[i] != 0 && length + 4 < capacity; i++) {
    length += ::core::ToUtf8(text[i], out + length);
  }
  if (capacity != 0) out[length] = '\0';
  return length;
}

u32 TextEditor::FromUtf8(const c8* in, u32 length, c16* out, u32 capacity) {
  return ::core::FromUtf8(in, length, out, capacity);
}

void TextEditor::MakePreview(const c16* raw, u32 length, c8* out, u32 size) {
  if (size == 0) return;
  u32 position = 0;
  u32 i = 0;
  while (i < length && position + 1 < size) {
    const c16 c = raw[i];
    if (c == kTagStart && i + 1 < length && raw[i + 1] >= 1 &&
        raw[i + 1] <= kMaxTagParams + 1 && i + 2 + raw[i + 1] <= length) {
      i += 2 + raw[i + 1];
      continue;
    }
    if (c == kNewLine) {
      out[position++] = ' ';
    } else if (c >= 0x20 && c < 0x7F) {
      out[position++] = static_cast<c8>(c);
    } else {
      out[position++] = '?';
    }
    i++;
  }
  out[position] = '\0';
}

} // namespace core
