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
 * @file main_application.cc
 * @brief The menu of the plugin. See ui/main_application.h.
 */

#include "ui/main_application.h"

#include <cstring>

#include "core/native/event_manager.h"
#include "core/native/process_manager.h"
#include "core/patch/device_patch.h"
#include "core/utils.h"
#include "system/native/controller.h"
#include "system/native/graphics.h"
#include "system/native/sound.h"
#include "ui/theme.h"

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1024
#endif

namespace ui {
namespace {

// The layout of the bottom screen (320 x 240 pixels).
constexpr s32 kEditorY = 64; // The top of the editor.
constexpr s32 kNavY = 206; // The top of the navigation buttons.
constexpr s32 kNavHeight = 30;
constexpr u32 kWrapLength = 58; // The characters of a description line.

// Draws a text (UTF-8) at the position, with a maximum of `max_length`
// characters. Returns the number of characters that it drew.
u32 DrawPart(s32 x, s32 y, const c8* text, u32 max_length, Color color) {
  c8 part[kWrapLength + 1];
  u32 length = strlen(text);
  if (length > max_length) {
    // Cut the text after the last space, when there is one.
    length = max_length;
    for (u32 i = max_length; i > max_length / 2; i--) {
      if (text[i] == ' ') {
        length = i;
        break;
      }
    }
  }
  memcpy(part, text, length);
  part[length] = '\0';
  c16 buffer[kWrapLength + 1];
  core::Utils::Format(buffer, u"%s", part);
  sys::Graphics::DrawText(x, y, buffer, color);
  return length;
}

// Draws a text on two lines at most.
void DrawWrapped(s32 x, s32 y, const c8* text, Color color) {
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  const u32 first = DrawPart(x, y, text, kWrapLength, color);
  const c8* rest = text + first;
  while (*rest == ' ') rest++;
  if (*rest != '\0') {
    sys::Graphics::SetTextScale(0.45f, 0.45f);
    DrawPart(x, y + 12, rest, kWrapLength, color);
  }
}

// The help line of an entry without a description.
const c8* GetHint(u8 type, bool has_texts) {
  switch (type) {
    case kTypeMenu:
      return "Press A or touch the button to open the page.";
    case kTypeIdle:
      return "Press A or touch the button to run the action.";
    case kTypeBoolean:
    case kTypeCheatCode:
      return "Press A, Left / Right, or touch On / Off.";
    case kTypeUnicode:
      return "Type the text on the keyboard, then touch OK.";
    case kTypeSeparator:
      return "";
    default:
      return has_texts ? "Press Left / Right, or touch a value."
                       : "Press Left / Right, type a number, or touch a step.";
  }
}

} // namespace

MainApplication MainApplication::instance_ = MainApplication();

void MainApplication::DrawTop(sys::Graphics& graphics) {
  if (!IsOpened()) return;

  painter_->DrawPageBackground(*this);
  sys::Graphics::SetTextScale(1.0f, 1.0f);
#ifdef GAME_ORAS
  static const c16* kPluginTitle = u"『さんご』";
#endif
#ifdef GAME_XY
  static const c16* kPluginTitle = u"『くじら』";
#endif
  sys::Graphics::DrawText(390, 220, kPluginTitle,
                          Theme::GetInstance().selected_text_color);
  painter_->DrawPageItems(*this);
}

void MainApplication::InitializeTouch() {
  // The navigation bar: four large buttons at the bottom edge.
  for (u32 i = 0; i < 4; i++) {
    touch_[kTouchSectionUp + i].Initialize(4 + i * 79, kNavY, 75, kNavHeight);
  }
  touch_[kTouchOff].Initialize(10, kEditorY + 10, 145, 50);
  touch_[kTouchOn].Initialize(165, kEditorY + 10, 145, 50);
  touch_[kTouchRun].Initialize(10, kEditorY + 10, 300, 50);
  // The step buttons, under the numpad.
  for (u32 i = 0; i < 4; i++) {
    touch_[kTouchMinus10 + i].Initialize(10 + i * 76, kEditorY + 72, 72, 34);
  }
  // A grid of 3 columns and 4 rows for the texts of an entry.
  for (u32 i = 0; i < kMaxChoices; i++) {
    touch_[kTouchChoice0 + i].Initialize(10 + (i % 3) * 101,
                                         kEditorY + (i / 3) * 31, 98, 28);
  }
  numpad_.Initialize(10, kEditorY);
}

MainApplication::Editor MainApplication::GetEditor(
    const PageItem& entry) const {
  switch (entry.GetType()) {
    case kTypeSeparator:
      return Editor::kNone;
    case kTypeUnicode:
      return Editor::kText;
    case kTypeMenu:
      return Editor::kRun;
    case kTypeIdle:
      return entry.HasCallback() ? Editor::kRun : Editor::kNone;
    case kTypeBoolean:
    case kTypeCheatCode:
      return Editor::kSwitch;
    default:
      break;
  }
  const u32 size = entry.GetArraySize();
  if (size != 0 && size <= kMaxChoices) return Editor::kChoices;
  return entry.HasValue() ? Editor::kNumber : Editor::kNone;
}

void MainApplication::DrawTouch(TouchId id, const c16* label,
                                bool is_active) const {
  const Button& button = touch_[id];
  const s32 x = button.GetX();
  const s32 y = button.GetY();
  const s32 width = button.GetWidth();
  const s32 height = button.GetHeight();
  const bool is_down = button.IsDown();

  if (is_active || is_down) {
    Color fill = theme_.selected_text_color;
    fill.a = is_down ? 0.7f : 0.4f;
    sys::Graphics::DrawRect(x, y, width, height, fill);
  }
  sys::Graphics::DrawRectStroke(x, y, width, height, 1,
                                is_active ? theme_.selected_text_color
                                          : theme_.unselected_text_color);
  sys::Graphics::SetTextScale(0.5f, 0.5f);
  sys::Graphics::DrawText(x + 6, y + (height - 12) / 2, label,
                          theme_.unselected_text_color);
}

void MainApplication::DrawBottom(sys::Graphics& graphics) {
  painter_->DrawBottomOverlay(graphics);

  if (!IsOpened() || !painter_->ShowBottom()) return;

  sys::Graphics::FillScreen(theme_.background_color);
  sys::Graphics::DrawRectStroke(0, 0, 320, 240, 1,
                                theme_.selected_text_color);
  if (entries_count_ == 0) return;

  const PageItem& entry = GetSelectedEntry();
  const Editor editor = GetEditor(entry);
  if (editor == Editor::kText) {
    keyboard_.Draw();
    return;
  }

  c16 buffer[BUFFER_SIZE];

  // Where the player is: the names of the open pages.
  c8 path[96] = "Menu";
  for (u32 i = 1; i < contexts_count_; i++) {
    const c8* title = contexts_[i].title;
    if (title == nullptr) continue;
    strncat(path, " > ", sizeof(path) - strlen(path) - 1);
    strncat(path, title, sizeof(path) - strlen(path) - 1);
  }
  sys::Graphics::SetTextScale(0.45f, 0.45f);
  DrawPart(6, 4, path, kWrapLength, theme_.unselected_text_color);

  // The selected entry and its description.
  const bool is_pinned = FindPin(entry) >= 0;
  sys::Graphics::SetTextScale(0.6f, 0.6f);
  core::Utils::Format(buffer, u"%s%s", entry.GetName(),
                      is_pinned ? "  [Pinned]" : "");
  sys::Graphics::DrawText(6, 16, buffer, theme_.selected_text_color);
  const c8* description = entry.GetDescription();
  if (description == nullptr) {
    description = GetHint(entry.GetType(), entry.GetArraySize() != 0);
  }
  DrawWrapped(6, 36, description, theme_.unselected_text_color);

  // The editor of the value.
  const s32 index = entry.HasValue() ? entry.GetIndex() : -1;
  switch (editor) {
    case Editor::kRun:
      DrawTouch(kTouchRun,
                entry.GetType() == kTypeMenu ? u"Open the page   (A)"
                                             : u"Run   (A)",
                false);
      break;
    case Editor::kSwitch: {
      const bool has_texts = entry.GetArraySize() == 2;
      core::Utils::Format(buffer, u"%s", has_texts ? entry.GetArray()[0] : "Off");
      DrawTouch(kTouchOff, buffer, index == 0);
      core::Utils::Format(buffer, u"%s", has_texts ? entry.GetArray()[1] : "On");
      DrawTouch(kTouchOn, buffer, index != 0);
      break;
    }
    case Editor::kChoices: {
      const u32 size = entry.GetArraySize();
      for (u32 i = 0; i < size; i++) {
        core::Utils::Format(buffer, u"%s", entry.GetArray()[i]);
        DrawTouch((TouchId)(kTouchChoice0 + i), buffer,
                  (u32)index % size == i);
      }
      break;
    }
    case Editor::kNumber:
      sys::Graphics::SetTextScale(0.6f, 0.6f);
      numpad_.Draw();
      DrawTouch(kTouchMinus10, u"-10", false);
      DrawTouch(kTouchMinus1, u"-1", false);
      DrawTouch(kTouchPlus1, u"+1", false);
      DrawTouch(kTouchPlus10, u"+10", false);
      break;
    default:
      break;
  }

  // The state of the game, for the developers.
  uptr vtable = 0;
  const char* process_name = core::Utils::Unmangle(
      core::ProcessManager::GetInstance().GetCurrentProcessName(vtable));
  const char* event_name = core::Utils::Unmangle(
      core::EventManager::GetInstance().GetCurrentEventName(vtable));
  core::Utils::Format(buffer, u"%s | %s", process_name, event_name);
  sys::Graphics::SetTextScale(0.4f, 0.4f);
  sys::Graphics::DrawText(6, 192, buffer, theme_.unselected_text_color);

  // The navigation bar. Each button also shows its key.
  DrawTouch(kTouchSectionUp, u"L  Prev.", false);
  DrawTouch(kTouchSectionDown, u"R  Next", false);
  DrawTouch(kTouchPin, is_pinned ? u"Y  Unpin" : u"Y  Pin", is_pinned);
  DrawTouch(kTouchBack, u"B  Back", false);
}

void MainApplication::ForceClose() {
  sys::Sound::PlaySoundEffect(IsOpened()
                                ? theme_.close_sound
                                : theme_.open_sound);
  is_opened_ = false;
  core::DevicePatch::GetInstance().use_redirection = is_opened_;
}

void MainApplication::ReadTouch(const PageItem& entry, Input& input) {
  if (!painter_->ShowBottom()) return;

  const Editor editor = GetEditor(entry);
  const u8 index = GetContext().cursor + GetContext().offset;
  if (editor != editor_ || index != touch_index_) {
    // A touch that started on a different entry does not count.
    for (u32 i = 0; i < kTouchMax; i++) touch_[i].Reset();
    editor_ = editor;
    touch_index_ = index;
  }
  if (editor == Editor::kText) return;

  for (u32 i = kTouchSectionUp; i <= kTouchBack; i++) touch_[i].Update();
  if (touch_[kTouchSectionUp].IsReleased()) input.section = -1;
  if (touch_[kTouchSectionDown].IsReleased()) input.section = 1;
  if (touch_[kTouchPin].IsReleased()) input.pin = true;
  if (touch_[kTouchBack].IsReleased()) input.back = true;

  switch (editor) {
    case Editor::kRun:
      touch_[kTouchRun].Update();
      if (touch_[kTouchRun].IsReleased()) input.run = true;
      break;
    case Editor::kSwitch:
      touch_[kTouchOff].Update();
      touch_[kTouchOn].Update();
      if (touch_[kTouchOff].IsReleased()) input.choice = 0;
      if (touch_[kTouchOn].IsReleased()) input.choice = 1;
      break;
    case Editor::kChoices:
      for (u32 i = 0; i < entry.GetArraySize(); i++) {
        touch_[kTouchChoice0 + i].Update();
        if (touch_[kTouchChoice0 + i].IsReleased()) input.choice = i;
      }
      break;
    case Editor::kNumber: {
      static const s32 kSteps[] = {-10, -1, 1, 10};
      for (u32 i = 0; i < 4; i++) {
        touch_[kTouchMinus10 + i].Update();
        if (touch_[kTouchMinus10 + i].IsReleased()) input.step = kSteps[i];
      }
      break;
    }
    default:
      break;
  }
}

void MainApplication::Update(sys::Controller& controller) {
  if (AreKeysReleased(controller)) {
    sys::Sound::PlaySoundEffect(IsOpened() ? theme_.close_sound : theme_.open_sound);
    is_opened_ ^= 1;
    core::DevicePatch::GetInstance().use_redirection = is_opened_;
    return;
  }

  if (!IsOpened()) return;
  if (entries_count_ == 0) {
    if (controller.IsKeyReleased(Key::kB)) Close();
    return;
  }

  PageItem& entry = GetSelectedEntry();
  // A copy: an action can build the page again.
  const PageItem selected = entry;
  const Editor editor = GetEditor(entry);

  if (painter_->ShowBottom()) {
    if (editor == Editor::kText) {
      keyboard_.Update();
    } else if (editor == Editor::kNumber) {
      numpad_.Update();
    }
  }

  // Read the buttons and the touch screen.
  Input input;
  if (controller.IsKeyDown(Key::kUp) || controller.IsKeyDown(Key::kDown)) {
    hold_frames_++;
  } else {
    hold_frames_ = 0;
  }
  const s32 speed = hold_frames_ > kFastScrollFrames ? 3 : 1;
  if (controller.IsKeyRepeated(Key::kDown)) input.move = speed;
  if (controller.IsKeyRepeated(Key::kUp)) input.move = -speed;
  if (controller.IsKeyRepeated(Key::kL)) input.section = -1;
  if (controller.IsKeyRepeated(Key::kR)) input.section = 1;
  if (controller.IsKeyRepeated(Key::kRight)) input.step = 1;
  if (controller.IsKeyRepeated(Key::kLeft)) input.step = -1;
  if (controller.IsKeyReleased(Key::kB)) input.back = true;
  if (controller.IsKeyReleased(Key::kA)) input.run = true;
  if (controller.IsKeyReleased(Key::kY)) input.pin = true;
  if (controller.IsKeyReleased(Key::kX) ||
      (editor == Editor::kNumber && numpad_.IsButtonOkReleased()) ||
      (editor == Editor::kText && keyboard_.IsButtonOkReleased())) {
    input.apply = true;
  }
  ReadTouch(entry, input);

  // Do the actions.
  if (input.back) {
    Close();
    return;
  }
  if (input.pin) {
    TogglePin(selected);
    return;
  }

  bool is_changed = false;
  const u8 type = entry.GetType();
  const bool is_switch = type == kTypeBoolean || type == kTypeCheatCode;
  if (input.run) {
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);
    if (is_switch && !entry.HasCallback()) {
      entry.Increment();
      is_changed = true;
    } else {
      if (type != kTypeMenu && entry.HasCallback()) AddRecent(selected);
      entry.Execute(*this);
      return;
    }
  } else if (input.step != 0) {
    sys::Sound::PlaySoundEffect(theme_.next_sound);
    if (input.step > 0) {
      entry.Increment(input.step);
    } else {
      entry.Decrement(-input.step);
    }
    is_changed = true;
  } else if (input.choice >= 0 && entry.HasValue()) {
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);
    const s32 delta = input.choice - entry.GetIndex();
    if (is_switch) {
      if (delta != 0) entry.Increment();
    } else if (delta > 0) {
      entry.Increment(delta);
    } else if (delta < 0) {
      entry.Decrement(-delta);
    }
    is_changed = true;
  } else if (input.apply) {
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);
    if (type == kTypeUnicode) {
      entry.Edit(keyboard_.GetInput());
    } else {
      u32 value = numpad_.GetInput();
      entry.Edit(&value);
    }
    is_changed = true;
  }
  if (is_changed) AddRecent(selected);

  if (input.move != 0) {
    sys::Sound::PlaySoundEffect(theme_.next_sound);
    // Wrap only for a new press: a held button stops at the end of the page.
    MoveCursor(input.move, hold_frames_ == 1);
  } else if (input.section != 0) {
    sys::Sound::PlaySoundEffect(theme_.next_sound);
    JumpSection(input.section);
  }
}

void MainApplication::Select(u32 index) {
  if (index >= entries_count_) return;
  MenuContext& ctx = GetContext();
  u32 offset = ctx.offset;
  if (index < offset) offset = index;
  if (index >= offset + ctx.display_count) {
    offset = index - ctx.display_count + 1;
  }
  // Show the title of the section of the entry.
  if (index > 0 && index - 1 < offset && !entries_[index - 1].IsSelectable()) {
    offset = index - 1;
  }
  ctx.offset = offset;
  ctx.cursor = index - offset;
}

void MainApplication::MoveCursor(s32 delta, bool wrap) {
  const s32 count = entries_count_;
  const s32 direction = delta > 0 ? 1 : -1;
  s32 index = GetContext().cursor + GetContext().offset;
  for (s32 n = 0; n < delta * direction; n++) {
    s32 next = index + direction;
    while (next >= 0 && next < count && !entries_[next].IsSelectable()) {
      next += direction;
    }
    if (next < 0 || next >= count) {
      if (!wrap || n > 0) break;
      next = direction > 0 ? 0 : count - 1;
      while (next >= 0 && next < count && !entries_[next].IsSelectable()) {
        next += direction;
      }
      if (next < 0 || next >= count) break;
    }
    index = next;
  }
  Select(index);
}

void MainApplication::JumpSection(s32 direction) {
  const s32 count = entries_count_;
  bool has_sections = false;
  for (s32 i = 0; i < count; i++) {
    if (!entries_[i].IsSelectable()) has_sections = true;
  }
  // Without sections, L and R jump one screen.
  if (!has_sections) {
    MoveCursor(direction * (s32)GetContext().display_count, false);
    return;
  }

  const s32 index = GetContext().cursor + GetContext().offset;
  if (direction > 0) {
    s32 i = index + 1;
    while (i < count && entries_[i].IsSelectable()) i++;
    while (i < count && !entries_[i].IsSelectable()) i++;
    if (i >= count) {
      // After the last section, go back to the first entry.
      i = 0;
      while (i < count && !entries_[i].IsSelectable()) i++;
    }
    Select(i);
    return;
  }

  // Go to the start of the section, then to the previous section.
  s32 start = index;
  while (start > 0 && entries_[start - 1].IsSelectable()) start--;
  if (start < index) {
    Select(start);
    return;
  }
  s32 i = start - 1;
  while (i >= 0 && !entries_[i].IsSelectable()) i--;
  if (i < 0) {
    // Before the first section, go to the last section.
    i = count - 1;
    while (i >= 0 && !entries_[i].IsSelectable()) i--;
    if (i < 0) return;
  }
  while (i > 0 && entries_[i - 1].IsSelectable()) i--;
  Select(i);
}

void MainApplication::FinishLoad() {
  MenuContext& ctx = GetContext();
  ctx.display_count =
      entries_count_ > kMaxDisplayCount ? kMaxDisplayCount : entries_count_;
  if (ctx.cursor + ctx.offset >= entries_count_) {
    ctx.cursor = 0;
    ctx.offset = 0;
  }
  if (entries_count_ == 0) return;
  const u32 index = ctx.cursor + ctx.offset;
  if (entries_[index].IsSelectable()) return;
  // The page starts with a title: select the first entry after it.
  for (u32 i = index; i < entries_count_; i++) {
    if (entries_[i].IsSelectable()) {
      Select(i);
      return;
    }
  }
}

void MainApplication::Open(menu_callback_t load_menu, void* args,
                           const c8* title) {
  if (contexts_count_ >= kMaxContexts) return;

  contexts_[contexts_count_].Initialize(load_menu, args, title);
  contexts_count_++;

  entries_count_ = 0;
  no_background_ = 0;
  is_quick_access_ = 0;
  process_vtable_ = 0;
  load_menu(*this, args);
  FinishLoad();
}

void MainApplication::Close() {
  if (contexts_count_ <= 1) return;

  contexts_count_--;

  MenuContext& ctx = GetContext();

  entries_count_ = 0;
  no_background_ = 0;
  is_quick_access_ = 0;
  process_vtable_ = 0;
  ctx.load_menu(*this, ctx.args);
  FinishLoad();
}

void MainApplication::Refresh() {
  MenuContext& ctx = GetContext();
  entries_count_ = 0;
  is_quick_access_ = 0;
  ctx.load_menu(*this, ctx.args);
  FinishLoad();
}

MainApplication& MainApplication::AddQuickAccess() {
  is_quick_access_ = 1;
  first_pin_ = first_recent_ = 0xFF;

  AddSection("Pinned");
  if (pin_count_ == 0) {
    Add("Press Y on an entry to pin it here")
        .WithDescription("The pinned entries stay at the top of the menu. "
                         "Press Y on a pinned entry to remove it.");
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
    PageItem& entry = entries_[entries_count_++];
    // An entry of a page that needs a different part of the game.
    if (shortcut.vtable != 0 &&
        !core::ProcessManager::GetInstance().IsCurrentProcess(shortcut.vtable)) {
      entry.Initialize(shortcut.name, nullptr, kTypeIdle);
      entry.WithDescription("Not available here. Go to the part of the game "
                            "that this entry needs (for example a battle).");
      continue;
    }
    entry = shortcut.item;
  }
}

void MainApplication::Store(Shortcut& shortcut, const PageItem& entry,
                            uptr vtable) {
  shortcut.item = entry;
  const c8* name = entry.GetName() != nullptr ? entry.GetName() : "";
  strncpy(shortcut.name, name, sizeof(shortcut.name) - 1);
  shortcut.name[sizeof(shortcut.name) - 1] = '\0';
  shortcut.item.SetName(shortcut.name);
  shortcut.vtable = vtable;
}

void MainApplication::Remove(Shortcut* list, u8& count, u32 index) {
  if (index >= count) return;
  for (u32 i = index; i + 1 < count; i++) {
    list[i] = list[i + 1];
    list[i].item.SetName(list[i].name);
  }
  count--;
}

s32 MainApplication::FindPin(const PageItem& entry) const {
  for (u32 i = 0; i < pin_count_; i++) {
    if (pins_[i].item.IsSameAs(entry)) return i;
  }
  return -1;
}

void MainApplication::TogglePin(const PageItem& entry) {
  const u32 index = GetContext().cursor + GetContext().offset;
  s32 pin = FindPin(entry);
  // An entry of the quick access that is not available has no data.
  if (pin < 0 && is_quick_access_ && first_pin_ != 0xFF &&
      index >= first_pin_ && index < first_pin_ + pin_count_) {
    pin = index - first_pin_;
  }

  if (pin >= 0) {
    Remove(pins_, pin_count_, pin);
    sys::Sound::PlaySoundEffect(theme_.close_sound);
  } else if (!entry.IsSelectable() || pin_count_ >= kMaxPins ||
             (entry.GetType() == kTypeIdle && !entry.HasCallback())) {
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return;
  } else {
    uptr vtable = process_vtable_;
    if (is_quick_access_ && first_recent_ != 0xFF && index >= first_recent_ &&
        index < first_recent_ + recent_count_) {
      vtable = recents_[index - first_recent_].vtable;
    }
    Store(pins_[pin_count_++], entry, vtable);
    sys::Sound::PlaySoundEffect(theme_.confirm_sound);
  }
  if (is_quick_access_) Refresh();
}

void MainApplication::AddRecent(const PageItem& entry) {
  if (is_quick_access_ || !entry.IsSelectable()) return;
  if (entry.GetType() == kTypeMenu) return;

  // The same entry moves to the top of the list.
  for (u32 i = 0; i < recent_count_; i++) {
    if (recents_[i].item.IsSameAs(entry)) {
      Remove(recents_, recent_count_, i);
      break;
    }
  }
  if (recent_count_ >= kMaxRecents) recent_count_ = kMaxRecents - 1;
  for (u32 i = recent_count_; i > 0; i--) {
    recents_[i] = recents_[i - 1];
    recents_[i].item.SetName(recents_[i].name);
  }
  Store(recents_[0], entry, process_vtable_);
  recent_count_++;
}

bool MainApplication::CheckProcess(uptr vtable) {
  if (!core::ProcessManager::GetInstance().IsCurrentProcess(vtable)) {
    while (contexts_count_ > 1) {
      Close();
    }
    sys::Sound::PlaySoundEffect(theme_.error_sound);
    return true;
  }
  process_vtable_ = vtable;
  return false;
}

bool MainApplication::AreKeysReleased(sys::Controller& controller) {
  if (theme_.keys[0] == 0) {
    return controller.IsKeyReleased(Key::kStart);
  }

  bool res = true;
  for (u32 i = 0; i < SIZE(theme_.keys) && theme_.keys[i] != 0; i++) {
    u32 key = 1 << (theme_.keys[i] - 1);
    res &= controller.IsKeyReleased((Key)key);
  }
  return res;
}

u32 Painter::GetCursor(MainApplication& app) { return app.GetContext().cursor; }

u32 Painter::GetOffset(MainApplication& app) { return app.GetContext().offset; }

u32 Painter::GetDisplayCount(MainApplication& app) {
  return app.GetContext().display_count;
}

PageItem& Painter::GetEntry(MainApplication& app, u32 index) {
  return app.entries_[index];
}

void MainAppPainter::DrawPageBackground(MainApplication& app) {
  if (!app.no_background_) {
    sys::Graphics::FillScreen(app.theme_.background_color);
  }
  sys::Graphics::DrawRectStroke(0, 0, 400, 240, 1,
                                Theme::GetInstance().selected_text_color);
}

void MainAppPainter::DrawPageItems(MainApplication& app) {
  MainApplication::MenuContext& ctx = app.GetContext();

  sys::Graphics::SetTextScale(0.6, 0.6);
  sys::Graphics::DrawText(5, 6 + ctx.cursor * MainApplication::kLineHeight,
                          u"", app.theme_.selected_text_color);

  c16 buffer[BUFFER_SIZE];
  for (u32 i = 0; i < ctx.display_count; i++) {
    app.entries_[i + ctx.offset].GetDisplayValue(buffer);
    sys::Graphics::DrawText(25, 5 + i * MainApplication::kLineHeight, buffer,
                            ctx.cursor == i
                              ? app.theme_.selected_text_color
                              : app.theme_.unselected_text_color);
  }

  // The scroll bar: where the visible entries are in the page.
  const u32 count = app.entries_count_;
  if (count > ctx.display_count && ctx.display_count != 0) {
    constexpr s32 kTrackY = 4;
    constexpr s32 kTrackHeight = 200;
    s32 thumb = kTrackHeight * ctx.display_count / count;
    if (thumb < 12) thumb = 12;
    const s32 y = kTrackY + (kTrackHeight - thumb) * ctx.offset /
                                (count - ctx.display_count);
    Color track = app.theme_.unselected_text_color;
    track.a = 0.25f;
    sys::Graphics::DrawRect(394, kTrackY, 2, kTrackHeight, track);
    sys::Graphics::DrawRect(393, y, 4, thumb, app.theme_.selected_text_color);
  }
}
} // namespace ui
