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
 * @file menu_shortcuts.cc
 * @brief The quick access, the search and the radial menu of the menu.
 *
 * A shortcut keeps the way to its entry as a text, for example
 * "Battle > Settings > [Display] Type Helper": the names of the pages, the
 * section in brackets, then the name of the entry. The menu finds the entry
 * again with this text: it runs the page functions into a second list of
 * entries (LoadScratch), without a change of the screen. The search uses
 * the same method to read all the pages.
 *
 * The declarations are in ui/main_application.h.
 */

#include <cstring>

#include "core/utils.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/sound.h"
#include "ui/main_application.h"

namespace ui {
namespace {
// The text between the names of the pages of a way.
const c8* const kSeparator = " > ";
constexpr u32 kSeparatorLength = 3;

// Adds `text` at the end of `out`. Returns false when there is no space.
bool Append(c8* out, u32 size, const c8* text) {
  const u32 length = strlen(out);
  const u32 add = strlen(text);
  if (length + add + 1 > size) return false;
  memcpy(out + length, text, add + 1);
  return true;
}

c8 ToLower(c8 c) { return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c; }

// Returns true when `text` contains `query` (in small letters).
bool Contains(const c8* text, const c8* query) {
  for (; *text != '\0'; text++) {
    u32 i = 0;
    while (query[i] != '\0' && ToLower(text[i]) == query[i]) i++;
    if (query[i] == '\0') return true;
  }
  return false;
}

// Converts a UTF-16 text into UTF-8 small letters.
void ToQuery(const c16* text, c8* out, u32 size) {
  u32 length = 0;
  for (; *text != 0; text++) {
    const c16 c = *text;
    if (c < 0x80) {
      if (length + 2 > size) break;
      out[length++] = ToLower((c8)c);
    } else if (c < 0x800) {
      if (length + 3 > size) break;
      out[length++] = (c8)(0xC0 | (c >> 6));
      out[length++] = (c8)(0x80 | (c & 0x3F));
    } else {
      if (length + 4 > size) break;
      out[length++] = (c8)(0xE0 | (c >> 12));
      out[length++] = (c8)(0x80 | ((c >> 6) & 0x3F));
      out[length++] = (c8)(0x80 | (c & 0x3F));
    }
  }
  out[length] = '\0';
}

// Copies a text, and cuts it when it is too long.
void Copy(c8* out, u32 size, const c8* text) {
  strncpy(out, text != nullptr ? text : "", size - 1);
  out[size - 1] = '\0';
}

// A page that the search must read.
struct Child {
  menu_callback_t load;
  void* args;
  const c8* name;
};
} // namespace

// The quick access.

MainApplication& MainApplication::AddQuickAccess() {
  // The search and the shortcuts do not read the quick access.
  if (is_indexing_) return *this;

  is_quick_access_ = 1;
  first_pin_ = first_recent_ = 0xFF;

  AddSection("Pinned");
  if (pin_count_ == 0) {
    Add("Press Y on an entry to pin it here")
        .WithDescription("The pinned entries stay here. X shows them in a "
                         "circle from any page. Y on a pin removes it.");
  } else {
    first_pin_ = entries_count_;
    AddShortcuts(pins_, pin_count_);
  }
  if (recent_count_ != 0) {
    AddSection("Recent");
    first_recent_ = entries_count_;
    AddShortcuts(recents_, recent_count_);
  }
  return *this;
}

void MainApplication::AddShortcuts(Shortcut* list, u32 count) {
  for (u32 i = 0; i < count && entries_count_ < kMaxEntries; i++) {
    Shortcut& shortcut = list[i];
    // Find the entry again: its data can be at a new place, and its page
    // can need a different part of the game.
    Frame frames[kMaxContexts];
    u32 depth = 0;
    shortcut.is_available = Resolve(shortcut.path, frames, depth,
                                    &shortcut.item, &shortcut.on_change);

    PageItem& entry = entries_[entries_count_++];
    if (!shortcut.is_available) {
      entry.Initialize(shortcut.name, nullptr, kTypeIdle);
      entry.WithDescription("Not available here. Go to the part of the game "
                            "that this entry needs (for example a battle).");
      continue;
    }
    entry = shortcut.item;
    entry.SetName(shortcut.name);
  }
}

MainApplication::Shortcut* MainApplication::GetShortcut(u32 index) {
  if (!is_quick_access_) return nullptr;
  if (first_pin_ != 0xFF && index >= first_pin_ &&
      index < first_pin_ + pin_count_) {
    return &pins_[index - first_pin_];
  }
  if (first_recent_ != 0xFF && index >= first_recent_ &&
      index < first_recent_ + recent_count_) {
    return &recents_[index - first_recent_];
  }
  return nullptr;
}

bool MainApplication::BuildPath(u32 index, c8* path, u32 size) {
  path[0] = '\0';
  if (is_search_page_ || index >= entries_count_) return false;
  for (u32 i = 1; i < contexts_count_; i++) {
    const c8* title = contexts_[i].title;
    if (title == nullptr) return false;
    if (!Append(path, size, title) || !Append(path, size, kSeparator)) {
      return false;
    }
  }
  // The section of the entry tells apart two entries with the same name.
  for (s32 i = (s32)index - 1; i >= 0; i--) {
    if (entries_[i].IsSelectable()) continue;
    const c8* section = entries_[i].GetName();
    if (section != nullptr && section[0] != '\0') {
      if (!Append(path, size, "[") || !Append(path, size, section) ||
          !Append(path, size, "] ")) {
        return false;
      }
      break;
    }
  }
  const c8* name = entries_[index].GetName();
  if (name == nullptr || name[0] == '\0') return false;
  return Append(path, size, name);
}

void MainApplication::SplitPath(const c8* path, c8* section, c8* name) {
  const c8* last = path;
  for (const c8* found = strstr(path, kSeparator); found != nullptr;
       found = strstr(found + kSeparatorLength, kSeparator)) {
    last = found + kSeparatorLength;
  }
  section[0] = '\0';
  if (last[0] == '[') {
    const c8* end = strchr(last, ']');
    if (end != nullptr) {
      u32 length = end - last - 1;
      if (length > kSectionLength - 1) length = kSectionLength - 1;
      memcpy(section, last + 1, length);
      section[length] = '\0';
      last = end + 1;
      while (*last == ' ') last++;
    }
  }
  Copy(name, kNameLength, last);
}

s32 MainApplication::FindPin(const c8* path) const {
  for (u32 i = 0; i < pin_count_; i++) {
    if (strcmp(pins_[i].path, path) == 0) return i;
  }
  return -1;
}

void MainApplication::TogglePin(u32 index) {
  c8 path[sizeof(Shortcut::path)];
  const Shortcut* shortcut = GetShortcut(index);
  if (shortcut != nullptr) {
    Copy(path, sizeof(path), shortcut->path);
  } else if (!BuildPath(index, path, sizeof(path))) {
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return;
  }

  const s32 pin = FindPin(path);
  const PageItem& entry = entries_[index];
  if (pin >= 0) {
    Remove(pins_, pin_count_, pin);
    sys::Sound::PlaySoundEffect(theme_.close_sound);
  } else if (!entry.IsSelectable() || pin_count_ >= kMaxPins ||
             (entry.GetType() == kTypeIdle && !entry.HasCallback())) {
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return;
  } else {
    Shortcut& added = pins_[pin_count_++];
    Copy(added.path, sizeof(added.path), path);
    c8 section[kSectionLength];
    SplitPath(path, section, added.name);
    added.item = shortcut != nullptr ? shortcut->item : entry;
    added.on_change = shortcut != nullptr ? shortcut->on_change : on_change_;
    added.is_available = true;
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);
  }
  if (is_quick_access_) Refresh();
}

void MainApplication::AddRecent(u32 index) {
  if (is_quick_access_ || is_search_page_ || is_indexing_) return;
  const PageItem& entry = entries_[index];
  if (!entry.IsSelectable() || entry.GetType() == kTypeMenu) return;
  c8 path[sizeof(Shortcut::path)];
  if (!BuildPath(index, path, sizeof(path))) return;

  // The same entry moves to the top of the list.
  for (u32 i = 0; i < recent_count_; i++) {
    if (strcmp(recents_[i].path, path) == 0) {
      Remove(recents_, recent_count_, i);
      break;
    }
  }
  if (recent_count_ >= kMaxRecents) recent_count_ = kMaxRecents - 1;
  for (u32 i = recent_count_; i > 0; i--) recents_[i] = recents_[i - 1];
  Shortcut& added = recents_[0];
  Copy(added.path, sizeof(added.path), path);
  c8 section[kSectionLength];
  SplitPath(path, section, added.name);
  added.item = entry;
  added.on_change = on_change_;
  added.is_available = true;
  recent_count_++;
}

void MainApplication::Remove(Shortcut* list, u8& count, u32 index) {
  if (index >= count) return;
  for (u32 i = index; i + 1 < count; i++) list[i] = list[i + 1];
  count--;
}

// The pages without the screen.

bool MainApplication::LoadScratch(menu_callback_t load, void* args) {
  // Keep the state of the page that the menu shows (or builds now).
  PageItem* const saved_entries = entries_;
  const u32 saved_count = entries_count_;
  const u32 saved_no_background = no_background_;
  const u32 saved_quick_access = is_quick_access_;
  const u32 saved_search_page = is_search_page_;
  const u32 saved_indexing = is_indexing_;
  const uptr saved_vtable = process_vtable_;
  const u8 saved_first_pin = first_pin_;
  const u8 saved_first_recent = first_recent_;
  const callback_t saved_on_change = on_change_;

  entries_ = scratch_entries_;
  entries_count_ = 0;
  is_indexing_ = 1;
  index_failed_ = 0;
  on_change_ = nullptr;
  load(*this, args);
  scratch_count_ = entries_count_;
  scratch_on_change_ = on_change_;
  const bool is_loaded = !index_failed_;

  entries_ = saved_entries;
  entries_count_ = saved_count;
  no_background_ = saved_no_background;
  is_quick_access_ = saved_quick_access;
  is_search_page_ = saved_search_page;
  is_indexing_ = saved_indexing;
  process_vtable_ = saved_vtable;
  first_pin_ = saved_first_pin;
  first_recent_ = saved_first_recent;
  on_change_ = saved_on_change;
  index_failed_ = 0;
  return is_loaded;
}

bool MainApplication::Resolve(const c8* path, Frame* frames, u32& depth,
                              PageItem* item, callback_t* on_change) {
  depth = 0;
  frames[0].load = contexts_[0].load_menu;
  frames[0].args = contexts_[0].args;
  frames[0].title = nullptr;
  if (frames[0].load == nullptr || path[0] == '\0') return false;
  if (!LoadScratch(frames[0].load, frames[0].args)) return false;

  // Open the pages of the way, one after the other.
  const c8* part = path;
  for (const c8* end = strstr(part, kSeparator); end != nullptr;
       end = strstr(part, kSeparator)) {
    const u32 length = end - part;
    s32 found = -1;
    for (u32 i = 0; i < scratch_count_ && found < 0; i++) {
      const PageItem& entry = scratch_entries_[i];
      const c8* name = entry.GetName();
      if (entry.GetType() == kTypeMenu && name != nullptr &&
          strlen(name) == length && memcmp(name, part, length) == 0) {
        found = i;
      }
    }
    if (found < 0 || depth + 1 >= kMaxContexts) return false;
    const PageItem& page = scratch_entries_[found];
    depth++;
    frames[depth].load = (menu_callback_t)page.GetAddress();
    frames[depth].args = page.GetArgs();
    frames[depth].title = page.GetName();
    if (!LoadScratch(frames[depth].load, frames[depth].args)) return false;
    part = end + kSeparatorLength;
  }

  // Then find the entry in its section.
  c8 section[kSectionLength];
  c8 name[kNameLength];
  SplitPath(path, section, name);
  const c8* current_section = "";
  for (u32 i = 0; i < scratch_count_; i++) {
    const PageItem& entry = scratch_entries_[i];
    if (!entry.IsSelectable()) {
      if (entry.GetName() != nullptr && entry.GetName()[0] != '\0') {
        current_section = entry.GetName();
      }
      continue;
    }
    if (entry.GetName() == nullptr || strcmp(entry.GetName(), name) != 0) {
      continue;
    }
    if (section[0] != '\0' && strcmp(current_section, section) != 0) continue;
    if (item != nullptr) *item = entry;
    if (on_change != nullptr) *on_change = scratch_on_change_;
    return true;
  }
  return false;
}

void MainApplication::NavigateTo(const Frame* frames, u32 depth,
                                 const c8* section, const c8* name) {
  // Go back to the first page, then open the pages of the way.
  contexts_count_ = 1;
  if (depth == 0) {
    undo_count_ = 0;
    Refresh();
    StartTransition(1);
  }
  for (u32 i = 1; i <= depth; i++) {
    Open(frames[i].load, frames[i].args, frames[i].title);
    // CheckProcess closed the page: the game is in a different process.
    if (contexts_count_ != i + 1) return;
  }

  const c8* current_section = "";
  for (u32 i = 0; i < entries_count_; i++) {
    const PageItem& entry = entries_[i];
    if (!entry.IsSelectable()) {
      if (entry.GetName() != nullptr && entry.GetName()[0] != '\0') {
        current_section = entry.GetName();
      }
      continue;
    }
    if (entry.GetName() == nullptr || strcmp(entry.GetName(), name) != 0) {
      continue;
    }
    if (section != nullptr && section[0] != '\0' &&
        strcmp(current_section, section) != 0) {
      continue;
    }
    Select(i);
    needs_prepare_ = true;
    return;
  }
}

void MainApplication::OpenShortcut(const Shortcut& shortcut) {
  Frame frames[kMaxContexts];
  u32 depth = 0;
  if (!Resolve(shortcut.path, frames, depth, nullptr, nullptr)) {
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return;
  }
  c8 section[kSectionLength];
  c8 name[kNameLength];
  SplitPath(shortcut.path, section, name);
  sys::Sound::PlaySoundEffect(theme_.confirm_sound);
  NavigateTo(frames, depth, section, name);
}

// The search.

void MainApplication::LoadSearchPage(MainApplication& app, void* args) {
  app.is_search_page_ = 1;
  app.Add("Search", app.search_query_, kQueryLength)
     .WithDescription("Type a part of a name: the results show while you "
                      "type. Press A on a result to go to it.");

  app.AddSection("Results");
  if (app.search_query_[0] == 0) {
    app.Add("Type a name, for example Shiny.");
    return;
  }
  app.RunSearch();
  if (app.result_count_ == 0) {
    app.Add("No result.");
    return;
  }
  for (u32 i = 0; i < app.result_count_; i++) {
    app.Add(app.results_[i].name,
            [i](void*) { MainApplication::GetInstance().OpenSearchResult(i); })
       .WithDescription(app.results_[i].where);
  }
}

void MainApplication::RunSearch() {
  result_count_ = 0;
  c8 query[kQueryLength * 3];
  ToQuery(search_query_, query, sizeof(query));
  if (query[0] == '\0' || contexts_[0].load_menu == nullptr) return;

  // The pages to read, for each depth, and the pages already read.
  static Child children[kMaxContexts][kMaxEntries];
  static u8 child_count[kMaxContexts];
  static u8 child_next[kMaxContexts];
  static Child visited[192];
  u32 visited_count = 0;

  Frame frames[kMaxContexts];
  frames[0].load = contexts_[0].load_menu;
  frames[0].args = contexts_[0].args;
  frames[0].title = nullptr;
  if (!LoadScratch(frames[0].load, frames[0].args)) return;

  u32 depth = 0;
  while (true) {
    // Read the page in the second list: keep the results and the pages.
    child_count[depth] = 0;
    child_next[depth] = 0;
    const c8* section = "";
    for (u32 i = 0; i < scratch_count_; i++) {
      const PageItem& entry = scratch_entries_[i];
      const c8* name = entry.GetName();
      if (!entry.IsSelectable()) {
        if (name != nullptr && name[0] != '\0') section = name;
        continue;
      }
      if (name == nullptr) continue;
      if (entry.GetType() == kTypeMenu) {
        Child& child = children[depth][child_count[depth]++];
        child.load = (menu_callback_t)entry.GetAddress();
        child.args = entry.GetArgs();
        child.name = name;
      }
      if (result_count_ >= kMaxResults || !Contains(name, query)) continue;

      SearchResult& result = results_[result_count_++];
      memcpy(result.frames, frames, sizeof(Frame) * (depth + 1));
      result.depth = depth;
      Copy(result.section, sizeof(result.section), section);
      Copy(result.name, sizeof(result.name), name);
      result.where[0] = '\0';
      for (u32 d = 1; d <= depth; d++) {
        Append(result.where, sizeof(result.where), frames[d].title);
        if (d < depth || section[0] != '\0') {
          Append(result.where, sizeof(result.where), kSeparator);
        }
      }
      Append(result.where, sizeof(result.where),
             section[0] != '\0' ? section : (depth == 0 ? "First page" : ""));
    }
    if (result_count_ >= kMaxResults) return;

    // The next page to read: a child of this page, or of a parent page.
    bool is_loaded = false;
    while (!is_loaded) {
      if (child_next[depth] >= child_count[depth]) {
        if (depth == 0) return;
        depth--;
        continue;
      }
      const Child child = children[depth][child_next[depth]++];
      if (depth + 1 >= kMaxContexts || child.load == LoadSearchPage) continue;
      bool is_visited = false;
      for (u32 i = 0; i < visited_count; i++) {
        if (visited[i].load == child.load && visited[i].args == child.args) {
          is_visited = true;
        }
      }
      if (is_visited || visited_count >= SIZE(visited)) continue;
      visited[visited_count++] = child;
      if (!LoadScratch(child.load, child.args)) continue;
      depth++;
      frames[depth].load = child.load;
      frames[depth].args = child.args;
      frames[depth].title = child.name;
      is_loaded = true;
    }
  }
}

void MainApplication::OpenSearchResult(u32 index) {
  if (index >= result_count_) return;
  const SearchResult result = results_[index];
  NavigateTo(result.frames, result.depth, result.section, result.name);
}

// The radial menu of the pins.

void MainApplication::UpdateRadial(sys::Controller& controller) {
  // The direction of the +Control Pad: up, then clockwise.
  const bool up = controller.IsKeyDown(Key::kUp);
  const bool down = controller.IsKeyDown(Key::kDown);
  const bool left = controller.IsKeyDown(Key::kLeft);
  const bool right = controller.IsKeyDown(Key::kRight);
  s32 direction = -1;
  if (up && right) direction = 1;
  else if (down && right) direction = 3;
  else if (down && left) direction = 5;
  else if (up && left) direction = 7;
  else if (up) direction = 0;
  else if (right) direction = 2;
  else if (down) direction = 4;
  else if (left) direction = 6;
  if (direction >= 0 && direction != radial_direction_) {
    radial_direction_ = direction;
    sys::Sound::PlaySoundEffect(theme_.next_sound);
  }

  s32 selected = -1;
  for (u32 i = 0; i < 8; i++) {
    touch_[kTouchRadial0 + i].Update();
    if (touch_[kTouchRadial0 + i].IsReleased()) selected = i;
  }
  if (controller.IsKeyReleased(Key::kA) && radial_direction_ >= 0) {
    selected = radial_direction_;
  }
  if (controller.IsKeyReleased(Key::kX)) {
    if (radial_direction_ >= 0) {
      // Hold X, push a direction, release X: the pin opens at once.
      selected = radial_direction_;
    } else if (!is_radial_held_) {
      is_radial_open_ = 0;
      sys::Sound::PlaySoundEffect(theme_.close_sound);
      return;
    }
    is_radial_held_ = 0;
  }
  if (controller.IsKeyReleased(Key::kB)) {
    is_radial_open_ = 0;
    sys::Sound::PlaySoundEffect(theme_.close_sound);
    return;
  }
  if (selected < 0) return;

  is_radial_open_ = 0;
  for (u32 i = 0; i < kTouchMax; i++) touch_[i].Reset();
  if ((u32)selected >= pin_count_) {
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return;
  }
  OpenShortcut(pins_[selected]);
}

void MainApplication::DrawRadial() {
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  DrawLabel(6, 4, u"Pinned entries: push a direction, then release X "
                  u"or press A.",
            theme_.unselected_text_color);

  c16 label[24];
  for (u32 i = 0; i < 8; i++) {
    if (i < pin_count_) {
      c8 name[15];
      Copy(name, sizeof(name), pins_[i].name);
      core::Utils::Format(label, u"%s", name);
    } else {
      core::Utils::Format(label, u"---");
    }
    DrawTouch((TouchId)(kTouchRadial0 + i), label,
              radial_direction_ == (s32)i);
  }
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  DrawLabel(128, 112, u"B: Close", theme_.unselected_text_color);
}
} // namespace ui
