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
 * @file page_text_editor.cc
 * @brief The menu pages of the texts of the game: the list of the last texts
 *        that the game read, then one text with the markers to insert.
 */

#include <cstdio>

#include "core/patch/text_editor.h"
#include "ui/main_application.h"
#include "ui/page/page_common.h"
#include "ui/page/pages.h"

namespace ui {
namespace {
using core::RecentText;
using core::TextEditor;
using core::TextKey;

// The menu has 64 entries at most.
constexpr u32 kListSize = 30;
constexpr u32 kLabelLength = 72;

TextKey list_keys[kListSize];
c8 list_labels[kListSize][kLabelLength];
c8 seen_note[kLabelLength];
c8 change_note[kLabelLength];

// The text of the page, and the values that its entries change.
TextKey selected_key;
bool has_selected = false;
bool is_too_long = false;
c16 marked[TextEditor::kMaxMarked + 1];

// The keyboard of the menu holds 31 characters at most: the page shows the
// text in parts of this size, and joins them again when the player changes
// one. A part is cut at a space or after a marker, never inside a marker.
constexpr u32 kPartSize = 32;
constexpr u32 kPartCut = 26;
constexpr u32 kMaxParts = 24;
c16 parts[kMaxParts][kPartSize];
c8 part_labels[kMaxParts][12];
u32 part_count = 0;

c8 key_note[kLabelLength];
c8 original_note[kLabelLength];

// The custom tag of the page.
u8 tag_group = 0xBD;
u8 tag_index = 0;
u8 tag_param_count = 1;
u16 tag_param_1 = 0;
u16 tag_param_2 = 0;

struct Insert {
  const c8* label;
  const c16* marker;
  const c8* description;
};

const Insert kInserts[] = {
    {"New Line", u"[nl]", "Starts a new line in the box."},
    {"Scroll One Line", u"[line]",
     "Changes the page: the next text scrolls up by one line."},
    {"Clear Page", u"[page]",
     "Changes the page: the box is cleared before the next text."},
    {"Wait 15 Frames", u"[wait:15]", "The text stops for a moment."},
    {"Wait 60 Frames", u"[wait:60]", "The text stops for one second."},
    {"Red", u"[red]", "The next letters are red, until [/color]."},
    {"Green", u"[green]", "The next letters are green, until [/color]."},
    {"Blue", u"[blue]", "The next letters are blue, until [/color]."},
    {"Orange", u"[orange]", "The next letters are orange, until [/color]."},
    {"Pink", u"[pink]", "The next letters are pink, until [/color]."},
    {"Purple", u"[purple]", "The next letters are purple, until [/color]."},
    {"Cyan", u"[cyan]", "The next letters are cyan, until [/color]."},
    {"Yellow", u"[yellow]", "The next letters are yellow, until [/color]."},
    {"Sky Blue", u"[sky]", "The next letters are light blue, until [/color]."},
    {"End Of Color", u"[/color]", "The next letters have the normal color."},
    {"Player Name", u"[tag:0100:0]",
     "A name that the game puts in the text. The number is the place in the "
     "list of names of the text: change it to use another name."},
};

c16 tag_marker[40];

// Cuts the text into the parts of the page. The last part is empty, so that
// the player can add text at the end.
void SplitMarked() {
  u32 length = 0;
  while (marked[length] != 0) length++;
  part_count = 0;
  u32 position = 0;
  while (position < length && part_count < kMaxParts) {
    u32 end = position + kPartCut;
    if (end >= length) {
      end = length;
    } else {
      u32 open = 0xFFFFFFFFu;
      for (u32 i = position; i < end; i++) {
        if (marked[i] == u'[') open = i;
        else if (marked[i] == u']') open = 0xFFFFFFFFu;
      }
      if (open != 0xFFFFFFFFu && open > position) {
        end = open;
      } else if (open != 0xFFFFFFFFu) {
        // The marker starts the part: keep it whole when it fits.
        u32 close = open;
        while (close < length && marked[close] != u']') close++;
        end = close < length ? close + 1 : length;
        if (end - position > kPartSize - 1) end = position + kPartSize - 1;
      } else {
        for (u32 i = end; i > position + 8; i--) {
          if (marked[i - 1] == u' ' || marked[i - 1] == u']') {
            end = i;
            break;
          }
        }
      }
    }
    for (u32 i = position; i < end; i++) parts[part_count][i - position] = marked[i];
    parts[part_count][end - position] = 0;
    part_count++;
    position = end;
  }
  if (position < length) is_too_long = true;
  if (part_count < kMaxParts) parts[part_count++][0] = 0;
}

// Joins the parts into the text. Returns false when the text is too long.
bool JoinParts() {
  u32 total = 0;
  for (u32 p = 0; p < part_count; p++) {
    for (u32 i = 0; parts[p][i] != 0; i++) {
      if (total >= TextEditor::kMaxMarked) return false;
      marked[total++] = parts[p][i];
    }
  }
  marked[total] = 0;
  return true;
}

void ReloadMarked() {
  is_too_long = !TextEditor::ReadMarked(selected_key, marked);
  TextEditor::ReadOriginalPreview(selected_key, original_note + 10,
                                  kLabelLength - 10);
  SplitMarked();
}

// A change on the page: the game gets the new text now. A wrong marker is not
// kept. Returns true when the text is kept.
bool ApplyAndReport() {
  if (!has_selected || is_too_long) return false;
  if (!JoinParts()) {
    MainApplication::GetInstance().ShowToast("The text is too long.");
    return false;
  }
  const c8* error = nullptr;
  if (!TextEditor::Apply(selected_key, marked, &error)) {
    MainApplication::GetInstance().ShowToast(error);
    return false;
  }
  return true;
}

void ApplyText(void*) { ApplyAndReport(); }

void AppendMarker(const c16* marker) {
  if (!has_selected || is_too_long) return;
  if (!JoinParts()) {
    MainApplication::GetInstance().ShowToast("The text is too long.");
    return;
  }
  u32 length = 0;
  while (marked[length] != 0) length++;
  u32 add = 0;
  while (marker[add] != 0) add++;
  if (length + add >= TextEditor::kMaxMarked) {
    MainApplication::GetInstance().ShowToast("The text is too long.");
    return;
  }
  for (u32 i = 0; i <= add; i++) marked[length + i] = marker[i];
  const c8* error = nullptr;
  if (!TextEditor::Apply(selected_key, marked, &error)) {
    MainApplication::GetInstance().ShowToast(error);
    return;
  }
  // The page reads the text again and cuts it into the parts.
  MainApplication::GetInstance().Refresh();
}

void InsertTag(void*) {
  u32 count = tag_param_count > 2 ? 2 : tag_param_count;
  c8 text[40];
  int used = snprintf(text, sizeof(text), "[tag:%02X%02X", (unsigned)tag_group,
                      (unsigned)tag_index);
  if (count >= 1) {
    used += snprintf(text + used, sizeof(text) - used, ":%u",
                     (unsigned)tag_param_1);
  }
  if (count >= 2) {
    used += snprintf(text + used, sizeof(text) - used, ":%u",
                     (unsigned)tag_param_2);
  }
  snprintf(text + used, sizeof(text) - used, "]");
  u32 i = 0;
  for (; text[i] != '\0'; i++) tag_marker[i] = static_cast<c16>(text[i]);
  tag_marker[i] = 0;
  AppendMarker(tag_marker);
}

void RestoreOriginal(void*) {
  if (!has_selected) return;
  TextEditor::Restore(selected_key);
  ReloadMarked();
  MainApplication::GetInstance().Refresh();
}

void SaveChanges(void*) {
  MainApplication::GetInstance().ShowToast(
      TextEditor::Save() ? "Saved to the SD card." : "The SD card refused.");
}

void RemoveAllChanges(void*) {
  TextEditor::RemoveAll();
  has_selected = false;
  MainApplication::GetInstance().Refresh();
}

void MakeKeyNote(const TextKey& key) {
  if (key.file == TextEditor::kScriptFile) {
    snprintf(key_note, kLabelLength, "A text of a script");
  } else {
    snprintf(key_note, kLabelLength, "Text file %lu, number %lu",
             (unsigned long)key.file, (unsigned long)key.id);
  }
}
} // namespace

void LoadTextEditorPage(MainApplication& app, void* args) {
  has_selected = false;
  RecentText recent[kListSize];
  const u32 count = TextEditor::GetRecent(recent, kListSize);

  app.AddSection("Last Texts Of The Game");
  snprintf(seen_note, kLabelLength, "Texts seen: %lu",
           (unsigned long)TextEditor::GetSeenCount());
  app.Add(seen_note)
     .WithDescription("How many texts the game read since the start. 0 means "
                      "that the plugin never saw a text.");
  if (count == 0) {
    app.Add("No text yet.")
       .WithDescription("Close the menu, talk to someone or open a menu of "
                        "the game, then look here again.");
  }
  for (u32 i = 0; i < count; i++) {
    list_keys[i] = recent[i].key;
    c8 preview[40];
    TextEditor::ReadOriginalPreview(recent[i].key, preview, sizeof(preview));
    if (recent[i].key.file == TextEditor::kScriptFile) {
      snprintf(list_labels[i], kLabelLength, "%sScript: %s",
               recent[i].is_changed ? "* " : "", preview);
    } else {
      snprintf(list_labels[i], kLabelLength, "%s%lu/%lu: %s",
               recent[i].is_changed ? "* " : "",
               (unsigned long)recent[i].key.file,
               (unsigned long)recent[i].key.id, preview);
    }
    app.Add(list_labels[i], LoadTextPage, &list_keys[i])
       .WithDescription("Edit this text. A star means that it is changed. "
                        "The numbers are the text file and the text.");
  }

  app.AddSection("Changes");
  snprintf(change_note, kLabelLength, "Changed texts: %lu",
           (unsigned long)TextEditor::GetChangeCount());
  app.Add(change_note)
     .Add("Save To The SD Card", SaveChanges)
     .WithDescription("Writes the changes to sdmc:/sango/texts.txt. The game "
                      "reads them again at the next start.")
     .Add("Remove All Changes", RemoveAllChanges)
     .WithConfirm()
     .WithDescription("The game uses its own texts again. The file on the "
                      "SD card stays until you save.");
}

void LoadTextPage(MainApplication& app, void* args) {
  if (args != nullptr) {
    selected_key = *static_cast<TextKey*>(args);
    has_selected = true;
    snprintf(original_note, kLabelLength, "Original: ");
    ReloadMarked();
  }
  if (!has_selected) {
    app.Add("This text is not known any more.")
       .WithDescription("Go back and open the list again.");
    return;
  }
  MakeKeyNote(selected_key);

  app.OnChange(ApplyText);
  app.AddSection("Text");
  app.Add(key_note);
  app.Add(original_note);
  if (is_too_long) {
    app.Add("This text is too long to edit here.")
       .WithDescription("The readable form has more than 254 characters.");
    return;
  }
  // The keyboard holds 31 characters: the text is in parts, joined again at
  // each change. The last part is empty, to add text at the end.
  for (u32 p = 0; p < part_count; p++) {
    snprintf(part_labels[p], sizeof(part_labels[p]), "Part %lu",
             (unsigned long)(p + 1));
    app.Add(part_labels[p], parts[p], kPartSize)
       .WithDescription("A part of the text, with markers between brackets: "
                        "[nl] is a new line, [red] ... [/color] a color. A "
                        "marker that does not exist is refused. A part holds "
                        "31 characters at most: write the rest in the next "
                        "part.");
  }
  app.Add("Restore Original", RestoreOriginal)
     .WithDescription("The game uses its own text again.");

  app.AddSection("Insert At The End");
  for (const Insert& insert : kInserts) {
    const c16* marker = insert.marker;
    app.Add(insert.label, [marker](void*) { AppendMarker(marker); })
       .WithDescription(insert.description);
  }

  app.AddSection("Custom Marker")
     .Add("Group", tag_group)
     .WithDescription("The first byte of the code of the tag. 189 (BD) is "
                      "the general controls, 190 (BE) the controls of the "
                      "message, 1 the names, 2 the numbers.")
     .Add("Index", tag_index)
     .WithDescription("The second byte of the code. For the group 189: 0 "
                      "starts a color, 1 ends it, 2 centers the text.")
     .Add("Parameters", tag_param_count)
     .WithDescription("0, 1 or 2 numbers after the code.")
     .Add("Parameter 1", tag_param_1)
     .Add("Parameter 2", tag_param_2)
     .Add("Insert Marker", InsertTag)
     .WithDescription("Adds [tag:HHHH:N] at the end of the text.");
}

} // namespace ui
