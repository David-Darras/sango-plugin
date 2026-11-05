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
 * @file page_maker_editor.cc
 * @brief The menus that change the data files of Sango Maker.
 *
 * Each file has a menu: the list of its sections, with the buttons to add,
 * change and delete a section, and to save the file. The menu works on the
 * text of the file (core::TextDocument), so the comments of the file stay. The
 * lists of names (species, moves...) come from the files of sdmc:/sango/names
 * (core::NameFile). The page reads them when it needs them and gives the
 * memory back when it closes.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "core/data_files.h"
#include "core/name_file.h"
#include "core/text_document.h"
#include "core/utf8.h"
#include "ui/main_application.h"
#include "ui/page/pages.h"

namespace ui {
namespace {

using core::NameFile;
using core::TextDocument;

constexpr u32 kNone = TextDocument::kNone;

// The files that the menus change.
enum FileKind : u32 {
  kTrainers,
  kEncounters,
  kSpecies,
  kStory,
  kCharacters,
  kRules,
  kShops,
  kItems,
  kMoves,
  kFileCount,
};

const c8* const kFileNames[kFileCount] = {
    "trainers.txt", "encounters.txt", "species.txt", "story.txt",
    "characters.txt", "rules.txt", "shops.txt", "items.txt", "moves.txt"};

// The lists of names.
enum ListId : u8 {
  kNoList,
  kListSpecies,
  kListMove,
  kListItem,
  kListAbility,
  kListNature,
  kListClass,
  kListTrainerModel,
  kListFormat,
  kListModel,
  kListMap,
  kListMethod,
  kListTrainer,
  kListType,
  kListCount,
};

const c8* const kListFiles[kListCount] = {
    "", "species", "move", "item", "ability", "nature", "trainerclass",
    "trainermodel", "format", "model", "map", "method", "trainer", "type"};

// The kinds of a value in a field.
enum class Type : u8 { kNumber, kText, kName, kChoice };

const c8* const kYesNo[] = {"no", "yes"};
const c8* const kFacings[] = {"Down", "Up", "Left", "Right"};
const c8* const kCategories[] = {"Physical", "Special", "Status"};

// A value of a line: "key = value" in a section, or a part of a row.
struct Field {
  const c8* label;
  const c8* key;  // The key of the line (null for the part of a row).
  Type type;
  ListId list;
  u32 min;
  u32 max;
  const c8* const* choices;
  u32 choice_count;
  const c8* hint;
};

#define NUMBER(label, key, min, max, hint) \
  {label, key, Type::kNumber, kNoList, min, max, nullptr, 0, hint}
#define TEXT(label, key, hint) \
  {label, key, Type::kText, kNoList, 0, 0, nullptr, 0, hint}
#define NAME(label, key, list, hint) \
  {label, key, Type::kName, list, 0, 0, nullptr, 0, hint}
#define CHOICE(label, key, array, hint) \
  {label, key, Type::kChoice, kNoList, 0, 0, array, \
   sizeof(array) / sizeof(array[0]), hint}

// ---- The fields of each kind of section.

const Field kTrainerFields[] = {
    TEXT("Name", "name", "The name in the battle."),
    NAME("Class", "class", kListClass,
         "The class: the title, the pictures, the music."),
    NAME("3D Model", "model", kListTrainerModel,
         "A model of Pokemon X and Y for the battle."),
    NAME("Format", "format", kListFormat, "Single, Double..."),
    NUMBER("Money", "money", 0, 255, "The prize money."),
};

const Field kClassFields[] = {
    TEXT("Title", "title", "The title in the battle."),
    NAME("Character", "overworld", kListModel, "The character on the map."),
    NAME("2D Picture Of", "picture", kListClass, "The picture of a class."),
    NAME("Introduction Of", "introduction", kListClass,
         "The introduction and the 3D model of a class."),
    NAME("Music Of", "music", kListClass,
         "The music of the sight of a class."),
};

const Field kItemFields[] = {
    NUMBER("Price", "price", 0, 65535, "The price in a Poke Mart."),
    NUMBER("Fling Power", "fling power", 0, 255, "The power of Fling."),
    NUMBER("HP Restore", "hp restore", 0, 255, "The HP that it restores."),
    TEXT("EVs", "evs", "Six numbers: HP, Atk, Def, SpA, SpD, Spe."),
};

const Field kMoveFields[] = {
    NAME("Type", "type", kListType, "The type of the move."),
    CHOICE("Category", "category", kCategories, "Physical, Special or Status."),
    NUMBER("Power", "power", 0, 255, "0 for a status move."),
    TEXT("Accuracy", "accuracy", "0 to 100, or never misses."),
    NUMBER("PP", "pp", 1, 64, "The PP without PP Up."),
    TEXT("Priority", "priority", "-7 to 5 (0 for most of the moves)."),
};

const Field kSpeciesFields[] = {
    TEXT("Stats", "stats", "Six numbers: HP, Atk, Def, SpA, SpD, Spe."),
    TEXT("Types", "types", "One or two types."),
    TEXT("Abilities", "abilities", "Two abilities and a hidden one."),
    NUMBER("Catch Rate", "catchrate", 0, 255, "From 0 to 255."),
};

const Field kRulesFields[] = {
    CHOICE("Items In Battle", "items in battle", kYesNo, "Yes or no."),
    CHOICE("One Mega Evolution", "one mega evolution", kYesNo, "Yes or no."),
    CHOICE("Same Catch Rate", "same catch rate", kYesNo, "Yes or no."),
    CHOICE("Inverse Stats", "inverse stats", kYesNo, "Yes or no."),
    CHOICE("Metronome Only", "metronome only", kYesNo, "Yes or no."),
    CHOICE("Type Helper", "type helper", kYesNo, "Yes or no."),
    TEXT("Experience", "experience", "A number from 0 to 20 (1 is normal)."),
    TEXT("Catch Rate", "catch rate", "A number from 0 to 20 (1 is normal)."),
    TEXT("Money", "money", "A number from 0 to 20 (1 is normal)."),
    CHOICE("Random Gifts", "random gifts", kYesNo, "Yes or no."),
    CHOICE("No EV Limit", "no ev limit", kYesNo, "Yes or no."),
    TEXT("Shiny Rate", "shiny rate", "game, or 1/N: 1/4096, 1/512..."),
    TEXT("Starters", "starters", "Three species with a comma between them."),
};

// The parts of a row.
const Field kEncounterRow[] = {
    NAME("Species", nullptr, kListSpecies, "The wild Pokemon."),
    NUMBER("Minimum Level", nullptr, 1, 100, "From 1 to 100."),
    NUMBER("Maximum Level", nullptr, 1, 100, "From 1 to 100."),
    TEXT("Form", nullptr, "The form (optional)."),
};

const Field kShopRow[] = {
    NAME("Item", nullptr, kListItem, "The item."),
    NUMBER("Price", nullptr, 0, 65535, "The price."),
};

const Field kCharacterRow[] = {
    NAME("Model", nullptr, kListModel, "The look of the character."),
    NUMBER("Tile X", nullptr, 0, 1000, "The position in tiles."),
    NUMBER("Tile Z", nullptr, 0, 1000, "The position in tiles."),
    NUMBER("Height", nullptr, 0, 255, "The height."),
    CHOICE("Facing", nullptr, kFacings, "Where it looks."),
    TEXT("Script Or Trainer", nullptr,
         "script 33000, or trainer Name (the name of a trainer)."),
    TEXT("Sight", nullptr, "sight 4 (a trainer only)."),
};

#define COUNT(array) (sizeof(array) / sizeof(array[0]))

// ---- The memory of the menus.

constexpr u32 kMaxSections = 128;
constexpr u32 kMaxRows = 96;
constexpr u32 kHeaderLength = 56;
constexpr u32 kMaxFields = 16;
constexpr u32 kTextLength = 48;

struct State {
  TextDocument document;
  NameFile lists[kListCount];
  FileKind file = kTrainers;
  bool is_loaded = false;
  u32 section = 0;       // The section that the menu shows.
  u32 row = 0;           // The row that the menu shows.
  u32 pokemon = 0;       // The Pokemon of a trainer that the menu shows.
  u32 add_a = 0;         // Values of the page that adds a section.
  u32 add_b = 0;
  u32 add_c = 0;
  c16 add_text[24] = {};
  // The names of the sections and of the rows, for the pickers.
  c8 header_text[kMaxSections][kHeaderLength];
  const c8* header_names[kMaxSections];
  c8 row_text[kMaxRows][kHeaderLength];
  const c8* row_names[kMaxRows];
};

State* state = nullptr;

// The values of the entries of a page.
struct Slots {
  u32 value[kMaxFields];
  u32 original[kMaxFields];
  c16 text[4][kTextLength];
  c8 raw[kMaxFields][kTextLength + 16];  // The text of the file.
  u32 text_slot[kMaxFields];  // The index of the text of a field.
};
Slots slots;

const u32 kUnset = 0xFFFFFFFF;

State& GetState() {
  if (state == nullptr) state = new (std::nothrow) State();
  return *state;
}

void UnloadAll() {
  if (state == nullptr) return;
  state->document.Unload();
  for (u32 i = 0; i < kListCount; i++) state->lists[i].Unload();
  state->is_loaded = false;
}

// Gives all the memory of the menus back.
void FreeState() {
  UnloadAll();
  delete state;
  state = nullptr;
}

// The rows of a section. A script has all its lines, the other files have the
// lines that are not keys.
u32 CountRows(State& s) {
  return s.file == kStory ? s.document.GetContentLineCount(s.section)
                          : s.document.GetRowCount(s.section);
}

u32 RowLineOf(State& s, u32 row) {
  return s.file == kStory ? s.document.GetContentLine(s.section, row)
                          : s.document.GetRowLine(s.section, row);
}

NameFile* List(ListId id) {
  State& s = GetState();
  if (id == kNoList) return nullptr;
  NameFile& list = s.lists[id];
  if (!list.IsLoaded()) list.Load(kListFiles[id]);
  return list.IsLoaded() ? &list : nullptr;
}

// ---- Text helpers.

u32 ToUtf8(const c16* in, c8* out, u32 capacity) {
  u32 length = 0;
  for (u32 i = 0; in[i] != 0; i++) {
    const u32 c = in[i];
    if (c < 0x80) {
      if (length + 1 >= capacity) break;
      out[length++] = static_cast<c8>(c);
    } else if (c < 0x800) {
      if (length + 2 >= capacity) break;
      out[length++] = static_cast<c8>(0xC0 | (c >> 6));
      out[length++] = static_cast<c8>(0x80 | (c & 0x3F));
    } else {
      if (length + 3 >= capacity) break;
      out[length++] = static_cast<c8>(0xE0 | (c >> 12));
      out[length++] = static_cast<c8>(0x80 | ((c >> 6) & 0x3F));
      out[length++] = static_cast<c8>(0x80 | (c & 0x3F));
    }
  }
  out[length] = 0;
  return length;
}

bool ParseNumber(const c8* text, u32* value) {
  if (text[0] == 0) return false;
  u32 result = 0;
  for (u32 i = 0; text[i] != 0; i++) {
    if (text[i] < '0' || text[i] > '9') return false;
    result = result * 10 + static_cast<u32>(text[i] - '0');
  }
  *value = result;
  return true;
}

bool SameText(const c8* a, const c8* b) {
  while (*a != 0 && *b != 0) {
    c8 x = *a;
    c8 y = *b;
    if (x >= 'A' && x <= 'Z') x = static_cast<c8>(x + 32);
    if (y >= 'A' && y <= 'Z') y = static_cast<c8>(y + 32);
    if (x != y) return false;
    a++;
    b++;
  }
  return *a == *b;
}

// ---- The fields.

// Gives the entries of the fields to the page, from the texts of the file.
// `read` gives the text of a field (false when the file has none).
typedef bool (*ReadFunction)(const Field& field, u32 index, c8* out,
                             u32 capacity);

void FillSlots(const Field* fields, u32 count, ReadFunction read) {
  std::memset(&slots, 0, sizeof(slots));
  u32 texts = 0;
  for (u32 i = 0; i < count && i < kMaxFields; i++) {
    const Field& field = fields[i];
    slots.value[i] = field.min;
    slots.original[i] = field.min;
    c8 text[kTextLength + 16] = {};
    const bool has = read(field, i, text, sizeof(text));
    if (has) std::memcpy(slots.raw[i], text, sizeof(slots.raw[i]));
    switch (field.type) {
      case Type::kNumber: {
        u32 number = 0;
        if (has && ParseNumber(text, &number)) {
          slots.value[i] = number;
          slots.original[i] = number;
        }
        break;
      }
      case Type::kName: {
        NameFile* list = List(field.list);
        u32 number = 0;
        if (has && list != nullptr && list->Find(text, &number)) {
          slots.value[i] = number;
          slots.original[i] = number;
        }
        break;
      }
      case Type::kChoice: {
        for (u32 c = 0; has && c < field.choice_count; c++) {
          if (SameText(field.choices[c], text)) {
            slots.value[i] = c;
            slots.original[i] = c;
          }
        }
        break;
      }
      case Type::kText: {
        if (texts < 4) {
          slots.text_slot[i] = texts;
          core::Utf8ToUtf16(text, static_cast<u32>(std::strlen(text)),
                            slots.text[texts], kTextLength);
          texts++;
          slots.original[i] = 0;
        }
        break;
      }
    }
  }
}

// Adds the entries of the fields to the page.
void AddEntries(MainApplication& app, const Field* fields, u32 count,
                callback_t on_change) {
  for (u32 i = 0; i < count && i < kMaxFields; i++) {
    const Field& field = fields[i];
    switch (field.type) {
      case Type::kNumber:
        app.Add(field.label, slots.value[i])
           .WithBounds(field.min, field.max)
           .WithCallback(on_change);
        break;
      case Type::kName: {
        NameFile* list = List(field.list);
        if (list == nullptr) {
          app.Add(field.label).WithDescription(
              "The names are missing: copy the folder sango/names.");
        } else {
          app.Add(field.label, slots.value[i])
             .WithArray(list->GetNames(), list->GetCount())
             .WithCallback(on_change);
        }
        break;
      }
      case Type::kChoice:
        app.Add(field.label, slots.value[i])
           .WithArray(const_cast<const c8**>(field.choices),
                      field.choice_count)
           .WithCallback(on_change);
        break;
      case Type::kText:
        app.Add(field.label, slots.text[slots.text_slot[i]], kTextLength - 1)
           .WithCallback(on_change);
        break;
    }
    if (field.hint != nullptr) app.WithDescription(field.hint);
  }
}

// The text that a field gives to the file.
void FieldText(const Field& field, u32 i, c8* out, u32 capacity) {
  switch (field.type) {
    case Type::kNumber:
      std::snprintf(out, capacity, "%u", slots.value[i]);
      break;
    case Type::kName: {
      NameFile* list = List(field.list);
      std::snprintf(out, capacity, "%s",
                    list != nullptr ? list->Get(slots.value[i]) : "");
      break;
    }
    case Type::kChoice:
      std::snprintf(out, capacity, "%s",
                    slots.value[i] < field.choice_count
                        ? field.choices[slots.value[i]]
                        : "");
      break;
    case Type::kText:
      ToUtf8(slots.text[slots.text_slot[i]], out, capacity);
      break;
  }
}

// True when the player changed the field.
bool IsChanged(const Field& field, u32 i) {
  if (field.type == Type::kText) {
    c8 now[kTextLength + 16];
    FieldText(field, i, now, sizeof(now));
    return std::strcmp(now, slots.raw[i]) != 0;
  }
  return slots.value[i] != slots.original[i];
}

// Writes the changed fields of a key section to the file.
void ApplyKeys(const Field* fields, u32 count, u32 section) {
  State& s = GetState();
  for (u32 i = 0; i < count && i < kMaxFields; i++) {
    if (!IsChanged(fields[i], i)) continue;
    c8 text[kTextLength + 16];
    FieldText(fields[i], i, text, sizeof(text));
    s.document.SetKey(section, fields[i].key, text);
    if (fields[i].type == Type::kText) {
      std::memcpy(slots.raw[i], text, sizeof(slots.raw[i]));
    } else {
      slots.original[i] = slots.value[i];
    }
  }
}

// Writes a row: the parts that the player did not change keep their text.
void ApplyRow(const Field* fields, u32 count, u32 line) {
  State& s = GetState();
  c8 text[TextDocument::kMaxLine] = {};
  u32 last = 0;
  for (u32 i = 0; i < count && i < kMaxFields; i++) {
    c8 part[kTextLength + 16];
    if (IsChanged(fields[i], i) || slots.raw[i][0] == 0) {
      FieldText(fields[i], i, part, sizeof(part));
    } else {
      std::snprintf(part, sizeof(part), "%s", slots.raw[i]);
    }
    if (part[0] != 0) last = i + 1;
    std::strncat(text, part, sizeof(text) - std::strlen(text) - 1);
    std::strncat(text, i + 1 < count ? ", " : "", sizeof(text) - std::strlen(text) - 1);
  }
  // Remove the empty parts at the end.
  u32 length = static_cast<u32>(std::strlen(text));
  (void)last;
  while (length > 0 && (text[length - 1] == ' ' || text[length - 1] == ',')) {
    text[--length] = 0;
  }
  s.document.SetLine(line, text);
}

// ---- The pages.

void LoadFilePage(MainApplication& app, void* args);
void LoadSectionPage(MainApplication& app, void* args);
void LoadRowPage(MainApplication& app, void* args);
void LoadPokemonPage(MainApplication& app, void* args);
void LoadAddPage(MainApplication& app, void* args);

void OpenPageClosed(menu_callback_t page) {
  // The player leaves the page of a file: the file and the lists go.
  if (page == LoadFilePage) UnloadAll();
  if (page == LoadMakerEditorPage) FreeState();
}

bool EnsureLoaded(FileKind file) {
  State& s = GetState();
  if (state == nullptr) return false;
  if (!s.is_loaded || s.file != file) {
    UnloadAll();
    s.file = file;
    s.section = 0;
    s.row = 0;
    if (!s.document.Load(kFileNames[file])) return false;
    s.is_loaded = true;
  }
  return true;
}

// The names of the sections, for the picker.
u32 FillHeaders() {
  State& s = GetState();
  u32 count = s.document.GetSectionCount();
  if (count > kMaxSections) count = kMaxSections;
  for (u32 i = 0; i < count; i++) {
    c8 header[kHeaderLength];
    if (!s.document.GetHeader(i, header, sizeof(header))) header[0] = 0;
    std::snprintf(s.header_text[i], kHeaderLength, "%s", header);
    s.header_names[i] = s.header_text[i];
  }
  if (s.section >= count) s.section = count == 0 ? 0 : count - 1;
  return count;
}

void Save(void*) {
  State& s = GetState();
  auto& app = MainApplication::GetInstance();
  if (!s.document.Save()) {
    app.ShowToast("The file was not saved");
    return;
  }
  core::DataFiles::Reload();
  app.ShowToast("Saved, the file is read again");
  app.Refresh();
}

void DeleteSection(void*) {
  State& s = GetState();
  s.document.RemoveSection(s.section);
  MainApplication::GetInstance().Refresh();
}

void DiscardChanges(void*) {
  State& s = GetState();
  const FileKind file = s.file;
  s.document.Unload();
  s.is_loaded = false;
  EnsureLoaded(file);
  MainApplication::GetInstance().ShowToast("Changes discarded");
  MainApplication::GetInstance().Refresh();
}

// What a section of the file is, for the menu.
bool IsClassHeader(const c8* header) {
  return std::strncmp(header, "new class ", 10) == 0 ||
         std::strncmp(header, "New class ", 10) == 0 ||
         std::strncmp(header, "new Class ", 10) == 0;
}

// ---- The page of a file.

void LoadFilePage(MainApplication& app, void* args) {
  const FileKind file = static_cast<FileKind>(reinterpret_cast<uptr>(args));
  if (!EnsureLoaded(file)) {
    app.Add("The file is too large or the memory is full.");
    return;
  }
  State& s = GetState();
  if (file == kRules) {
    // A file without sections: the keys are the fields.
    auto read = [](const Field& field, u32, c8* out, u32 capacity) {
      return GetState().document.GetValue(TextDocument::kPreamble, field.key,
                                          out, capacity);
    };
    FillSlots(kRulesFields, COUNT(kRulesFields), read);
    app.AddSection("Rules");
    AddEntries(app, kRulesFields, COUNT(kRulesFields), [](void*) {
      ApplyKeys(kRulesFields, COUNT(kRulesFields), TextDocument::kPreamble);
    });
    app.AddSection("File")
       .Add("Save And Read The File", Save)
       .Add("Discard The Changes", DiscardChanges);
    return;
  }
  const u32 count = FillHeaders();
  app.AddSection(kFileNames[file]);
  if (count == 0) {
    app.Add("The file has no section.");
  } else {
    app.Add("Section", s.section).WithBounds(0, count - 1)
       .WithArray(s.header_names, count)
       .WithDescription("Choose a section. L and R jump in the list.");
    app.Add("Change The Section", LoadSectionPage, args)
       .WithDescription("Opens the values of the section.")
       .Add("Delete The Section", DeleteSection)
       .WithDescription("Removes the section and its lines.");
  }
  app.Add("Add A Section", LoadAddPage, args)
     .WithDescription("Makes a new section.");
  app.AddSection("File")
     .Add("Save And Read The File", Save)
     .WithDescription("Writes the file, with its comments, and reads it "
         "again. The old file stays as a .bak file.")
     .Add("Discard The Changes", DiscardChanges);
  if (s.document.IsChanged()) {
    app.Add("Not saved: the changes are in memory.");
  }
}

// ---- The page of a section.

bool ReadKey(const Field& field, u32, c8* out, u32 capacity) {
  State& s = GetState();
  return s.document.GetValue(s.section, field.key, out, capacity);
}

bool ReadRowPart(const Field&, u32 index, c8* out, u32 capacity) {
  State& s = GetState();
  const u32 line = RowLineOf(s, s.row);
  c8 text[TextDocument::kMaxLine];
  if (line == kNone || !s.document.GetLine(line, text, sizeof(text))) {
    return false;
  }
  return TextDocument::GetPart(text, index, out, capacity);
}

// The AI of a trainer: the line "ai = Casual, Competitive, Strategist".
bool ai_casual = false;
bool ai_competitive = false;
bool ai_strategist = false;

void ApplyAi(void*) {
  State& s = GetState();
  c8 text[48] = {};
  const bool flags[3] = {ai_casual, ai_competitive, ai_strategist};
  const c8* const names[3] = {"Casual", "Competitive", "Strategist"};
  for (u32 i = 0; i < 3; i++) {
    if (!flags[i]) continue;
    if (text[0] != 0) std::strcat(text, ", ");
    std::strcat(text, names[i]);
  }
  s.document.SetKey(s.section, "ai", text);
}

// The items of a trainer: the line "items = A, B".
u16 trainer_items[4] = {};

void ApplyItems(void*) {
  State& s = GetState();
  NameFile* list = List(kListItem);
  if (list == nullptr) return;
  c8 text[TextDocument::kMaxLine] = {};
  for (u32 i = 0; i < 4; i++) {
    if (trainer_items[i] == 0) continue;
    if (text[0] != 0) std::strcat(text, ", ");
    std::strncat(text, list->Get(trainer_items[i]),
                 sizeof(text) - std::strlen(text) - 1);
  }
  s.document.SetKey(s.section, "items", text);
}

void ReadAiAndItems() {
  State& s = GetState();
  c8 text[TextDocument::kMaxLine];
  ai_casual = ai_competitive = ai_strategist = false;
  if (s.document.GetValue(s.section, "ai", text, sizeof(text))) {
    for (u32 i = 0; i < 3; i++) {
      c8 part[24];
      if (!TextDocument::GetPart(text, i, part, sizeof(part))) break;
      if (SameText(part, "casual")) ai_casual = true;
      if (SameText(part, "competitive")) ai_competitive = true;
      if (SameText(part, "strategist")) ai_strategist = true;
    }
  }
  for (u32 i = 0; i < 4; i++) trainer_items[i] = 0;
  NameFile* list = List(kListItem);
  if (list != nullptr &&
      s.document.GetValue(s.section, "items", text, sizeof(text))) {
    for (u32 i = 0; i < 4; i++) {
      c8 part[48];
      u32 value = 0;
      if (!TextDocument::GetPart(text, i, part, sizeof(part))) break;
      if (list->Find(part, &value)) trainer_items[i] = static_cast<u16>(value);
    }
  }
}

void AddPokemon(void*) {
  State& s = GetState();
  u32 count = 0;
  while (s.document.FindKey(s.section, "pokemon", count) != kNone) count++;
  if (count >= 6) {
    MainApplication::GetInstance().ShowToast("6 Pokemon at most");
    return;
  }
  s.document.SetKey(s.section, "pokemon", "Pikachu, level 5", count);
  MainApplication::GetInstance().Refresh();
}

void RemovePokemon(void*) {
  State& s = GetState();
  u32 count = 0;
  while (s.document.FindKey(s.section, "pokemon", count) != kNone) count++;
  if (count > 0) s.document.RemoveKey(s.section, "pokemon", count - 1);
  MainApplication::GetInstance().Refresh();
}

// A row of a section: the row picker and the button to open it.
void AddRowEntries(MainApplication& app, void* args) {
  State& s = GetState();
  u32 count = CountRows(s);
  if (count > kMaxRows) count = kMaxRows;
  for (u32 i = 0; i < count; i++) {
    const u32 line = RowLineOf(s, i);
    c8 text[TextDocument::kMaxLine];
    if (line == kNone || !s.document.GetLine(line, text, sizeof(text))) {
      text[0] = 0;
    }
    std::snprintf(s.row_text[i], kHeaderLength, "%s", text);
    s.row_names[i] = s.row_text[i];
  }
  if (s.row >= count) s.row = count == 0 ? 0 : count - 1;
  app.AddSection("Lines");
  if (count == 0) {
    app.Add("The section has no line.");
  } else {
    app.Add("Line", s.row).WithBounds(0, count - 1)
       .WithArray(s.row_names, count)
       .Add("Change The Line", LoadRowPage, args)
       .Add("Delete The Line", [](void*) {
         State& st = GetState();
         const u32 line = RowLineOf(st, st.row);
         if (line != kNone) st.document.RemoveLine(line);
         MainApplication::GetInstance().Refresh();
       });
  }
  app.Add("Add A Line", [](void*) {
    State& st = GetState();
    const FileKind file = st.file;
    const c8* text = "Pikachu, 5, 5";
    if (file == kShops) text = "Potion, 100";
    if (file == kCharacters) text = "Youngster, 10, 10, 0, Down, script 33000";
    st.document.AddRow(st.section, text);
    MainApplication::GetInstance().Refresh();
  });
}

void LoadSectionPage(MainApplication& app, void* args) {
  State& s = GetState();
  const FileKind file = static_cast<FileKind>(reinterpret_cast<uptr>(args));
  if (state == nullptr || !s.is_loaded) {
    app.Add("The file is not loaded.");
    return;
  }
  c8 header[kHeaderLength] = {};
  s.document.GetHeader(s.section, header, sizeof(header));
  app.AddSection(header);

  switch (file) {
    case kTrainers: {
      if (IsClassHeader(header)) {
        FillSlots(kClassFields, COUNT(kClassFields), ReadKey);
        AddEntries(app, kClassFields, COUNT(kClassFields), [](void*) {
          ApplyKeys(kClassFields, COUNT(kClassFields),
                    GetState().section);
        });
        return;
      }
      FillSlots(kTrainerFields, COUNT(kTrainerFields), ReadKey);
      AddEntries(app, kTrainerFields, COUNT(kTrainerFields), [](void*) {
        ApplyKeys(kTrainerFields, COUNT(kTrainerFields), GetState().section);
      });
      ReadAiAndItems();
      app.AddSection("AI")
         .Add("Casual", ai_casual).WithCallback(ApplyAi)
         .Add("Competitive", ai_competitive).WithCallback(ApplyAi)
         .Add("Strategist", ai_strategist).WithCallback(ApplyAi);
      app.AddSection("Items");
      for (u32 i = 0; i < 4; i++) {
        app.AddItem("Item", trainer_items[i]).WithCallback(ApplyItems);
      }
      app.AddSection("Team");
      u32 count = 0;
      while (s.document.FindKey(s.section, "pokemon", count) != kNone) {
        count++;
      }
      for (u32 i = 0; i < count; i++) {
        c8 line[TextDocument::kMaxLine];
        c8 value[TextDocument::kMaxLine];
        static c8 labels[6][48];
        if (i >= 6) break;
        if (s.document.GetValue(s.section, "pokemon", value, sizeof(value), i)) {
          c8 name[32];
          TextDocument::GetPart(value, 0, name, sizeof(name));
          std::snprintf(labels[i], sizeof(labels[i]), "%u. %s", i + 1, name);
        }
        (void)line;
        app.Add(labels[i], LoadPokemonPage,
                reinterpret_cast<void*>(static_cast<uptr>(i)));
      }
      app.Add("Add A Pokemon", AddPokemon)
         .Add("Remove The Last Pokemon", RemovePokemon);
      return;
    }
    case kItems:
      FillSlots(kItemFields, COUNT(kItemFields), ReadKey);
      AddEntries(app, kItemFields, COUNT(kItemFields), [](void*) {
        ApplyKeys(kItemFields, COUNT(kItemFields), GetState().section);
      });
      return;
    case kMoves:
      FillSlots(kMoveFields, COUNT(kMoveFields), ReadKey);
      AddEntries(app, kMoveFields, COUNT(kMoveFields), [](void*) {
        ApplyKeys(kMoveFields, COUNT(kMoveFields), GetState().section);
      });
      return;
    case kSpecies:
      FillSlots(kSpeciesFields, COUNT(kSpeciesFields), ReadKey);
      AddEntries(app, kSpeciesFields, COUNT(kSpeciesFields), [](void*) {
        ApplyKeys(kSpeciesFields, COUNT(kSpeciesFields), GetState().section);
      });
      app.Add("The learnset and the evolutions are in the web editor.");
      return;
    case kEncounters:
    case kShops:
    case kCharacters:
    case kStory:
      AddRowEntries(app, args);
      return;
    default:
      return;
  }
}

// ---- The page of a row.

void LoadRowPage(MainApplication& app, void* args) {
  State& s = GetState();
  const FileKind file = static_cast<FileKind>(reinterpret_cast<uptr>(args));
  const u32 line = RowLineOf(s, s.row);
  if (line == kNone) {
    app.Add("The line does not exist.");
    return;
  }
  if (file == kStory) {
    // A line of a script: the text of the line.
    static c16 text16[96];
    static c8 original[TextDocument::kMaxLine];
    s.document.GetLine(line, original, sizeof(original));
    core::Utf8ToUtf16(original, static_cast<u32>(std::strlen(original)),
                      text16, 96);
    app.AddSection("Line of the script")
       .Add("Text", text16, 95)
       .WithCallback([](void*) {
         State& st = GetState();
         const u32 row_line = RowLineOf(st, st.row);
         c8 out[TextDocument::kMaxLine];
         ToUtf8(text16, out, sizeof(out));
         if (row_line != kNone) st.document.SetLine(row_line, out);
       })
       .WithDescription("A command of story.txt: say, give, battle trainer...");
    return;
  }
  const Field* fields = kEncounterRow;
  u32 count = COUNT(kEncounterRow);
  if (file == kShops) {
    fields = kShopRow;
    count = COUNT(kShopRow);
  } else if (file == kCharacters) {
    fields = kCharacterRow;
    count = COUNT(kCharacterRow);
  }
  FillSlots(fields, count, ReadRowPart);
  app.AddSection("Line");
  // The callback needs the fields: one function for each kind of row.
  if (file == kShops) {
    AddEntries(app, fields, count, [](void*) {
      ApplyRow(kShopRow, COUNT(kShopRow),
               RowLineOf(GetState(), GetState().row));
    });
  } else if (file == kCharacters) {
    AddEntries(app, fields, count, [](void*) {
      ApplyRow(kCharacterRow, COUNT(kCharacterRow),
               RowLineOf(GetState(), GetState().row));
    });
  } else {
    AddEntries(app, fields, count, [](void*) {
      ApplyRow(kEncounterRow, COUNT(kEncounterRow),
               RowLineOf(GetState(), GetState().row));
    });
  }
}

// ---- The page of a Pokemon of a trainer.

u16 pokemon_species = 0;
u16 pokemon_item = 0;
u8 pokemon_ability = 0;
u8 pokemon_nature = 0;
u16 pokemon_moves[4] = {};
u32 pokemon_level = 5;
bool pokemon_shiny = false;
c16 pokemon_form[24] = {};

void ApplyPokemon(void*) {
  State& s = GetState();
  NameFile* species = List(kListSpecies);
  NameFile* items = List(kListItem);
  NameFile* abilities = List(kListAbility);
  NameFile* natures = List(kListNature);
  NameFile* moves = List(kListMove);
  if (species == nullptr) return;
  c8 text[TextDocument::kMaxLine];
  std::snprintf(text, sizeof(text), "%s, level %u", species->Get(pokemon_species),
                pokemon_level);
  auto add = [&](const c8* word, const c8* value) {
    if (value == nullptr || value[0] == 0) return;
    std::strncat(text, ", ", sizeof(text) - std::strlen(text) - 1);
    std::strncat(text, word, sizeof(text) - std::strlen(text) - 1);
    std::strncat(text, " ", sizeof(text) - std::strlen(text) - 1);
    std::strncat(text, value, sizeof(text) - std::strlen(text) - 1);
  };
  if (pokemon_item != 0 && items != nullptr) add("item", items->Get(pokemon_item));
  if (pokemon_ability != 0 && abilities != nullptr) {
    add("ability", abilities->Get(pokemon_ability));
  }
  if (natures != nullptr) add("nature", natures->Get(pokemon_nature));
  c8 form[48];
  ToUtf8(pokemon_form, form, sizeof(form));
  add("form", form);
  if (moves != nullptr) {
    c8 list[160] = {};
    for (u32 i = 0; i < 4; i++) {
      if (pokemon_moves[i] == 0) continue;
      if (list[0] != 0) std::strcat(list, " / ");
      std::strncat(list, moves->Get(pokemon_moves[i]),
                   sizeof(list) - std::strlen(list) - 1);
    }
    add("moves", list);
  }
  if (pokemon_shiny) {
    std::strncat(text, ", shiny", sizeof(text) - std::strlen(text) - 1);
  }
  s.document.SetKey(s.section, "pokemon", text, s.pokemon);
}

// Reads the line "pokemon = ..." of the trainer.
void ReadPokemon() {
  State& s = GetState();
  NameFile* species = List(kListSpecies);
  NameFile* items = List(kListItem);
  NameFile* abilities = List(kListAbility);
  NameFile* natures = List(kListNature);
  NameFile* moves = List(kListMove);
  pokemon_species = 0;
  pokemon_item = 0;
  pokemon_ability = 0;
  pokemon_nature = 0;
  pokemon_level = 5;
  pokemon_shiny = false;
  pokemon_form[0] = 0;
  for (u32 i = 0; i < 4; i++) pokemon_moves[i] = 0;
  c8 line[TextDocument::kMaxLine];
  if (!s.document.GetValue(s.section, "pokemon", line, sizeof(line),
                           s.pokemon)) {
    return;
  }
  for (u32 i = 0;; i++) {
    c8 part[TextDocument::kMaxLine];
    if (!TextDocument::GetPart(line, i, part, sizeof(part))) break;
    u32 value = 0;
    if (i == 0) {
      if (species != nullptr && species->Find(part, &value)) {
        pokemon_species = static_cast<u16>(value);
      }
      continue;
    }
    // "level 50", "item Leftovers", "moves A / B / C / D", "shiny".
    c8* space = std::strchr(part, ' ');
    const c8* rest = space != nullptr ? space + 1 : "";
    if (space != nullptr) *space = 0;
    if (SameText(part, "level") && ParseNumber(rest, &value)) {
      pokemon_level = value;
    } else if (SameText(part, "item") && items != nullptr &&
               items->Find(rest, &value)) {
      pokemon_item = static_cast<u16>(value);
    } else if (SameText(part, "ability") && abilities != nullptr &&
               abilities->Find(rest, &value)) {
      pokemon_ability = static_cast<u8>(value);
    } else if (SameText(part, "nature") && natures != nullptr &&
               natures->Find(rest, &value)) {
      pokemon_nature = static_cast<u8>(value);
    } else if (SameText(part, "form")) {
      core::Utf8ToUtf16(rest, static_cast<u32>(std::strlen(rest)),
                        pokemon_form, 24);
    } else if (SameText(part, "shiny")) {
      pokemon_shiny = true;
    } else if (SameText(part, "moves") && moves != nullptr) {
      // The moves have a / between them.
      c8 copy[160];
      std::snprintf(copy, sizeof(copy), "%s", rest);
      u32 slot = 0;
      c8* start = copy;
      while (slot < 4 && start != nullptr && *start != 0) {
        c8* slash = std::strchr(start, '/');
        if (slash != nullptr) *slash = 0;
        while (*start == ' ') start++;
        c8* end = start + std::strlen(start);
        while (end > start && end[-1] == ' ') *--end = 0;
        if (moves->Find(start, &value)) {
          pokemon_moves[slot] = static_cast<u16>(value);
        }
        slot++;
        start = slash != nullptr ? slash + 1 : nullptr;
      }
    }
  }
}

void LoadPokemonPage(MainApplication& app, void* args) {
  State& s = GetState();
  s.pokemon = static_cast<u32>(reinterpret_cast<uptr>(args));
  ReadPokemon();
  app.AddSection("Pokemon")
     .AddSpecies("Species", pokemon_species).WithCallback(ApplyPokemon)
     .Add("Level", pokemon_level).WithBounds(1, 100).WithCallback(ApplyPokemon)
     .AddItem("Item", pokemon_item).WithCallback(ApplyPokemon)
     .AddAbility("Ability", pokemon_ability).WithCallback(ApplyPokemon);
  NameFile* natures = List(kListNature);
  if (natures != nullptr) {
    app.Add("Nature", pokemon_nature)
       .WithArray(natures->GetNames(), natures->GetCount())
       .WithCallback(ApplyPokemon);
  }
  app.Add("Form", pokemon_form, 23).WithCallback(ApplyPokemon)
     .Add("Shiny", pokemon_shiny).WithCallback(ApplyPokemon)
     .AddSection("Moves");
  for (u32 i = 0; i < 4; i++) {
    app.AddMove("Move", pokemon_moves[i]).WithCallback(ApplyPokemon);
  }
}

// ---- The page that adds a section.

void AddSectionNow(void*) {
  State& s = GetState();
  c8 header[TextDocument::kMaxLine] = {};
  NameFile* a = nullptr;
  switch (s.file) {
    case kTrainers: {
      if (s.add_a == 0) {  // A trainer of the game.
        a = List(kListTrainer);
        if (a != nullptr) std::snprintf(header, sizeof(header), "%s", a->Get(s.add_b));
      } else if (s.add_a == 1) {  // A new trainer.
        c8 name[48];
        ToUtf8(s.add_text, name, sizeof(name));
        std::snprintf(header, sizeof(header), "new %s", name);
      } else {  // A new class.
        c8 name[48];
        ToUtf8(s.add_text, name, sizeof(name));
        a = List(kListClass);
        std::snprintf(header, sizeof(header), "new class %s, %s", name,
                      a != nullptr ? a->Get(s.add_b) : "Youngster");
      }
      break;
    }
    case kEncounters: {
      NameFile* maps = List(kListMap);
      NameFile* methods = List(kListMethod);
      if (maps == nullptr || methods == nullptr) return;
      std::snprintf(header, sizeof(header), "%s, %s", maps->Get(s.add_b),
                    methods->Get(s.add_c));
      break;
    }
    case kSpecies:
      a = List(kListSpecies);
      if (a != nullptr) std::snprintf(header, sizeof(header), "%s", a->Get(s.add_b));
      break;
    case kItems:
      a = List(kListItem);
      if (a != nullptr) std::snprintf(header, sizeof(header), "%s", a->Get(s.add_b));
      break;
    case kMoves:
      a = List(kListMove);
      if (a != nullptr) std::snprintf(header, sizeof(header), "%s", a->Get(s.add_b));
      break;
    case kCharacters:
      a = List(kListMap);
      if (a != nullptr) std::snprintf(header, sizeof(header), "%s", a->Get(s.add_b));
      break;
    case kShops:
      std::snprintf(header, sizeof(header), "shop %u", s.add_b);
      break;
    case kStory:
      std::snprintf(header, sizeof(header), "%u", s.add_b);
      break;
    default:
      return;
  }
  if (header[0] == 0) return;
  const u32 section = s.document.AddSection(header);
  auto& app = MainApplication::GetInstance();
  if (section == kNone) {
    app.ShowToast("The file is full");
    return;
  }
  s.section = section;
  app.ShowToast("Section added");
}

const c8* const kTrainerKinds[] = {"A trainer of the game", "A new trainer",
                                   "A new class"};

void LoadAddPage(MainApplication& app, void*) {
  State& s = GetState();
  app.AddSection("New Section");
  switch (s.file) {
    case kTrainers: {
      app.Add("Kind", s.add_a).WithArray(const_cast<const c8**>(kTrainerKinds), 3).WithRefresh();
      NameFile* trainers = List(kListTrainer);
      NameFile* classes = List(kListClass);
      if (s.add_a == 0 && trainers != nullptr) {
        app.Add("Trainer", s.add_b)
           .WithArray(trainers->GetNames(), trainers->GetCount());
      } else if (s.add_a != 0) {
        app.Add("Name", s.add_text, 16)
           .WithDescription("The name of the new trainer or of the class.");
        if (s.add_a == 2 && classes != nullptr) {
          app.Add("Copy Of", s.add_b)
             .WithArray(classes->GetNames(), classes->GetCount());
        }
      }
      break;
    }
    case kEncounters: {
      NameFile* maps = List(kListMap);
      NameFile* methods = List(kListMethod);
      if (maps != nullptr) {
        app.Add("Map", s.add_b).WithArray(maps->GetNames(), maps->GetCount());
      }
      if (methods != nullptr) {
        app.Add("Method", s.add_c)
           .WithArray(methods->GetNames(), methods->GetCount());
      }
      break;
    }
    case kSpecies: {
      NameFile* list = List(kListSpecies);
      if (list != nullptr) {
        app.Add("Species", s.add_b).WithArray(list->GetNames(), list->GetCount());
      }
      break;
    }
    case kItems: {
      NameFile* list = List(kListItem);
      if (list != nullptr) {
        app.Add("Item", s.add_b).WithArray(list->GetNames(), list->GetCount());
      }
      break;
    }
    case kMoves: {
      NameFile* list = List(kListMove);
      if (list != nullptr) {
        app.Add("Move", s.add_b).WithArray(list->GetNames(), list->GetCount());
      }
      break;
    }
    case kCharacters: {
      NameFile* list = List(kListMap);
      if (list != nullptr) {
        app.Add("Map", s.add_b).WithArray(list->GetNames(), list->GetCount());
      }
      break;
    }
    case kShops:
      app.Add("Shop Number", s.add_b).WithBounds(0, 9999);
      break;
    case kStory:
      app.Add("Script Number", s.add_b).WithBounds(33000, 33999);
      break;
    default:
      break;
  }
  app.Add("Add The Section", AddSectionNow);
}

} // namespace

// ---- The page that lists the files.

void LoadMakerEditorPage(MainApplication& app, void*) {
  MainApplication::SetPageClosedCallback(OpenPageClosed);
  GetState();
  app.AddSection("Edit The Data Files")
     .Add("trainers.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kTrainers)))
     .WithDescription("The trainers, their teams, the new trainers and the "
         "new classes.")
     .Add("encounters.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kEncounters)))
     .Add("species.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kSpecies)))
     .Add("story.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kStory)))
     .Add("characters.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kCharacters)))
     .Add("rules.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kRules)))
     .Add("shops.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kShops)))
     .Add("items.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kItems)))
     .Add("moves.txt", LoadFilePage,
          reinterpret_cast<void*>(static_cast<uptr>(kMoves)));
}

} // namespace ui
